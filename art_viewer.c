#define _WIN32_WINNT 0x400
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "resource.h"
#include "vga737.h"

extern HINSTANCE ghinstance;
static HWND hstatic;
int color_lookup[16]={
	0x000000,
	0xFFFFFF,
	0x68372B,
	0x70A4B2,
	0x6F3D86,
	0x588D43,
	0x352879,
	0xB8C76F,
	0x6F4F25,
	0x433900,
	0x9A6759,
	0x444444,
	0x6C6C6C,
	0x9AD284,
	0x6C5EB5,
	0x959595
};
enum{MIRC_BOLD=2,MIRC_COLOR=3,MIRC_UNDERLINE=31,MIRC_REVERSE=22,MIRC_PLAIN=15};

int draw_char(HDC hdc,char a,int x,int y,int cf,int cb)
{
	int i,j;
	char *p=vga737_bin+a*12;
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
int draw_edit_art(HDC hdc,int line)
{
	char str[1024];
	int i,cpy,x,y,state=0,count=0;
	unsigned char cf,cb;
	cf=1;
	cb=0;
	x=y=0;
	for(i=0;i<50;i++){
		str[0]=sizeof(str)-1;
		str[1]=(sizeof(str)-1)>>8;
		str[sizeof(str)-1]=0;
		cpy=SendMessage(hstatic,EM_GETLINE,line+i,str);
		if(cpy>0 && cpy<sizeof(str)){
			int j;
			//printf("%s\n",str);
			for(j=0;j<cpy;j++){
				if(j==0 && i>0){
					if(str[j]=='<'){// || str[j]=='*'){
						cf=1;
						cb=0;
						x=0;
						y+=12;
					}
				}
				if(str[j]=='\r')
					continue;
				else if(str[j]==0)
					continue;
				else if(str[j]==MIRC_COLOR){
					state=1;
					count=0;
				}
				else if(str[j]>=' '){
					switch(state){
					case 0:
do_draw:
						draw_char(hdc,str[j],x,y,color_lookup[cf%16],color_lookup[cb%16]);
						x+=8;
						count=0;
						break;
					case 1:
						if(str[j]>='0' && str[j]<='9'){
							if(count==0)
								cf=0;
							cf*=10;
							cf+=str[j]-'0';
							if(cf==99)
								cf=1;
							count++;
							if(count>=2){
								if(str[j+1]!=',')
									state=0;
							}
						}
						else if(str[j]==','){
							state=2;
							count=0;
						}
						else{
							state=0;
							goto do_draw;
						}
						break;
					case 2:
						if(str[j]>='0' && str[j]<='9'){
							if(count==0)
								cb=0;
							cb*=10;
							cb+=str[j]-'0';
							if(cb==99)
								cb=0;
							count++;
							if(count>=2)
								state=0;
						}
						else{
							state=0;
							goto do_draw;
						}
						break;

					}
				}
			}
		}
	}
}
static int set_title(HWND hwnd,int line)
{
	char str[80];
	_snprintf(str,sizeof(str),"Ascii art  %u",line);
	SetWindowText(hwnd,str);
	return 0;
}
BOOL CALLBACK art_viewer(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static HWND grippy=0;
	static DWORD tick;
	static int line=0;
	PAINTSTRUCT ps;
	HDC hdc;
	if(FALSE)
	if(msg!=WM_MOUSEFIRST&&msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE/*&&msg!=WM_NOTIFY*/)
	//if(msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE)
	{
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
			
		}
		set_title(hwnd,line);
		break;
	case WM_SIZE:
		{
		RECT rect={0};
		grippy_move(hwnd,grippy);
		if(GetClientRect(hwnd,&rect)!=0){
			int x,y,w,h;
			x=rect.right-20;
			y=rect.top-20;
			w=20;
			h=rect.bottom-rect.top;
			SetWindowPos(GetDlgItem(hwnd,IDC_SCROLLBAR),NULL,x,y,w,h,SWP_NOZORDER);
		}
		}
		break;
	case WM_COMMAND:
		switch(wparam){
		case IDCANCEL:
			EndDialog(hwnd,0);
			break;
		}
		break;
	case WM_MOUSEWHEEL:
		{
			short y=HIWORD(wparam);
			int modifier=(LOWORD(wparam)&MK_RBUTTON)?10:1;
			if(y>0){
				if(line<3)
					line=0;
				else
					line-=3*modifier;
			}
			else
				line+=3*modifier;
			InvalidateRect(hwnd,NULL,TRUE);
			set_title(hwnd,line);
		}
		break;
	case WM_PAINT:
		hdc=BeginPaint(hwnd,&ps);
		draw_edit_art(hdc,line);
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