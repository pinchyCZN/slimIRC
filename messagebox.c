#include <windows.h>
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