#include <windows.h>
#include <commctrl.h>
#include "resource.h"


char *message_ptr=0;
char *title_ptr=0;
RECT prect={0};
BOOL CALLBACK message_box(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	RECT rect;
	int width,height;
	int xpos=0,ypos=0;
	switch(msg)
	{
	case WM_INITDIALOG:
		if(title_ptr!=0)
			SetWindowText(hwnd,title_ptr);
		if(message_ptr!=0)
			SetWindowText(GetDlgItem(hwnd,IDC_MESSAGETEXT),message_ptr);
		GetWindowRect(hwnd,&rect);
		width=rect.right-rect.left;
		height=rect.bottom-rect.top;
		xpos=(prect.right+prect.left-width)/2;
		ypos=(prect.bottom+prect.top-height)/2;
		SetWindowPos(hwnd,NULL,xpos,ypos,0,0,SWP_NOZORDER|SWP_NOSIZE);
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		case WM_DESTROY:
			EndDialog(hwnd,IDCANCEL);
			break;
		case IDOK:
			EndDialog(hwnd,IDOK);
			break;
		}
		break;
	case WM_CLOSE:
	case WM_QUIT:
		EndDialog(hwnd,IDCANCEL);
		break;
	}
	return 0;
}

int show_messagebox(HWND hwnd,char *message,char *title,int type)
{
	extern HINSTANCE ghinstance;
	GetWindowRect(hwnd,&prect);
	message_ptr=message;
	title_ptr=title;
	return DialogBox(ghinstance,MAKEINTRESOURCE(IDD_MESSAGEBOX),hwnd,message_box);
}

char input_str[80];
BOOL CALLBACK user_input(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	RECT rect;
	int width,height;
	int xpos=0,ypos=0;
	int quitcode=IDCANCEL;
	switch(msg)
	{
	case WM_INITDIALOG:
		if(title_ptr!=0)
			SetWindowText(hwnd,title_ptr);
		GetWindowRect(hwnd,&rect);
		width=rect.right-rect.left;
		height=rect.bottom-rect.top;
		xpos=(prect.right+prect.left-width)/2;
		ypos=(prect.bottom+prect.top-height)/2;
		SetWindowPos(hwnd,NULL,xpos,ypos,0,0,SWP_NOZORDER|SWP_NOSIZE);
		SendDlgItemMessage(hwnd,IDC_USER_EDIT,EM_LIMITTEXT,40,0);
		SetFocus(GetDlgItem(hwnd,IDC_USER_EDIT));
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		case WM_DESTROY:
			goto quit;
			break;
		case IDOK:
			input_str[0]=0;
			GetWindowText(GetDlgItem(hwnd,IDC_USER_EDIT),input_str,sizeof(input_str));
			quitcode=IDOK;
			goto quit;
			break;
		}
		break;
	case WM_CLOSE:
	case WM_QUIT:
quit:
		EndDialog(hwnd,quitcode);
		break;
	}
	return 0;
}
int show_user_input(HWND hwnd,char *title,char *out,int size)
{
	extern HINSTANCE ghinstance;
	int ret;
	GetWindowRect(hwnd,&prect);
	title_ptr=title;
	ret=DialogBox(ghinstance,MAKEINTRESOURCE(IDD_USER_INPUT),hwnd,user_input);
	if(ret==IDOK)
		strncpy(out,input_str,size);
	return ret;
}

static HWND hwndTT=0;
static char tt_text[1024]={0};
int hide_tooltip()
{
	extern HWND ghmainframe;
	PostMessage(ghmainframe,WM_USER+2,0,0);
	return TRUE;
}
int show_tooltip(char *msg,int x,int y)
{
	extern HWND ghmainframe;
	strncpy(tt_text,msg,sizeof(tt_text));
	tt_text[sizeof(tt_text)-1]=0;
	PostMessage(ghmainframe,WM_USER+1,x,y);
	return TRUE;
}
int create_tooltip(HWND hwnd,int x, int y)
{
	TOOLINFO ti;
	if((hwndTT==0) && (tt_text[0]!=0)){
		hwndTT=CreateWindowEx(WS_EX_TOPMOST,
			TOOLTIPS_CLASS,NULL,
			WS_POPUP|TTS_NOPREFIX|TTS_ALWAYSTIP,        
			CW_USEDEFAULT,CW_USEDEFAULT,
			CW_USEDEFAULT,CW_USEDEFAULT,
			hwnd,NULL,NULL,NULL);
		if(hwndTT!=0){
			ti.cbSize = sizeof(TOOLINFO);
			ti.uFlags = TTF_IDISHWND|TTF_TRACK|TTF_ABSOLUTE;
			ti.hwnd = hwndTT;
			ti.uId = hwndTT;
			ti.lpszText = tt_text;
			SendMessage(hwndTT,TTM_ADDTOOL,0,&ti);
			SendMessage(hwndTT,TTM_UPDATETIPTEXTA,0,&ti);
			SendMessage(hwndTT,TTM_TRACKPOSITION,0,MAKELONG(x,y)); 
			SendMessage(hwndTT,TTM_TRACKACTIVATE,TRUE,&ti);
		}
	}
	return hwndTT;
}
int destroy_tooltip()
{
	if(hwndTT!=0){
		DestroyWindow(hwndTT);
		hwndTT=0;
	}
	return hwndTT;
}