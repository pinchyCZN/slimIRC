#if _WIN32_WINNT<0x400
#define _WIN32_WINNT 0x400
#endif
#include <windows.h>
#include <richedit.h>
#include <stdlib.h>
#include <stdio.h>
#include "resource.h"
#include "vga737.h"

extern HINSTANCE ghinstance;
static HWND hstatic;
int client_width=0,client_height=0;
int default_color=FALSE;
char *vgargb=0;
char *fontname=0;

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
	struct TMP{
		BITMAPINFOHEADER bmiHeader;
		DWORD colors[2];
	};
	struct TMP bmi={0};
	bmi.bmiHeader.biBitCount=8;
	bmi.bmiHeader.biWidth=8;
	bmi.bmiHeader.biHeight=12;
	bmi.bmiHeader.biPlanes=1;
	bmi.bmiHeader.biSizeImage=8*12;
	bmi.bmiHeader.biXPelsPerMeter=12;
	bmi.bmiHeader.biYPelsPerMeter=12;
	bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bmi.colors[1]=cf;
	bmi.colors[0]=cb;
	SetDIBitsToDevice(hdc,x,y,8,12,
		0,0, //src xy
		0,12, //startscan,scanlines
		vgargb+a*12*8,
		(BITMAPINFO*)&bmi,DIB_RGB_COLORS);
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
int draw_unicode(HDC hdc,int line,int line_count)
{
	char str[768];
	int i,offset=3;
	HFONT hfont=0,hfold=0;
	clear_screen(hdc);
	if(fontname){
		TEXTMETRIC tm;
		GetTextMetrics(hdc,&tm);
		hfont=CreateFont(tm.tmHeight,tm.tmAveCharWidth,0,0,tm.tmWeight,
			0,0,0,tm.tmCharSet,
			OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,DEFAULT_PITCH,
			fontname);
		if(hfont)
			hfold=SelectObject(hdc,hfont);
	}
	if(default_color){
		SetBkColor(hdc,color_lookup[0]);
		SetTextColor(hdc,color_lookup[1]);
	}else{
		SetBkColor(hdc,GetSysColor(COLOR_BACKGROUND));
		SetTextColor(hdc,GetSysColor(COLOR_WINDOWTEXT));
	}
	for(i=0;i<line_count;i++){
		int cpy;
		memset(str,0,sizeof(str));
		((short*)(str+offset))[0]=sizeof(str)-offset;
		cpy=SendMessage(hstatic,EM_GETLINE,line+i,str+offset);
		if(cpy>0 && cpy<(sizeof(str)/sizeof(WCHAR))){
			WCHAR tmp[1024];
			SIZE size={0};
			RECT rect={0};
			int len;
			str[0]=0xEF;
			str[1]=0xBB;
			str[2]=0xBF;
			cpy+=offset;
			len=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,str,cpy,tmp,sizeof(tmp)/sizeof(WCHAR));
			if(len>0){
				GetTextExtentPoint32W(hdc,tmp,len,&size);
				rect.right=size.cx;
				rect.top=i*size.cy;
				rect.bottom=rect.top+size.cy;
				DrawTextExW(hdc,tmp,len,&rect,0,NULL);
			}
		}
	}
	if(hfold){
		SelectObject(hdc,hfold);
		DeleteObject(hfont);
	}
	return 0;
}
static int free_vga_font()
{
	if(vgargb){
		free(vgargb);
		vgargb=0;
	}
	return 0;
}
static int create_vga_font()
{
	if(vgargb==0)
		vgargb=malloc(8*12*256);
	if(vgargb){
		int i,j,k;
		for(k=0;k<256;k++){
			for(i=0;i<12;i++){
				for(j=0;j<8;j++){
					int c;
					char *p=vga737_bin+k*12;
					if(p[i]&(1<<(7-j)))
						c=1;
					else
						c=0;
					//flip y
					vgargb[k*8*12+j+(11-i)*8]=c;
				}
			}
		}
	}
	return 0;
}
static int set_title(HWND hwnd,int view_utf8,int default_color,int line)
{
	char str[80];
	_snprintf(str,sizeof(str),"%s - %s , line %i",
		view_utf8?"UTF8 view":"Art viewer",
		default_color?"default color":"system color",line);
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
static BOOL CALLBACK select_font(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#define IDC_LIST_BOX (IDC_USER_EDIT+1)
	static HWND hparent=0,hlist=0;
	static int selected_font=0;
	static char *fonts[5]={"Default","Arial","Tahoma","Verdana","MS Serif"};

	switch(msg){
	case WM_INITDIALOG:
		{
			RECT rect={0};
			hparent=lparam;
			GetWindowRect(GetDlgItem(hwnd,IDC_USER_EDIT),&rect);
			MapWindowPoints(0,hwnd,&rect,2);
			hlist=CreateWindow("COMBOBOX","combo",WS_CHILD|WS_TABSTOP|WS_VISIBLE|CBS_DROPDOWNLIST|CBS_SIMPLE,
				rect.left,rect.top,rect.right-rect.left,600,
				hwnd,IDC_LIST_BOX,ghinstance,0);
			DestroyWindow(GetDlgItem(hwnd,IDC_USER_EDIT));
			if(hlist){
				int i;
				for(i=0;i<sizeof(fonts)/sizeof(char*);i++)
					SendMessage(hlist,CB_ADDSTRING,0,fonts[i]);
				SendMessage(hlist,CB_SETCURSEL,selected_font,0);
				SetFocus(hlist);
			}
			SetWindowText(hwnd,"Select font");
			if(hparent){
				RECT rparent={0};
				GetWindowRect(hparent,&rparent);
				GetWindowRect(hwnd,&rect);
				SetWindowPos(hwnd,0,
					rparent.right-(rect.right-rect.left),
					rparent.top,0,0,SWP_NOZORDER|SWP_NOSIZE);
			}
		}
		break;
	case WM_QUIT:
	case WM_CLOSE:
		EndDialog(hwnd,0);
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam)){
		case IDOK:
		case IDCANCEL:
			EndDialog(hwnd,0);
			break;
		case IDC_USER_EDIT+1:
			if(HIWORD(wparam)==CBN_SELCHANGE){
				selected_font=SendMessage(hlist,CB_GETCURSEL,0,0);
				if(selected_font==0)
					fontname=0;
				else
					fontname=fonts[selected_font];
				InvalidateRect(hparent,NULL,TRUE);
			}
			break;
		}
		break;
	}
	return 0;
}
static WNDPROC old_win_proc=0;
static LRESULT CALLBACK subclass_proc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	switch(msg){
	case WM_KEYFIRST:
		SendMessage(GetParent(hwnd),WM_APP,LOWORD(wparam),0);
		break;
	}
	return CallWindowProc(old_win_proc,hwnd,msg,wparam,lparam);
}

BOOL CALLBACK art_viewer(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static HWND grippy=0;
	static int line=0;
	static int vlines=30;
	static int view_utf8=0;
	PAINTSTRUCT ps;
	HDC hdc;
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
		set_title(hwnd,view_utf8,default_color,line);
		calc_scrollbar(hwnd,line);
		default_color=0;
		view_utf8=0;
		color_lookup[0]=GetSysColor(COLOR_BACKGROUND);
		color_lookup[MIRC_BG]=GetSysColor(COLOR_BACKGROUND);
		color_lookup[MIRC_FG]=GetSysColor(COLOR_WINDOWTEXT);
		old_win_proc=SetWindowLong(GetDlgItem(hwnd,IDC_SCROLLBAR),GWL_WNDPROC,subclass_proc);
		create_vga_font();
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
	case WM_APP:
		switch(wparam){
		case VK_F2:
			view_utf8^=1;
			set_title(hwnd,view_utf8,default_color,line);
			InvalidateRect(hwnd,NULL,TRUE);
			break;
		case ' ':
		case VK_F3:
			if(view_utf8)
				DialogBoxParam(ghinstance,MAKEINTRESOURCE(IDD_USER_INPUT),hwnd,select_font,hwnd);
			break;
		}
		break;
	case WM_RBUTTONDOWN:
	case WM_HELP:
		default_color^=1;
		if(default_color){
			color_lookup[0]=0xFFFFFF;
			color_lookup[MIRC_BG]=0xFFFFFF;
			color_lookup[MIRC_FG]=0;
		}
		else{
			color_lookup[0]=GetSysColor(COLOR_BACKGROUND);
			color_lookup[MIRC_BG]=GetSysColor(COLOR_BACKGROUND);
			color_lookup[MIRC_FG]=GetSysColor(COLOR_WINDOWTEXT);
		}
		set_title(hwnd,view_utf8,default_color,line);
		InvalidateRect(hwnd,NULL,TRUE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam)){
		case IDCANCEL:
			free_vga_font();
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
			set_title(hwnd,view_utf8,default_color,line);
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
			set_title(hwnd,view_utf8,default_color,line);
			calc_scrollbar(hwnd,line);
			InvalidateRect(hwnd,NULL,TRUE);
		}
		break;
	case WM_ERASEBKGND:
		return TRUE;
	case WM_PAINT:
		hdc=BeginPaint(hwnd,&ps);
		if(view_utf8)
			draw_unicode(hdc,line,vlines);
		else
			draw_edit_art(hdc,line,vlines);
		EndPaint(hwnd,&ps);
		break;
	case WM_CLOSE:
	case WM_QUIT:
		free_vga_font();
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