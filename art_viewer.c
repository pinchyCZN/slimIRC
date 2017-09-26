#if _WIN32_WINNT<0x400
#define _WIN32_WINNT 0x400
#endif
#include <windows.h>
#include <richedit.h>
#include <stdlib.h>
#include <stdio.h>
#include "resource.h"
#include "vga737.h"
#include "unicode_font.h"

extern HINSTANCE ghinstance;
static HWND hstatic;
int client_width=0,client_height=0;
int default_color=FALSE;
char *vgargb=0;
char *fontname=0;

#define MIRC_MAX_COLORS 15
#define MAX_COLOR_LOOKUP 18
int color_lookup[MAX_COLOR_LOOKUP]={
	//RRGGBB
	0xFFFFFF, //0 white
	0x000000, //1 black
	0x00007F, //2 blue (navy)
	0x009300, //3 green
	0xFF0000, //4 red
	0x7F0000, //5 brown (maroon)
	0x9C009C, //6 purple
	0xFC7F00, //7 orange (olive)
	0xFFFF00, //8 yellow
	0x00FC00, //9 light green (lime)
	0x009393, //10 teal (a green/blue cyan)
	0x00FFFF, //11 light cyan (cyan) (aqua)
	0x0000FC, //12 light blue (royal)
	0xFF00FF, //13 pink (light purple) (fuchsia)
	0x7F7F7F, //14 grey
	0xD2D2D2, //15 light grey (silver)
	0x000000, //16 windows background
	0x00FF00  //17 windows foreground
};
enum{MIRC_BOLD=2,MIRC_COLOR=3,MIRC_UNDERLINE=31,MIRC_REVERSE=22,MIRC_PLAIN=15,MIRC_BG=16,MIRC_FG=17};
int get_RGB(DWORD bgr)
{
	BYTE r,g,b;
	r=bgr;
	g=bgr>>8;
	b=bgr>>16;
	return (r<<16)|(g<<8)|b;
}
int get_BGR(DWORD rgb)
{
	BYTE r,g,b;
	r=rgb>>16;
	g=rgb>>8;
	b=rgb;
	return (b<<16)|(g<<8)|r;
}

int draw_char(HDC hdc,unsigned char *font,unsigned char a,int x,int y,int cf,int cb)
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
	bmi.colors[0]=cb;
	bmi.colors[1]=cf;
	SetDIBitsToDevice(hdc,x,y,8,12,
		0,0, //src xy
		0,12, //startscan,scanlines
		font+a*12*8,
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
int is_utf8(BYTE a,int *len)
{
	int result=FALSE;
	if((a&0xE0)==0xC0){
		*len=2;
		result=TRUE;
	}else if((a&0xF0)==0xE0){
		*len=3;
		result=TRUE;
	}else if((a&0xF8)==0xF0){
		*len=4;
		result=TRUE;
	}
	return result;
}
int get_utf8_code(int len,BYTE *data)
{
	int result=0;
	switch(len){
	case 2:
		result=((data[0]&0x1F)<<6)|(data[1]&0x3F);
		break;
	case 3:
		result=((data[0]&0x0F)<<12)|((data[1]&0x3F)<<6)|(data[2]&0x3F);
		break;
	case 4:
		result=((data[0]&0x07)<<18)|((data[1]&0x3F)<<12)|((data[2]&0x3F)<<6)|(data[3]&0x3F);
		break;
	}
	return result;
}
int convert_utf8(int code)
{
	int result=code&0xFF;
	int i;
	//DOS CP / UTF8
	static WORD utf8_table[]={
		0x80,0x00C7,
		0x81,0x00FC,
		0x82,0x00E9,
		0x83,0x00E2,
		0x84,0x00E4,
		0x85,0x00E0,
		0x86,0x00E5,
		0x87,0x00E7,
		0x88,0x00EA,
		0x89,0x00EB,
		0x8A,0x00E8,
		0x8B,0x00EF,
		0x8C,0x00EE,
		0x8D,0x00EC,
		0x8E,0x00C4,
		0x8F,0x00C5,
		0x90,0x00C9,
		0x91,0x00E6,
		0x92,0x00C6,
		0x93,0x00F4,
		0x94,0x00F6,
		0x95,0x00F2,
		0x96,0x00FB,
		0x97,0x00F9,
		0x98,0x00FF,
		0x99,0x00D6,
		0x9A,0x00DC,
		0x9B,0x00A2,
		0x9C,0x00A3,
		0x9D,0x00A5,
		0x9E,0x20A7,
		0x9F,0x0192,
		0xA0,0x00E1,
		0xA1,0x00ED,
		0xA2,0x00F3,
		0xA3,0x00FA,
		0xA4,0x00F1,
		0xA5,0x00D1,
		0xA6,0x00AA,
		0xA7,0x00BA,
		0xA8,0x00BF,
		0xA9,0x2310,
		0xAA,0x00AC,
		0xAB,0x00BD,
		0xAC,0x00BC,
		0xAD,0x00A1,
		0xAE,0x00AB,
		0xAF,0x00BB,
		0xB0,0x2591,
		0xB1,0x2592,
		0xB2,0x2593,
		0xB3,0x2502,
		0xB4,0x2524,
		0xB5,0x2561,
		0xB6,0x2562,
		0xB7,0x2556,
		0xB8,0x2555,
		0xB9,0x2563,
		0xBA,0x2551,
		0xBB,0x2557,
		0xBC,0x255D,
		0xBD,0x255C,
		0xBE,0x255B,
		0xBF,0x2510,
		0xC0,0x2514,
		0xC1,0x2534,
		0xC2,0x252C,
		0xC3,0x251C,
		0xC4,0x2500,
		0xC5,0x253C,
		0xC6,0x255E,
		0xC7,0x255F,
		0xC8,0x255A,
		0xC9,0x2554,
		0xCA,0x2569,
		0xCB,0x2566,
		0xCC,0x2560,
		0xCD,0x2550,
		0xCE,0x256C,
		0xCF,0x2567,
		0xD0,0x2568,
		0xD1,0x2564,
		0xD2,0x2565,
		0xD3,0x2559,
		0xD4,0x2558,
		0xD5,0x2552,
		0xD6,0x2553,
		0xD7,0x256B,
		0xD8,0x256A,
		0xD9,0x2518,
		0xDA,0x250C,
		0xDB,0x2588,
		0xDC,0x2584,
		0xDD,0x258C,
		0xDE,0x2590,
		0xDF,0x2580,
		0xE0,0x03B1,
		0xE1,0x00DF,
		0xE2,0x0393,
		0xE3,0x03C0,
		0xE4,0x03A3,
		0xE5,0x03C3,
		0xE6,0x00B5,
		0xE7,0x03C4,
		0xE8,0x03A6,
		0xE9,0x0398,
		0xEA,0x03A9,
		0xEB,0x03B4,
		0xEC,0x221E,
		0xED,0x03C6,
		0xEE,0x03B5,
		0xEF,0x2229,
		0xF0,0x2261,
		0xF1,0x00B1,
		0xF2,0x2265,
		0xF3,0x2264,
		0xF4,0x2320,
		0xF5,0x2321,
		0xF6,0x00F7,
		0xF7,0x2248,
		0xF8,0x00B0,
		0xF9,0x2219,
		0xFA,0x00B7,
		0xFB,0x221A,
		0xFC,0x207F,
		0xFD,0x00B2,
		0xFE,0x25A0,
		0xFF,0x00A0
	};
	for(i=0;i<sizeof(utf8_table)/sizeof(WORD);i+=2){
		if(code==utf8_table[i+1]){
			result=utf8_table[i]&0xFF;
			break;
		}
	}
	return result;
}
__inline void draw_char_color(HDC hdc,unsigned char *font,int cf,int cb,
							int x,int y,unsigned char current_char)
{
	int fg=cf,bg=cb;
	if(fg==bg && (!default_color)){
		if(bg==0 || bg==1){ //black or white
			bg=MIRC_BG;fg=MIRC_FG;
		}
	}
	draw_char(hdc,font,current_char,x,y,
		color_lookup[fg%MAX_COLOR_LOOKUP],
		color_lookup[bg%MAX_COLOR_LOOKUP]);
	return;
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
	for(i=0;i<1000;i++){
		memset(str,0,sizeof(str));
		str[0]=sizeof(str)-1;
		str[1]=(sizeof(str)-1)>>8;
		str[sizeof(str)-1]=0;
		cpy=SendMessage(hstatic,EM_GETLINE,line+i,str);
		if(cpy>0){
			int j;
			int utf8_len,utf8_count;
			BYTE utf8_block[4];
			if(cpy>sizeof(str))
				cpy=sizeof(str);
			//printf("%s\n",str);
			for(j=0;j<cpy;j++){
				BYTE current_char;
				current_char=str[j];
				if(current_char=='\r'){
					cf=MIRC_FG;
					cb=MIRC_BG;
					x=0;
					y+=12;
					out_line++;
					if(out_line>=line_count)
						return 0;
					continue;
				}
				else if(current_char==0){
					state=0;
					count=0;
					continue;
				}
				else if(current_char==MIRC_REVERSE){
					state=0;
					cf=MIRC_BG;
					cb=MIRC_FG;
				}
				else if(current_char==MIRC_COLOR){
					state=1;
					count=0;
				}
				else if(current_char==MIRC_PLAIN){
					state=0;
					cf=MIRC_FG;
					cb=MIRC_BG;
				}
				else{
					if(current_char<' ')
						current_char=' ';
					switch(state){
					case 0:
do_draw:
						{
							utf8_len=0;
							if(is_utf8(current_char,&utf8_len)){
								utf8_block[0]=current_char;
								utf8_count=1;
								state=4;
							}
							else{
								draw_char_color(hdc,vgargb,cf,cb,x,y,current_char);
								x+=8;
							}
						}
						count=0;
						break;
					case 1:
						if(isdigit(current_char)){
							if(count==0)
								cf=0;
							cf*=10;
							cf+=current_char-'0';
							if(cf>MIRC_MAX_COLORS)
								cf=MIRC_FG;
							count++;
							if(count>=3){
								count=0;
								state=0;
							}
						}
						else if(current_char==','){
							count=0;
							state=3;
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
						if(isdigit(current_char)){
							state=3;
							count=0;
						}
						else{
							state=0;
							goto do_draw;
						}
						break;
					case 3:
						if(isdigit(current_char)){
							if(count==0)
								cb=0;
							cb*=10;
							cb+=current_char-'0';
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
					case 4:
						{
							utf8_block[utf8_count]=current_char;
							utf8_count++;
							if(utf8_count>=utf8_len){
								unsigned char *font=vgargb;
								int code=get_utf8_code(utf8_len,utf8_block);
								if((code>=0x2580) && (code<=0x259F)){
									current_char=code-0x2580;
									font=vgargb+8*12*256;
								}
								else
									current_char=convert_utf8(code);
								draw_char_color(hdc,font,cf,cb,x,y,current_char);
								x+=8;
								state=0;
							}
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
		else
			break;
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
		SetBkColor(hdc,get_BGR(color_lookup[0]));
		SetTextColor(hdc,get_BGR(color_lookup[1]));
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
			len=MultiByteToWideChar(CP_UTF8,0,str,cpy,tmp,sizeof(tmp)/sizeof(WCHAR));
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
	const int vga_count=(sizeof(vga737_bin)/12);
	const int total=vga_count+(sizeof(block_elements)/12);
	if(vgargb==0)
		vgargb=malloc(8*12*total);
	if(vgargb){
		int i,j,k;
		for(k=0;k<total;k++){
			for(i=0;i<12;i++){
				for(j=0;j<8;j++){
					int c;
					char *font=vga737_bin;
					char *p;
					int offset=k*12;
					if(k>=vga_count){
						font=block_elements;
						offset=(k-vga_count)*12;
					}
					p=font+offset;
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
	static char *fonts[6]={"Default","Consolas","Arial","Tahoma","Verdana","MS Serif"};

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
		calc_scrollbar(hwnd,line);
		default_color=0;
		view_utf8=0;
		color_lookup[0]=get_RGB(GetSysColor(COLOR_BACKGROUND));
		color_lookup[MIRC_BG]=get_RGB(GetSysColor(COLOR_BACKGROUND));
		color_lookup[MIRC_FG]=get_RGB(GetSysColor(COLOR_WINDOWTEXT));
		old_win_proc=SetWindowLong(GetDlgItem(hwnd,IDC_SCROLLBAR),GWL_WNDPROC,subclass_proc);
		create_vga_font();
		set_title(hwnd,view_utf8,default_color,line);
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
		case 'H':
			MessageBox(hwnd,"F5=update main window pos\r\n"
							"F2=UTF8 view\r\n"
							"F1=change default color",
							"HELP",MB_OK|MB_SYSTEMMODAL);
			break;
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
		case VK_F5:
			{
				int pos=SendMessage(hstatic,EM_GETFIRSTVISIBLELINE,0,0);
				pos=line-pos;
				SendMessage(hstatic,EM_LINESCROLL,0,pos);
			}
			break;
		}
		break;
	case WM_HELP:
		default_color^=1;
		if(default_color){
			color_lookup[0]=0xFFFFFF;
			color_lookup[MIRC_BG]=0xFFFFFF;
			color_lookup[MIRC_FG]=0;
		}
		else{
			color_lookup[0]=get_RGB(GetSysColor(COLOR_BACKGROUND));
			color_lookup[MIRC_BG]=get_RGB(GetSysColor(COLOR_BACKGROUND));
			color_lookup[MIRC_FG]=get_RGB(GetSysColor(COLOR_WINDOWTEXT));
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
			case SB_PAGEDOWN:dir=1;modifier=vlines;break;
			case SB_PAGEUP:dir=-1;modifier=vlines;break;
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
			int modifier=3;
			int count=SendMessage(hstatic,EM_GETLINECOUNT,0,0);
			if(y>0){
				if(line==0)
					break;
				dir=-1;
			}
			else
				dir=1;
			if(LOWORD(wparam)&(MK_RBUTTON|MK_SHIFT))
				modifier=vlines;
			else if(wparam&MK_CONTROL)
				modifier=1;
			line+=dir*modifier;
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