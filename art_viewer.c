#if _WIN32_WINNT<0x400
#define _WIN32_WINNT 0x400
#endif
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "resource.h"
#include "vga737.h"

extern HINSTANCE ghinstance;
static HWND hstatic;
int client_width=0,client_height=0;
int default_color=FALSE;

#define MIRC_MAX_COLORS 15
#define MAX_COLOR_LOOKUP 18
int color_lookup[MAX_COLOR_LOOKUP]={
	0xFFFFFF, //0 white
	0x000000, //1 black
	0x7F0000, //2 blue (navy)
	0x009300, //3 green
	0x0000FF, //4 red
	0x00007F, //5 brown (maroon)
	0x9C009C, //6 purple
	0x007FFC, //7 orange (olive)
	0x00FFFF, //8 yellow
	0x00FC00, //9 light green (lime)
	0x939300, //10 teal (a green/blue cyan)
	0xFFFF00, //11 light cyan (cyan) (aqua)
	0xFC0000, //12 light blue (royal)
	0xFF00FF, //13 pink (light purple) (fuchsia)
	0x7F7F7F, //14 grey
	0xD2D2D2, //15 light grey (silver)
	0x000000, //16 windows background
	0x00FF00  //17 windows foreground
};
enum{MIRC_BOLD=2,MIRC_COLOR=3,MIRC_UNDERLINE=31,MIRC_REVERSE=22,MIRC_PLAIN=15,MIRC_BG=16,MIRC_FG=17};

int draw_char(HDC hdc,unsigned char a,int x,int y,int cf,int cb)
{
	int i,j;
	unsigned char *p=vga737_bin+a*12;
	for(i=0;i<12;i++){
		for(j=0;j<8;j++){
			int c=0;
			if(p[i]&(1<<(7-j)))
				c=cf;
			else
				c=cb;
			SetPixel(hdc,x+j,y+i,c);
		}
	}
	return 0;
}
int clear_screen(HDC hdc)
{
	int back_ground=BLACKNESS;
	if(default_color)
		back_ground=WHITENESS;
	BitBlt(hdc,0,0,client_width,client_height,hdc,0,0,back_ground);
	return 0;
}
int draw_edit_art(HDC hdc,int line,int line_count)
{
	unsigned char str[1024];
	int i,cpy,x,y,state=0,count=0,out_line=0;
	unsigned char cf,cb;
	cf=MIRC_FG;
	cb=MIRC_BG;
	x=y=0;
	clear_screen(hdc);
	for(i=0;i<100;i++){
		memset(str,0,sizeof(str));
		str[0]=sizeof(str)-1;
		str[1]=(sizeof(str)-1)>>8;
		str[sizeof(str)-1]=0;
		cpy=SendMessage(hstatic,EM_GETLINE,line+i,str);
		if(cpy>0 && cpy<sizeof(str)){
			int j;
			//printf("%s\n",str);
			for(j=0;j<cpy;j++){
				if(str[j]=='\r'){
					cf=MIRC_FG;
					cb=MIRC_BG;
					x=0;
					y+=12;
					out_line++;
					if(out_line>=line_count)
						return 0;
					continue;
				}
				else if(str[j]==0)
					continue;
				else if(str[j]==MIRC_REVERSE){
					state=0;
					cf=MIRC_BG;
					cb=MIRC_FG;
				}
				else if(str[j]==MIRC_COLOR){
					state=1;
					count=0;
				}
				else if(str[j]==MIRC_PLAIN){
					state=0;
					cf=MIRC_FG;
					cb=MIRC_BG;
				}
				else if(str[j]>=' '){
					switch(state){
					case 0:
do_draw:
						{
							int fg=cf,bg=cb;
							if(fg==bg && (!default_color)){
								if(bg==0 || bg==1){ //black or white
									bg=MIRC_BG;fg=MIRC_FG;
								}
							}
							draw_char(hdc,str[j],x,y,color_lookup[fg%MAX_COLOR_LOOKUP],color_lookup[bg%MAX_COLOR_LOOKUP]);
						}
						x+=8;
						count=0;
						break;
					case 1:
						if(isdigit(str[j])){
							if(count==0)
								cf=0;
							cf*=10;
							cf+=str[j]-'0';
							if(cf>MIRC_MAX_COLORS)
								cf=MIRC_FG;
							count++;
							if(count>=2){
								if(str[j+1]!=',')
									state=0;
							}
						}
						else if(str[j]==','){
							if(isdigit(str[j+1])){
								state=2;
								count=0;
							}
							else{
								state=0;
								goto do_draw;
							}
						}
						else{
							if(count==0){
								cf=MIRC_FG;cb=MIRC_BG;
							}
							state=0;
							goto do_draw;
						}
						break;
					case 2:
						if(isdigit(str[j])){
							if(count==0)
								cb=0;
							cb*=10;
							cb+=str[j]-'0';
							if(cb>MIRC_MAX_COLORS)
								cb=MIRC_BG;
							count++;
							if(count>=2)
								state=0;
						}
						else{
							state=0;
							goto do_draw;
						}
						break;
					default:
						state=0;
						count=0;
						break;
					}
				}
			}
		}
	}
	return 0;
}
static int set_title(HWND hwnd,int line)
{
	char str[80];
	_snprintf(str,sizeof(str),"Ascii art  %u",line);
	SetWindowText(hwnd,str);
	return 0;
}
static int calc_scrollbar(HWND hwnd,int line)
{
	int count,ratio;
	count=SendMessage(hstatic,EM_GETLINECOUNT,0,0);
	if(count==0)count=line;
	if(line==0)
		ratio=0;
	else
		ratio=(float)line/(float)count*100.0;
	SendMessage(GetDlgItem(hwnd,IDC_SCROLLBAR),SBM_SETPOS,ratio,TRUE);
	return 0;
}
BOOL CALLBACK art_viewer(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static HWND grippy=0;
	static int line=0;
	static int vlines=30;
	PAINTSTRUCT ps;
	HDC hdc;
	if(FALSE)
	if(msg!=WM_MOUSEFIRST&&msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE/*&&msg!=WM_NOTIFY*/)
	//if(msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE)
	{
		static DWORD tick;
		if((GetTickCount()-tick)>500)
			printf("--\n");
		print_msg(msg,lparam,wparam,hwnd);
		tick=GetTickCount();
	}
	switch(msg){
	case WM_INITDIALOG:
		grippy=create_grippy(hwnd);
		line=SendMessage(hstatic,EM_GETFIRSTVISIBLELINE,0,0);
		{
			RECT rect={0};
			if(GetWindowRect(GetParent(hstatic),&rect)!=0){
				SetWindowPos(hwnd,NULL,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_NOZORDER);
			}
			SendMessage(GetDlgItem(hwnd,IDC_SCROLLBAR),SBM_SETRANGE,0,100);
		}
		set_title(hwnd,line);
		calc_scrollbar(hwnd,line);
		default_color=0;
		color_lookup[0]=GetSysColor(COLOR_BACKGROUND);
		color_lookup[MIRC_BG]=GetSysColor(COLOR_BACKGROUND);
		color_lookup[MIRC_FG]=GetSysColor(COLOR_WINDOWTEXT);
		break;
	case WM_SIZE:
		{
		RECT rect={0};
		grippy_move(hwnd,grippy);
		if(GetClientRect(hwnd,&rect)!=0){
			int x,y,w,h;
			x=rect.right-20;
			y=rect.top;
			w=20;
			h=rect.bottom-rect.top-GetSystemMetrics(SM_CYCAPTION);
			SetWindowPos(GetDlgItem(hwnd,IDC_SCROLLBAR),NULL,x,y,w,h,SWP_NOZORDER);
			vlines=(rect.bottom-rect.top)/12;
			if(vlines<=5)
				vlines=5;
			InvalidateRect(hwnd,NULL,TRUE);
			client_width=rect.right-rect.left-20;
			client_height=rect.bottom-rect.top;
		}
		}
		break;
	case WM_RBUTTONDOWN:
	case WM_HELP:
		default_color^=1;
		if(default_color){
			color_lookup[0]=0xFFFFFF;
			color_lookup[MIRC_BG]=0xFFFFFF;
			color_lookup[MIRC_FG]=0;
			SetWindowText(hwnd,"Art viewer - default color");
		}
		else{
			color_lookup[0]=GetSysColor(COLOR_BACKGROUND);
			color_lookup[MIRC_BG]=GetSysColor(COLOR_BACKGROUND);
			color_lookup[MIRC_FG]=GetSysColor(COLOR_WINDOWTEXT);
			SetWindowText(hwnd,"Art viewer - system color");
		}
		InvalidateRect(hwnd,NULL,TRUE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam)){
		case IDCANCEL:
			EndDialog(hwnd,0);
			break;
		}
		break;
	case WM_VSCROLL:
		{
			int modifier=1;
			int dir=0;
			int count=SendMessage(hstatic,EM_GETLINECOUNT,0,0);
			switch(LOWORD(wparam)){
			case SB_PAGEDOWN:dir=1;modifier=10;break;
			case SB_PAGEUP:dir=-1;modifier=10;break;
			case SB_LINEUP:dir=-1;modifier=1;break;
			case SB_LINEDOWN:dir=1;modifier=1;break;
			case SB_BOTTOM:line=count-1;break;
			case SB_TOP:line=0;break;
			case SB_THUMBTRACK:
				{
				int pos=HIWORD(wparam);
				line=(float)pos*(float)count/100.0;
				if(line>=count)
					line=count-1;
				}
				break;
			case SB_ENDSCROLL:return 0;
			}
			if(line==0 && dir<0)
				break;
			line+=dir*modifier;
			if(line<0)
				line=0;
			if(dir>0){
				if(line>=count)
					line=count-1;
			}
			set_title(hwnd,line);
			calc_scrollbar(hwnd,line);
			InvalidateRect(hwnd,NULL,TRUE);
		}
		break;
	case WM_MOUSEWHEEL:
		{
			short y=HIWORD(wparam);
			int dir=0;
			int modifier=(LOWORD(wparam)&MK_RBUTTON)?10:1;
			int count=SendMessage(hstatic,EM_GETLINECOUNT,0,0);
			if(y>0){
				if(line==0)
					break;
				dir=-1;
			}
			else
				dir=1;
			line+=dir*modifier*5;
			if(line>=count)
				line=count-1;
			else if(line<0)
				line=0;
			set_title(hwnd,line);
			calc_scrollbar(hwnd,line);
			InvalidateRect(hwnd,NULL,TRUE);
		}
		break;
	case WM_ERASEBKGND:
		return TRUE;
	case WM_PAINT:
		hdc=BeginPaint(hwnd,&ps);
		draw_edit_art(hdc,line,vlines);
		EndPaint(hwnd,&ps);
		break;
	case WM_CLOSE:
	case WM_QUIT:
		EndDialog(hwnd,0);
		break;	
	}
	return 0;
}
int show_art_viewer(HWND hwnd,HWND htext)
{
	if(htext==0)
		return 0;
	hstatic=htext;
	DialogBox(ghinstance,MAKEINTRESOURCE(IDD_ARTVIEWER),hwnd,art_viewer);
	return 0;
}