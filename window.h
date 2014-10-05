/*
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#include "resource.h"
extern HWND ghmdiclient,ghswitchbar,ghinstance;
*/
int move_console()
{
	char title[MAX_PATH]={0}; 
	HWND hcon; 
	GetConsoleTitle(title,sizeof(title));
	if(title[0]!=0){
		hcon=FindWindow(NULL,title);
		SetWindowPos(hcon,0,600,0,800,600,SWP_NOZORDER);
	}
	return 0;
}
void open_console()
{
	char title[MAX_PATH]={0}; 
	HWND hcon; 
	FILE *hf;
	static BYTE consolecreated=FALSE;
	static int hcrt=0;
	
	if(consolecreated==TRUE)
	{
		GetConsoleTitle(title,sizeof(title));
		if(title[0]!=0){
			hcon=FindWindow(NULL,title);
			ShowWindow(hcon,SW_SHOW);
		}
		hcon=(HWND)GetStdHandle(STD_INPUT_HANDLE);
		FlushConsoleInputBuffer(hcon);
		return;
	}
	AllocConsole(); 
	hcrt=_open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE),_O_TEXT);

	fflush(stdin);
	hf=_fdopen(hcrt,"w"); 
	*stdout=*hf; 
	setvbuf(stdout,NULL,_IONBF,0);
	GetConsoleTitle(title,sizeof(title));
	if(title[0]!=0){
		hcon=FindWindow(NULL,title);
		ShowWindow(hcon,SW_SHOW); 
		SetForegroundWindow(hcon);
	}
	consolecreated=TRUE;
}
void hide_console()
{
	char title[MAX_PATH]={0}; 
	HANDLE hcon; 
	
	GetConsoleTitle(title,sizeof(title));
	if(title[0]!=0){
		hcon=FindWindow(NULL,title);
		ShowWindow(hcon,SW_HIDE);
		SetForegroundWindow(hcon);
	}
}
int create_mdi_window(HWND hclient,HINSTANCE hinstance,IRC_WINDOW *win)
{
	HWND hstatic,hedit,hlist=0,hscroll_lock;
	HMENU hmenu;
	if(win==0)
		return FALSE;
    hscroll_lock=CreateWindowEx(WS_EX_TOPMOST,"button","", 
      WS_CHILD|WS_VISIBLE|BS_ICON,
        0, 0, 0, 0, hclient, MDI_SCROLL_LOCK, hinstance, 0);

	if(hscroll_lock){
		static HICON hicon=0;
		if(hicon==0)
			hicon=LoadImage(ghinstance,MAKEINTRESOURCE(IDI_LOCK),IMAGE_ICON,12,12,NULL);
		if(hicon)
			SendMessage(hscroll_lock,BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)hicon);
	}

	hstatic=CreateWindowEx(WS_EX_STATICEDGE,RICHEDIT_CLASS,"",
		WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|
		//SS_LEFT, //|SS_SUNKEN|SS_NOPREFIX, ////|SS_CENTER|SS_RIGHT|, //|SS_GRAYFRAME,
		WS_VSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY,
		0,0,0,0,hclient,MDI_STATIC,hinstance,0);

    hedit=CreateWindowEx(WS_EX_STATICEDGE,"edit","", 
      WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_TABSTOP|
	  ES_AUTOHSCROLL|ES_AUTOVSCROLL|ES_MULTILINE|ES_WANTRETURN,
        0, 0, 0, 0, hclient, MDI_EDIT, hinstance, 0);

	if(win->type==CHANNEL_WINDOW){
		char str[80];
		hlist=CreateWindowEx(WS_EX_STATICEDGE,"listbox","", 
		  WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_VSCROLL|
		  LBS_NOTIFY|LBS_SORT|LBS_EXTENDEDSEL|LBS_WANTKEYBOARDINPUT|LBS_NOINTEGRALHEIGHT|LBS_HASSTRINGS,
			0, 0, 0, 0, hclient, MDI_LIST, hinstance, 0);
		hmenu=GetSystemMenu(hclient,FALSE);
		_snprintf(str,sizeof(str),"%s.%s.log (ctrl=explore) context menu",win->channel,win->network);
		InsertMenu(hmenu,0,MF_BYPOSITION|MF_STRING,MDI_MENU_OPENLOG,str);
		InsertMenu(hmenu,1,MF_BYPOSITION|MF_SEPARATOR,0,0);
	}
	win->hwnd=hclient;
	win->hstatic=hstatic;
	win->hlist=hlist;
	win->hedit=hedit;
	win->hscroll_lock=hscroll_lock;
	resize_mdi_window(hclient);
	return TRUE;
}
int reposition_scroll(IRC_WINDOW *win,int step)
{
	static int scroll_bottom=FALSE;
	SCROLLINFO si;
	if(step==0){
		si.cbSize=sizeof(si);
		si.fMask=SIF_ALL; //SIF_RANGE|SIF_PAGE|SIF_POS;
		GetScrollInfo(win->hstatic,SB_VERT,&si);
		if((si.nMax-si.nPage-si.nPos)<=(si.nPage/2))
			scroll_bottom=TRUE;
		else
			scroll_bottom=FALSE;
	}
	//else if(scroll_bottom)
	if(!win->scroll_free)
		SendMessage(win->hstatic,WM_VSCROLL,SB_BOTTOM,0);
	return scroll_bottom;
}
int resize_mdi_window(HWND hclient)
{
	extern short chat_anchor_list[];
	extern short server_anchor_list[];
	IRC_WINDOW *win;
	win=find_window_by_hwnd(hclient);
	if(win!=0){
//		reposition_scroll(win,0);
		if(win->type==CHANNEL_WINDOW)
			reposition_controls(hclient,chat_anchor_list);
		else
			reposition_controls(hclient,server_anchor_list);
		reposition_scroll(win,1);
		return TRUE;
	}
	return FALSE;
}

int create_mdiclient(HWND hwnd,HMENU hmenu,HINSTANCE hinstance)
{
	CLIENTCREATESTRUCT MDIClientCreateStruct;
	HWND hmdiclient=0;
	MDIClientCreateStruct.hWindowMenu   = GetSubMenu(hmenu,5);
	MDIClientCreateStruct.idFirstChild  = 50000;
	hmdiclient = CreateWindow("MDICLIENT",NULL,
		WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_VSCROLL|WS_HSCROLL|WS_VISIBLE, //0x56300001
		0,0,0,0,
		hwnd,
		IDC_MDI_CLIENT,//ID
		hinstance,
		(LPVOID)&MDIClientCreateStruct);
	return hmdiclient;
}
int get_window_classname(char **s)
{
	static char *cname="slimIRC_mdiframe";
	if(s)
		*s=cname;
	return 0;
}
int create_mainwindow(void *wndproc,HMENU hmenu,HINSTANCE hinstance)
{
	WNDCLASS wndclass;
	HWND hframe=0;
	char *class_name="mdiframe";
	memset(&wndclass,0,sizeof(wndclass));
	get_window_classname(&class_name);

	wndclass.style=0; //CS_HREDRAW|CS_VREDRAW;
	wndclass.lpfnWndProc=wndproc;
	wndclass.hCursor=LoadCursor(NULL, IDC_ARROW);
	wndclass.hInstance=hinstance;
	wndclass.hbrBackground=COLOR_BTNFACE+1;
	wndclass.lpszClassName=class_name;
	
	if(RegisterClass(&wndclass)!=0){	
		hframe = CreateWindow(class_name,"slimIRC",
			WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_OVERLAPPEDWINDOW, //0x6CF0000
			0,0,
			400,300,
			NULL,           // handle to parent window
			hmenu,
			hinstance,
			NULL);
	}
	return hframe;
}
