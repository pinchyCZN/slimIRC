#if _WIN32_WINNT<0x400
#define _WIN32_WINNT 0x400
#define WINVER 0x500
#endif
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <richedit.h>
#include <math.h>
#include <assert.h>
#include "libircclient.h"
#include "resource.h"

extern HINSTANCE ghinstance;
extern HWND ghmainframe,ghmdiclient,ghswitchbar;
typedef struct{
	irc_session_t *session;
	char server[80];
	char network[80];
	int port;
	char password[40];
	char nick[20];
	char user[40];
	int thread_started;
	int disconnected;
}SERVER_THREAD;
static SERVER_THREAD server_threads[20];

typedef struct{
	int type;
	char network[80];
	char server[80];
	char channel[80];
	char nick[20];
	char password[20];
	int port;
	int ssl;
	int dccid;
	void *session;
	int disconnect;
	HWND hwnd,hbutton,hstatic,hlist,hedit,hscroll_lock;
	int button_id,pressed,activity,scroll_free,selecting;
}IRC_WINDOW;

static IRC_WINDOW irc_windows[100];
static HMENU list_menu=0,static_menu=0,url_menu=0;
enum {
	CMD_SLAP=1200,
	CMD_CHANGE_NICK,
	CMD_PRIV_MSG,
	CMD_WHOIS,
	CMD_OP,
	CMD_DEOP,
	CMD_VOICE,
	CMD_DEVOICE,
	CMD_BAN,
	CMD_KICK,
	CMD_KICKBAN,
	CMD_REFRESH_LIST,
	CMD_TSEARCH,
	CMD_CLEAR,
	CMD_CLEARTYPED,
	CMD_CHANMODES,
	CMD_OPENURL,
	CMD_COPYURL,
	CMD_TEST,
};
#define MAX_EDIT_LENGTH (512+2)
#define MAX_EDIT_HISTORY 40
static char edit_buffer[MAX_EDIT_HISTORY][MAX_EDIT_LENGTH];
static int scroll_history_pos=0;
static int buffer_pos=0;
static char mouse_target[512]={0};
int tab_continue=FALSE,tab_pos=0;
char tab_word[20]={0};
#include "chan_modes.h"
#include "static_window.h"
#include "window.h"
#include "file_logging.h"
#include "help.h"

int control_debug(int type,char *name,char *set)
{
	int flag=FALSE;
	int readonly=TRUE;
	int i;
	typedef struct{
		char *name;
		int type;
		int val;
	}debug;
	static debug list[]={
		{"switchbar",IDC_SWITCHBAR,FALSE},
		{"mdi",IDC_MDI_CLIENT,FALSE},
		{"mainmsg",IDC_MAIN_MSG_PUMP,FALSE},
		{"mdistatic",MDI_STATIC,FALSE},
		{"wndproc",IDC_TOP_WNDPROC,FALSE}};
#ifndef _DEBUG
	return FALSE;
#endif
	if(set!=0){
		if(stricmp(set,"on")==0){flag=TRUE;readonly=FALSE;}
		else if(stricmp(set,"off")==0){flag=FALSE;readonly=FALSE;}
	}
	for(i=0;i<sizeof(list)/sizeof(debug);i++){
		if(type==0){
			if(name==0)
				break;
			if(name[0]=='?' && name[1]==0)
				printf("%s %s\n",list[i].name,list[i].val?"ON":"OFF");
			if(stricmp(name,"all")==0){
				if(!readonly)
					list[i].val=flag;
			}
			if(stricmp(name,list[i].name)==0){
				if(readonly)
					return list[i].val;
				list[i].val=flag;
				return list[i].val;
			}
		}else{
			if(type==list[i].type)
				return list[i].val;
		}
	}
	return FALSE;
}
int strstri(const char *s1,const char *s2)
{
	int i,j,k;
	for(i=0;s1[i];i++)
		for(j=i,k=0;tolower(s1[j])==tolower(s2[k]);j++,k++)
			if(!s2[k+1])
				return (s1+i);
	return NULL;
}
int handle_debug(char *s)
{
	int opt=0;
	char p1[7]={0},p2[10]={0},p3[7]={0};
	char *str=strstri(s,"debug ");
	if(str!=0){
		if(get_subitem(str,p1,sizeof(p1),0))opt|=1;
		if(get_subitem(str,p2,sizeof(p2),1))opt|=2;
		if(get_subitem(str,p3,sizeof(p3),2))opt|=4;
		if(opt==1 || (opt==3 && p2[0]=='?'))
			control_debug(0,"?",0);
		else if(opt==0x7){
			if(stricmp(p1,"DEBUG")==0){
				control_debug(0,p2,p3);
				if(stricmp(p2,"CONSOLE")==0){
					if(stricmp(p3,"OFF")==0)
						hide_console();
					else if(stricmp(p3,"ON")==0)
						open_console();

				}
				else if(stricmp(p2,"LEVEL")==0){
					if(p3[0]!=0 && p3[0]!='?')
						set_irc_debug_level(atoi(p3));
					printf("irc_debug_level=%i\n",get_irc_debug_level());
				}
			}
		}
	}
	return 0;
}
HWND hmdi_static=0;
char search_text[80]={0};

BOOL CALLBACK text_search(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	FINDTEXT ftext;
	int pos,dir,fpos;
	static int fontheight=0,timer=0,last_search_pos=0,last_dir=0,at_top=FALSE;
#ifdef _DEBUG
	//print_msg(msg,lparam,wparam,hwnd);
#endif
	switch(msg){
	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd,IDC_SEARCH_BOX,EM_LIMITTEXT,80,0);
		{
		RECT rdlg,rstatic;
		POINT p;
		GetWindowRect(hmdi_static,&rstatic);
		GetWindowRect(hwnd,&rdlg);
		p.x=((rstatic.right-rstatic.left)/2)-((rdlg.right-rdlg.left)/2);
		p.y=((rstatic.bottom-rstatic.top)/2)-((rdlg.bottom-rdlg.top)/2);
		ClientToScreen(hmdi_static,&p);
		SetWindowPos(hwnd,NULL,p.x,p.y,0,0,SWP_NOZORDER|SWP_NOSIZE);
		}
		last_search_pos=0;
		at_top=FALSE;
		SetWindowText(GetDlgItem(hwnd,IDC_SEARCH_BOX),search_text);
		SendMessage(GetDlgItem(hwnd,IDC_SEARCH_BOX),EM_SETSEL,0,80);
		if(hmdi_static!=0){
			HDC hdc;
			hdc=GetDC(hmdi_static);
			if(hdc!=0){
				TEXTMETRIC tm;
				if(GetTextMetrics(hdc,&tm)!=0)
					fontheight=tm.tmHeight;
				ReleaseDC(hmdi_static,hdc);
			}
		}
		break;
	case WM_COMMAND:
		//print_msg(msg,lparam,wparam,hwnd);
		switch(LOWORD(wparam))
		{
		case IDC_SEARCH_BOX:
			if(HIWORD(wparam)==EN_CHANGE){
				last_search_pos=0;
				at_top=FALSE;
			}
			break;
		case IDOK:
			if((GetKeyState(VK_CONTROL)&0x8000)||(GetKeyState(VK_SHIFT)&0x8000))
				wparam=IDC_SEARCH_DOWN;
		case IDC_SEARCH_UP:
		case IDC_SEARCH_DOWN:
			if(LOWORD(wparam)==IDC_SEARCH_DOWN)
				dir=FR_DOWN;
			else
				dir=0;
			search_text[0]=0;
			GetWindowText(GetDlgItem(hwnd,IDC_SEARCH_BOX),search_text,sizeof(search_text));
			if(search_text[0]==0)
				break;
			if(last_search_pos==0){
				POINT p;
				if(dir&FR_DOWN){
					if(at_top)
						pos=1;
					else{
						p.x=p.y=0;
						pos=SendMessage(hmdi_static,EM_CHARFROMPOS,0,&p);
					}
				}
				else{
					RECT rect;
					if(at_top){
						pos=0;
					}
					else{
						GetClientRect(hmdi_static,&rect);
						p.x=rect.right-rect.left;
						p.y=rect.bottom-rect.top;
						pos=SendMessage(hmdi_static,EM_CHARFROMPOS,0,&p);
					}
				}
			}
			else{
				pos=last_search_pos;
				if(dir!=last_dir){
					if(dir&FR_DOWN)
						pos++;
					else
						pos--;
				}
				else if(dir&FR_DOWN)
					pos++;

			}
			ftext.lpstrText=search_text;
			ftext.chrg.cpMin=pos;
			if(dir&FR_DOWN)
				ftext.chrg.cpMax=-1;
			else
				ftext.chrg.cpMax=0;
			fpos=SendMessage(hmdi_static,EM_FINDTEXT,dir,&ftext);
			//printf("pos=%i fpos=%i last=%i\n",pos,fpos,last_search_pos);
			if(fpos>=0){
				int line,line2,ltmp;
				RECT rect;
				POINT point;
				GetClientRect(hmdi_static,&rect);
				//printf(" point.x=%i point.y=%i rect.bottom=%i\n",point.x,point.y,rect.bottom);
				line2=SendMessage(hmdi_static,EM_GETFIRSTVISIBLELINE,0,0);
				ltmp=SendMessage(hmdi_static,EM_LINEINDEX,line2,0);
				point.y=0;
				SendMessage(hmdi_static,EM_POSFROMCHAR,&point,ltmp);
				if(point.y<0) //check if first visible line is actually visible
					line2++;
				line=SendMessage(hmdi_static,EM_LINEFROMCHAR,fpos,0);
				point.y=0;
				SendMessage(hmdi_static,EM_POSFROMCHAR,&point,fpos);
				if(line-line2==0){
					if(point.y<0){
						line=0;line2=1;
					}
					else if(point.y>0 && point.y>rect.bottom-fontheight){
						line=1;line2=0;
					}
				}
				else{
					if(point.y>0 && point.y<rect.bottom){
						if(point.y>=rect.bottom-fontheight){
							line=1;line2=0;
						}
						else{
							line=0;line2=0;
						}
					}

				}
				SendMessage(hmdi_static,EM_LINESCROLL,0,line-line2);
				//determine if the text is scrolled up beyond the bottom
				if(line-line2>0){
					line=SendMessage(hmdi_static,EM_GETLINECOUNT,0,0);
					pos=SendMessage(hmdi_static,EM_LINEINDEX,line-1,0);
					point.y=0;
					SendMessage(hmdi_static,EM_POSFROMCHAR,&point,pos);
					if(point.y<rect.bottom-fontheight)
						SendMessage(hmdi_static,EM_SCROLL,SB_LINEDOWN,0);
				}
				point.y=-1;
				SendMessage(hmdi_static,EM_POSFROMCHAR,&point,fpos+strlen(search_text));
				if(point.y>=0 && point.x>=0 &&
					point.y<=rect.bottom && point.x<=rect.right){
					//printf("x=%i y=%i h=%i\n",point.x,point.y,rect.bottom-rect.top);
					ClientToScreen(hmdi_static,&point);
					SetWindowPos(hwnd,NULL,point.x,point.y+fontheight,0,0,SWP_NOZORDER|SWP_NOSIZE);
				}
				//printf("fpos=%i %i %i dif=%i\n",fpos,line,line2,line-line2);
				last_search_pos=fpos;
				if(fpos==0)
					at_top=TRUE;
				else
					at_top=FALSE;
				{
					IRC_WINDOW *win=find_window_by_hwnd(GetParent(hmdi_static));
					if(win){
						win->scroll_free=TRUE;
					}
				}
			}
			else{
				POINT point={0,0};
				if(dir&FR_DOWN){
					RECT rect;
					GetWindowRect(hmdi_static,&rect);
					point.x=rect.left;
					point.y=rect.bottom;
				}
				else{
					ClientToScreen(hmdi_static,&point);
				}
				if(timer!=0){
					KillTimer(hwnd,timer);
					timer=0;
				}
				hide_tooltip();
				show_tooltip("nothing more found",point.x,point.y);
				timer=SetTimer(hwnd,0x1337,550,NULL);
			}
			last_dir=dir;
			break;
		case WM_DESTROY:
			goto quit;
			break;
		}
		break;
	case WM_TIMER:
		if(timer!=0){
			KillTimer(hwnd,timer);
			timer=0;
		}
		hide_tooltip();
		break;
	case WM_CLOSE:
	case WM_QUIT:
quit:
		if(timer!=0){
			KillTimer(hwnd,timer);
			timer=0;
		}
		hide_tooltip();
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}

LRESULT CALLBACK MDIChildWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static int list_drag=FALSE,list_width=60;
	IRC_WINDOW *win=0;
	if(control_debug(IDC_MDI_CLIENT,0,0))
	if(/*msg!=WM_NCMOUSEMOVE&&*/msg!=WM_MOUSEFIRST&&msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE/*&&msg!=WM_NOTIFY*/)
		//if(msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE)
	{
		static DWORD tick=0;
		if((GetTickCount()-tick)>500)
			printf("--\n");
		printf("m");
		print_msg(msg,lparam,wparam,hwnd);
		tick=GetTickCount();
	}
    switch(msg)
    {
	case WM_CREATE:
		{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lparam;
		LPMDICREATESTRUCT pmdics = (LPMDICREATESTRUCT)(pcs->lpCreateParams);
		create_mdi_window(hwnd,ghinstance,pmdics->lParam);
		SendDlgItemMessage(hwnd,MDI_EDIT,EM_LIMITTEXT,1024*64,0);
		orig_static_proc=(WNDPROC)SetWindowLong(GetDlgItem(hwnd,MDI_STATIC),GWL_WNDPROC,static_proc);
		SendDlgItemMessage(hwnd,MDI_STATIC,WM_SETFONT,(WPARAM)GetStockObject(SYSTEM_FIXED_FONT),0);
		SendDlgItemMessage(hwnd,MDI_LIST,WM_SETFONT,(WPARAM)GetStockObject(SYSTEM_FIXED_FONT),0);
		SendDlgItemMessage(hwnd,MDI_EDIT,WM_SETFONT,(WPARAM)GetStockObject(SYSTEM_FIXED_FONT),0);
		}
        break;
	case WM_MOUSEACTIVATE:
		if(LOWORD(lparam)==HTCLIENT){
			//SetFocus(GetDlgItem(hwnd,MDI_EDIT));
			//return MA_NOACTIVATE;
		}
		break;
	case WM_VKEYTOITEM:
		if(LOWORD(wparam)==VK_TAB){
			SetFocus(GetDlgItem(hwnd,MDI_EDIT));
			return -2;
		}
		break;
	case WM_SETFOCUS:
	case WM_IME_SETCONTEXT:
		if(wparam){
			SetFocus(GetDlgItem(hwnd,MDI_EDIT));
			return 0;
		}
		break;
	case WM_MDIACTIVATE:
        {
			SetFocus(GetDlgItem(lparam,MDI_EDIT));
			win=find_window_by_hwnd(lparam);
			if(win!=0){
				win->activity=FALSE;
				handle_switch_button(win->hbutton,FALSE);
			}
        }
        break;
	case WM_CONTEXTMENU:
		{
		int x=LOWORD(lparam);
		int y=HIWORD(lparam);
		int id=GetDlgCtrlID(wparam);
		if(id==MDI_LIST)
			if(lparam==-1){
				POINT p={0};
				GetCursorPos(&p);
				x=p.x;y=p.y;
			}
			TrackPopupMenu(list_menu,TPM_LEFTALIGN,x,y,0,hwnd,NULL);
		}
		break;
	case WM_SYSCOMMAND:
		switch(wparam&0xFFF0){
		case SC_MAXIMIZE:
			write_ini_str("SETTINGS","MDI_MAXIMIZED","1");
			break;
		case SC_RESTORE:
			write_ini_str("SETTINGS","MDI_MAXIMIZED","0");
			break;
		}
		break;
	case WM_CTLCOLORSTATIC:
		/*{
			RECT rect;
			HDC  hdc=(HDC)wparam;
			HWND ctrl=(HWND)lparam;
			COLORREF color=GetSysColor(COLOR_WINDOWTEXT);
			HBRUSH hbrush=0;
			hbrush=GetSysColorBrush(COLOR_BACKGROUND);
			if(last_static_msg!=WM_PAINT && last_static_msg!=WM_ERASEBKGND){
			//	GetClientRect(ctrl,&rect);
			//	FillRect(hdc,&rect,hbrush);
				InvalidateRect(ctrl,NULL,TRUE);
			}
			SetBkMode(hdc,TRANSPARENT);
			SetTextColor(hdc,color);
			return (LRESULT)hbrush;
		}*/
		break;
	case WM_DESTROY:
		break;
	case WM_ENDSESSION:
		win=find_window_by_hwnd(hwnd);
		if(win!=0 && win->type==SERVER_WINDOW)
			disconnect_server(win);
		break;
	case WM_CLOSE:
		win=find_window_by_hwnd(hwnd);
		if(win!=0){
			switch(win->type){
			case SERVER_WINDOW:
				if(win->session!=0 && irc_is_connected(win->session)){
					char str[128];
					_snprintf(str,sizeof(str),"Are you sure you want to disconnect from server\r\n[%s] %s ?",win->network,win->server);
					if(show_messagebox(hwnd,str,"Warning",MB_OKCANCEL)==IDOK)
						disconnect_server(win);
					else
						return 0;
				}
				else{
					disconnect_server(win);
				}
				break;
			default:
			case CHANNEL_WINDOW:
				close_log(win->channel,win->network);
				part_channel(win);
				break;
			case DCC_WINDOW:
				dcc_close_window(win);
				break;
			}
		}
		break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
		ReleaseCapture();
		write_ini_value("SETTINGS","LIST_WIDTH",list_width);
		list_drag=FALSE;
		break;
	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		SetCursor(LoadCursor(NULL,IDC_SIZEWE));
		list_drag=TRUE;
		break;
	case WM_MOUSEFIRST:
		{
			int x=LOWORD(lparam);
			SetCursor(LoadCursor(NULL,IDC_SIZEWE));
			if(list_drag){
				RECT rect;
				GetClientRect(hwnd,&rect);
				if((rect.right-x)>5 && x>5){
					list_width=rect.right-x;
					set_list_width(list_width);
					resize_mdi_window(hwnd);
				}
			}
		}
		break;
    case WM_COMMAND:
		//HIWORD(wParam) notification code
		//LOWORD(wParam) item control
		//lParam handle of control
		switch(LOWORD(wparam)){
		case CMD_BAN:
		case CMD_KICK:
		case CMD_KICKBAN:
		case CMD_DEVOICE:
		case CMD_VOICE:
		case CMD_DEOP:
		case CMD_OP:
		case CMD_CHANGE_NICK:
		case CMD_PRIV_MSG:
		case CMD_REFRESH_LIST:
		case CMD_SLAP:
		case CMD_WHOIS:
			do_cmd_on_list(hwnd,LOWORD(wparam));
			break;
		case MDI_LIST:
			if(HIWORD(wparam)==LBN_DBLCLK){
				int sel=0;
				if(SendMessage(lparam,LB_GETSELITEMS,1,&sel)==1){
					char nick[20]={0},*n;
					if(SendMessage(lparam,LB_GETTEXT,sel,nick)>0){
						n=nick;
						if(strlen(nick)>1)
							if(nick[0]=='@' || nick[0]=='+')
								n=nick+1;
						initiate_privmsg(GetParent(lparam),n);
					}
				}
			}
			break;
		case MDI_EDIT:
			switch(HIWORD(wparam)){
			case EN_CHANGE:
				{
				char str[MAX_EDIT_LENGTH]={0};
				str[MAX_EDIT_LENGTH-1]=0;
				GetWindowText(lparam,str,sizeof(str)-1);
				strncpy(edit_buffer[0],str,sizeof(edit_buffer[0]));
				edit_buffer[0][sizeof(edit_buffer[0])-1]=0;
				scroll_history_pos=0;
				tab_continue=FALSE;
				}
				break;
			case EN_VSCROLL:
				{
					HWND hedit=lparam;
					char str[MAX_EDIT_LENGTH]={0};
					int i,len,lines=0;
					GetWindowText(hedit,str,sizeof(str));
					len=strlen(str);
					for(i=0;i<len;i++){
						if(str[i]=='\n')
							lines++;
					}
					if(lines>=2 || len>=(MAX_EDIT_LENGTH-1)){
						PostMessage(hwnd,WM_APP+1,0,hedit);
						break;
					}
					post_message(hwnd,str);
					add_history(str);
					SetWindowText(hedit,"");

				}
				break;
			}
			break;
		case MDI_SCROLL_LOCK:
			win=find_window_by_hwnd(hwnd);
			if(win!=0)
				SetFocus(win->hedit);
			break;
		}
		break;
	case WM_HELP:
		win=find_window_by_hwnd(hwnd);
		show_help(win);
		break;
	case WM_APP://custom edit input wparam=key
		{
			int scroll=-1;
			switch(wparam){
				case 0x17: //ctrl-w
					delete_word(GetDlgItem(hwnd,MDI_EDIT));
					break;
				case 0x12: //ctrl-r
					{
					char str[2]={22,0};
					SendMessage(GetDlgItem(hwnd,MDI_EDIT),EM_REPLACESEL,TRUE,str);
					}
					break;
				case 0xB: //ctrl-k
					{
					char str[2]={3,0};
					SendMessage(GetDlgItem(hwnd,MDI_EDIT),EM_REPLACESEL,TRUE,str);
					}
					break;
				case 0x6: //ctr-f
					hmdi_static=GetDlgItem(hwnd,MDI_STATIC);
					DialogBox(ghinstance,MAKEINTRESOURCE(IDD_TEXTSEARCH),hwnd,text_search);
					break;
				case VK_F2:
					{
						IRC_WINDOW *win=find_window_by_hwnd(hwnd);
						if(win!=0 && win->hstatic!=0)
							show_art_viewer(hwnd,win->hstatic);
					}
					break;
				case VK_TAB:
					if(tab_continue)
						tab_next(hwnd,tab_word,tab_pos);
					else
						tab_completion(hwnd);
					break;
				case VK_HOME:
					scroll=SB_TOP;
					break;
				case VK_END:
					scroll=SB_BOTTOM;
					break;
				case VK_ESCAPE:
					if(edit_buffer[0][0]==0)
						scroll_history_pos=0;
					else
						scroll_history_pos=-1;
					SetWindowText(GetDlgItem(hwnd,MDI_EDIT),"");
					break;
				case VK_PRIOR:
					if(GetKeyState(VK_CONTROL)&0x8000){
						if(GetKeyState(VK_SHIFT)&0x8000)
							scroll=SB_TOP;
						else
							scroll=SB_LINEUP;
						break;
					}
					else{
						scroll=SB_PAGEUP;
						break;
					}
				case VK_NEXT:
					if(GetKeyState(VK_CONTROL)&0x8000){
						if(GetKeyState(VK_SHIFT)&0x8000)
							scroll=SB_BOTTOM;
						else
							scroll=SB_LINEDOWN;
						break;
					}
					else{
						scroll=SB_PAGEDOWN;
						break;
					}
				case VK_UP:
					restore_buffer(GetDlgItem(hwnd,MDI_EDIT),1);
					break;
				case VK_DOWN:
					restore_buffer(GetDlgItem(hwnd,MDI_EDIT),-1);
					break;
				case VK_RETURN:
					SendDlgItemMessage(hwnd,MDI_EDIT,EM_SETSEL,0,-1);
					SendDlgItemMessage(hwnd,MDI_EDIT,EM_SETSEL,-1,-1);
					break;
			}
			if(scroll!=-1)
				SendDlgItemMessage(hwnd,MDI_STATIC,EM_SCROLL,scroll,0);
			set_scroll_lock(hwnd,scroll);
		}
		break;
	case WM_APP+1: //send multiple lines
		{
			char *str;
			int str_len=1024*64;
			HWND hedit=lparam;
			if(hedit==0)
				break;
			str=malloc(str_len);
			if(str){
				GetWindowText(hedit,str,str_len);
				post_long_message(hwnd,str);
				free(str);
			}
			SetWindowText(hedit,"");
		}
		break;
	case WM_SIZE:
		resize_mdi_window(hwnd);
		break;

    }
	return DefMDIChildProc(hwnd, msg, wparam, lparam);
}

BOOL setup_mdi_classes(HINSTANCE hinstance)
{
	int result=TRUE;
    WNDCLASS wc;
	memset(&wc,0,sizeof(wc));
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = MDIChildWndProc;
    wc.hInstance     = hinstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
    wc.lpszClassName = "channelwindow";

    if(!RegisterClass(&wc))
		result=FALSE;
    wc.lpszClassName = "privmsgwindow";
    if(!RegisterClass(&wc))
		result=FALSE;
    wc.lpszClassName = "serverwindow";
    if(!RegisterClass(&wc))
		result=FALSE;
	return result;
}
int create_window_type(HWND hmdiclient,IRC_WINDOW *win,int type,char *nick)
{
	int maximized=0,style,handle;
	char title[256]={0};
	MDICREATESTRUCT cs;
	get_ini_value("SETTINGS","MDI_MAXIMIZED",&maximized);
	if(maximized!=0)
		style = WS_MAXIMIZE|MDIS_ALLCHILDSTYLES;
	else
		style = MDIS_ALLCHILDSTYLES;
	cs.cx=cs.cy=cs.x=cs.y=CW_USEDEFAULT;
	if(type==CHANNEL_WINDOW){
		cs.szClass="channelwindow";
		cs.szTitle=win->channel;
	}else if(type==SERVER_WINDOW){
		cs.szClass="serverwindow";
		_snprintf(title,sizeof(title),"%s %s",win->network,win->server);
		cs.szTitle=title;
	}else if(type==PRIVMSG_WINDOW){
		cs.szClass="privmsgwindow";
		_snprintf(title,sizeof(title),"%s on %s %s",nick,win->network,win->server);
		cs.szTitle=title;
	}else if(type==DCC_WINDOW){
		cs.szClass="privmsgwindow";
		_snprintf(title,sizeof(title),"DCC from %s on %s %s",nick,win->network,win->server);
		cs.szTitle=title;
	}
	cs.style=style;
	cs.hOwner=ghinstance;
	cs.lParam=win;
	handle=SendMessage(hmdiclient,WM_MDICREATE,0,&cs);
	return handle;
}
int acquire_network_window(char *network,char *server,int port,int ssl)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].type==SERVER_WINDOW)
			if(stricmp(irc_windows[i].network,network)==0)
				return &irc_windows[i];
	}
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].network[0]==0){
			strncpy(irc_windows[i].network,network,sizeof(irc_windows[i].network));
			strncpy(irc_windows[i].server,server,sizeof(irc_windows[i].server));
			irc_windows[i].port=port;
			irc_windows[i].ssl=ssl;
			irc_windows[i].type=SERVER_WINDOW;
			return &irc_windows[i];
		}
	}
	return 0;
}
int acquire_channel_window(const char *network,const char *channel,int type)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].type==type)
			if(stricmp(irc_windows[i].channel,channel)==0 &&
				stricmp(irc_windows[i].network,network)==0)
				return &irc_windows[i];
	}
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].network[0]==0){
			strncpy(irc_windows[i].network,network,sizeof(irc_windows[i].network));
			strncpy(irc_windows[i].channel,channel,sizeof(irc_windows[i].channel));
			irc_windows[i].type=type;
			return &irc_windows[i];
		}
	}
	return 0;
}
int erase_all_sessions(void *s)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].session==s){
			irc_windows[i].session=0;
			PostMessage(irc_windows[i].hlist,LB_RESETCONTENT,0,0);
		}
	}
	return TRUE;
}
int erase_irc_window(HWND hwnd)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].hwnd==hwnd){
			memset(&irc_windows[i],0,sizeof(IRC_WINDOW));
			return TRUE;
		}
	}
	return FALSE;
}


int find_window_by_hwnd(HWND hwnd)
{
	int i;
	static IRC_WINDOW *last_win=0;
	if(hwnd==0)
		return 0;
	if(last_win!=0 && last_win->hwnd==hwnd)
		return last_win;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].hwnd==hwnd){
			last_win=&irc_windows[i];
			return &irc_windows[i];
		}
	}
	return 0;
}

int get_hwnd_by_session(void *session,char *channel)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].session==session){
			if(stricmp(irc_windows[i].channel,channel)==0)
				return irc_windows[i].hwnd;
		}
	}
	return 0;
}

int restore_buffer(HWND hedit,int dir)
{
	int len;
	if(edit_buffer[0][0]!=0 && edit_buffer[1][0]!=0 && strcmp(edit_buffer[0],edit_buffer[1])==0){
		if(scroll_history_pos==0 && dir==1)
			scroll_history_pos=2;
		else if(scroll_history_pos==1 && dir==-1)
			scroll_history_pos=-1;
		else
			scroll_history_pos+=dir;
	}
	else
		scroll_history_pos+=dir;
	if(scroll_history_pos<0){
		if(edit_buffer[0][0]==0)
			scroll_history_pos=0;
		else
			scroll_history_pos=-1;
		SetWindowText(hedit,"");
		return TRUE;
	}
	else if(scroll_history_pos>=MAX_EDIT_HISTORY)
		scroll_history_pos=MAX_EDIT_HISTORY-1;
	len=strlen(edit_buffer[scroll_history_pos]);
	if(len!=0){
		SetWindowText(hedit,edit_buffer[scroll_history_pos]);
		SendMessage(hedit,EM_SETSEL,0,-1);
		SendMessage(hedit,EM_SETSEL,-1,-1);
	}
	else{
		if(scroll_history_pos!=0)
			scroll_history_pos-=dir;
		else
			SetWindowText(hedit,"");
		scroll_history_pos%=MAX_EDIT_HISTORY;
	}
	return TRUE;
}
int resort_history(char *str,int index)
{
	int i;
	for(i=index-1;i>0;i--){
		strncpy(edit_buffer[i+1],edit_buffer[i],sizeof(edit_buffer[i]));
	}
	strncpy(edit_buffer[1],str,sizeof(edit_buffer[i]));
	edit_buffer[1][sizeof(edit_buffer[1])-1]=0;
	return TRUE;
}
int add_history(char *str)
{
	int i;
	scroll_history_pos=0;
	for(i=1;i<MAX_EDIT_HISTORY;i++){
		if(strncmp(edit_buffer[i],str,sizeof(edit_buffer[i]))==0){
			resort_history(str,i);
			return TRUE;
		}
	}
	for(i=MAX_EDIT_HISTORY-2;i>=0;i--){
		strncpy(edit_buffer[i+1],edit_buffer[i],sizeof(edit_buffer[i]));
	}
	strncpy(edit_buffer[1],str,sizeof(edit_buffer[1]));
	edit_buffer[1][sizeof(edit_buffer[1])-1]=0;
	memset(edit_buffer[0],0,sizeof(edit_buffer[0]));
	return TRUE;
}

int add_line_mdi_nolog(IRC_WINDOW *win,char *str)
{
	int len,selecting;
	CHARRANGE chr={0};
	selecting=win->selecting;
	if(selecting){ //save current selection
		SendMessage(win->hstatic,EM_EXGETSEL,0,&chr);
		SendMessage(win->hstatic,WM_LBUTTONUP,0,0);
	}
	len=GetWindowTextLength(win->hstatic);
	SendMessage(win->hstatic,EM_SETSEL,len,len);
	if(len>0)
		SendMessage(win->hstatic,EM_REPLACESEL,FALSE,"\r\n");
	SendMessage(win->hstatic,EM_REPLACESEL,FALSE,str);
	if(selecting){
		SendMessage(win->hstatic,WM_LBUTTONDOWN,0,0);
		SendMessage(win->hstatic,EM_EXSETSEL,0,&chr);
	}
	if(!win->scroll_free)
		SendMessage(win->hstatic,WM_VSCROLL,SB_BOTTOM,0);
	return TRUE;
}
int add_line_mdi(IRC_WINDOW *win,char *str)
{
	add_line_mdi_nolog(win,str);
	if(win->type==CHANNEL_WINDOW || win->type==PRIVMSG_WINDOW)
		log_str(win->channel,win->network,str);
	return TRUE;
}

int extract_list_nick(char *list,char *nick,int size)
{
	int i,len,index=0;
	len=strlen(list);
	for(i=0;i<len;i++){
		if(list[i]=='@' || list[i]=='+')
			continue;
		nick[index++]=list[i];
		if(index>=(size-1))
			break;
	}
	nick[index++]=0;
	return TRUE;
}
int extract_nick(const char *origin,char *nick,int size)
{
	int i,len,index=0;
	len=strlen(origin);
	for(i=0;i<len;i++){
		if(origin[i]=='!')
			break;
		nick[index++]=origin[i];
		if(index>=(size-1))
			break;
	}
	nick[index++]=0;
	return TRUE;
}
int update_nick_in_list(HWND hlist,const char *oldnick,const char *newnick)
{
	char tmp[25]={0};
	char pre[3]={'@','+',0};
	int i,index;
	if(hlist==0)
		return LB_ERR;
	for(i=0;i<sizeof(pre);i++){
		if(pre[i]!=0)
			_snprintf(tmp,sizeof(tmp),"%c%s",pre[i],oldnick);
		else
			strncpy(tmp,oldnick,sizeof(tmp));
		index=SendMessage(hlist,LB_FINDSTRINGEXACT,-1,tmp);
		if(index!=LB_ERR){
			SendMessage(hlist,LB_DELETESTRING,index,0);
			if(newnick!=0 && newnick[0]!=0){
				if(pre[i]!=0)
					_snprintf(tmp,sizeof(tmp),"%c%s",pre[i],newnick);
				else
					strncpy(tmp,newnick,sizeof(tmp));
				index=SendMessage(hlist,LB_ADDSTRING,0,tmp);
			}
			break;
		}
	}
	return index;
}

int update_nick(void *session,const char *oldnick,const char *nick)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].session==session){
			update_nick_in_list(irc_windows[i].hlist,oldnick,nick);
			if(strcmp(irc_windows[i].nick,oldnick)==0){
				strncpy(irc_windows[i].nick,nick,sizeof(irc_windows[i].nick));
			}
		}
	}
	return TRUE;
}
int update_user_nick(void *session,char *nick)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].session==session){
			update_nick_in_list(irc_windows[i].hlist,irc_windows[i].nick,nick);
			strncpy(irc_windows[i].nick,nick,sizeof(irc_windows[i].nick));
		}
	}
	return TRUE;
}
int get_substr(unsigned char *str,int start,char *substr,int size,int *pos)
{
	int i,index,len,found=FALSE;
	len=strlen(str);
	if(start<=len){
		if(start>0 && str[start-1]<=' ' && str[start]<=' ')
			return found;
		if(start==0 && str[0]<=' ')
			return found;

		while(start>0){
			start--;
			if(str[start]=='<' || isspace(str[start])){
				start++;
				break;
			}
		}
		*pos=start;
		len=strlen(str+start);
		index=0;
		for(i=0;i<len;i++){
			if(str[start+i]=='>' || isspace(str[start+i])){
				break;
			}
			else
				substr[index++]=str[start+i];
			if(index>=(size-1))
				break;
		}
		substr[index++]=0;
		if(substr[0]!=0)
			found=TRUE;
	}
	return found;
}
int replace_word(char *str,int max,char *word,char pos)
{
	char substr[40]={0};
	int found=0;
	if(get_substr(str,pos,substr,sizeof(substr),&found)){
		int sublen;
		char temp[MAX_EDIT_LENGTH]={0};
		sublen=strlen(substr);
		str[max-1]=0;
		str[found]=0;
		temp[sizeof(temp)-1]=0;
		strncpy(temp,str+found+sublen,sizeof(temp)-1);
		_snprintf(str,max-1,"%s%s%s",str,word,temp);
		return TRUE;
	}
	return FALSE;
}
int find_next_nick(HWND hlist,char *substr,int last)
{
	int i,min,index[3];
	char n[20]={0};
	_snprintf(n,sizeof(n),"@%s",substr);
	index[0]=SendMessage(hlist,LB_FINDSTRING,last,n);
	_snprintf(n,sizeof(n),"+%s",substr);
	index[1]=SendMessage(hlist,LB_FINDSTRING,last,n);
	_snprintf(n,sizeof(n),"%s",substr);
	index[2]=SendMessage(hlist,LB_FINDSTRING,last,n);
	min=-1;
	for(i=0;i<sizeof(index)/sizeof(int);i++){
		int v=index[i];
		if(v>=0){
			if(v>last){
				if(min>=0){
					if(v<min)
						min=v;
				}
				else
					min=v;
			}

		}
	}
	if(min>=0)
		return min;
	min=-1;
	for(i=0;i<sizeof(index)/sizeof(int);i++){
		int v=index[i];
		if(v>=0){
			if(min>=0){
				if(v<min)
					min=v;
			}
			else
				min=v;
		}
	}
	if(min>=0)
		return min;
	return LB_ERR;
}
int tab_next(HWND hwnd,char *substr,int pos)
{
	IRC_WINDOW *win;
	int index;
	static int last=-1;
	win=find_window_by_hwnd(hwnd);
	if(win!=0){
		char str[MAX_EDIT_LENGTH]={0};
		str[sizeof(str)-1]=0;
		if(GetWindowText(win->hedit,str,sizeof(str)-1)>0){
			int attempts=0;
			do{
				index=find_next_nick(win->hlist,substr,last);
				attempts++;
				if(last==-1)
					break;
				if(index==LB_ERR)
					last=-1;
				else
					break;
			}while(attempts<2);
			if(index!=LB_ERR){
				char nick[20]={0};
				if(SendMessage(win->hlist,LB_GETTEXT,index,nick)>0){
					int len;
					last=index;
					extract_list_nick(nick,nick,sizeof(nick));
					len=strlen(nick);
					printf("oldstr=%s\n",str);
					replace_word(str,sizeof(str),nick,tab_pos);
					printf("newstr=%s\n",str);
					SetWindowText(win->hedit,str);
					SendMessage(win->hedit,EM_SETSEL,pos+len,pos+len);
				}
			}
			else
				last=-1;
		}
	}
	return tab_continue;
}
int tab_completion(HWND hwnd)
{
	IRC_WINDOW *win;
	win=find_window_by_hwnd(hwnd);
	if(win!=0){
		char str[MAX_EDIT_LENGTH]={0};
		str[sizeof(str)-1]=0;
		if(GetWindowText(win->hedit,str,sizeof(str)-1)>0){
			int start=0,end=0;
			SendMessage(win->hedit,EM_GETSEL,&start,&end);
			if(end<start)
				start=end;

			tab_word[0]=0;
			if(get_substr(str,start,tab_word,sizeof(tab_word),&tab_pos)){
				tab_continue=TRUE;
				printf("substr=%s\n",tab_word);
				tab_next(hwnd,tab_word,tab_pos);
			}
		}
	}
	return tab_continue;
}
int delete_word(HWND hedit)
{
	int result=FALSE;
	char *buf;
	int size=4096;
	if(0==hedit)
		return result;
	buf=calloc(size,1);
	if(buf!=0){
		int cursor=0;
		SendMessage(hedit,EM_GETSEL,&cursor,NULL);
		GetWindowText(hedit,buf,size);
		if(cursor<size){
			int i,start,end,type;
			i=cursor-1;
			if(i<0)
				i=0;
			type=!isspace(buf[i]);
			start=end=cursor;
			for(;i>=0;i--){
				if(type!=(!isspace(buf[i]))){
					end=i+1;
					break;
				}else if(0==i){
					end=i;
					break;
				}
			}
			if(start!=end){
				char *replace="";
				SendMessage(hedit,EM_SETSEL,start,end);
				SendMessage(hedit,EM_REPLACESEL,TRUE,replace);
			}
		}
		free(buf);
	}
	return result;
}
int do_cmd_on_list(HWND hwnd,int cmd)
{
	IRC_WINDOW *win;
	win=find_window_by_hwnd(hwnd);
	if(win!=0){
		int count,items[20];
		char nick[40]={0};
		if(win->session==0 || (!irc_is_connected(win->session)))
			return FALSE;
		switch(cmd){
		case CMD_REFRESH_LIST:
			SendMessage(win->hlist,LB_RESETCONTENT,0,0);
			irc_cmd_names(win->session,win->channel);
			break;
		case CMD_CHANGE_NICK:
			if(show_user_input(hwnd,"enter new nick",nick,sizeof(nick))==IDOK){
				irc_cmd_nick(win->session,nick);
				return TRUE;
			}
			break;
		default:
			count=SendMessage(win->hlist,LB_GETSELITEMS,sizeof(items)/sizeof(int),&items);
			if(count>0){
				int i;
				for(i=0;i<count;i++){
					if(SendMessage(win->hlist,LB_GETTEXT,items[i],&nick)>0){
						char str[80];
						extract_list_nick(nick,nick,sizeof(nick));
						switch(cmd){
						case CMD_PRIV_MSG:
							initiate_privmsg(win->hwnd,nick);
							break;
						case CMD_KICK:
							irc_send_raw(win->session,"KICK %s %s kick-it",win->channel,nick);
							SendMessage(win->hlist,LB_RESETCONTENT,0,0);
							irc_cmd_names(win->session,win->channel);
							break;
						case CMD_KICKBAN:
							_snprintf(str,sizeof(str),"+b %s",nick);
							irc_cmd_channel_mode(win->session,win->channel,str);
							irc_send_raw(win->session,"KICK %s %s kick-it",win->channel,nick);
							SendMessage(win->hlist,LB_RESETCONTENT,0,0);
							irc_cmd_names(win->session,win->channel);
							break;
						case CMD_BAN:
							_snprintf(str,sizeof(str),"+b %s",nick);
							irc_cmd_channel_mode(win->session,win->channel,str);
							break;
						case CMD_DEVOICE:
							_snprintf(str,sizeof(str),"-v %s",nick);
							irc_cmd_channel_mode(win->session,win->channel,str);
							break;
						case CMD_VOICE:
							_snprintf(str,sizeof(str),"+v %s",nick);
							irc_cmd_channel_mode(win->session,win->channel,str);
							break;
						case CMD_DEOP:
							_snprintf(str,sizeof(str),"-o %s",nick);
							irc_cmd_channel_mode(win->session,win->channel,str);
							break;
						case CMD_OP:
							_snprintf(str,sizeof(str),"+o %s",nick);
							irc_cmd_channel_mode(win->session,win->channel,str);
							break;
						case CMD_WHOIS:
							irc_cmd_whois(win->session,nick);
							break;
						case CMD_SLAP:
							irc_slap(win->session,win->channel,nick);
							break;
						}
					}
				}
				return TRUE;
			}
			break;
		}
	}
	return FALSE;
}
int custom_dispatch(MSG *msg)
{
	int i;
	IRC_WINDOW *win=0;
	HWND hwnd=0;
	int type=0;
	static int mbutton_down=FALSE;

	if(msg->message==WM_MOUSEWHEEL)
		hwnd=WindowFromPoint(msg->pt);

	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].hedit==msg->hwnd){
			win=&irc_windows[i];
			type=MDI_EDIT;
		}
		else if(irc_windows[i].hstatic==msg->hwnd){
			win=&irc_windows[i];
			type=MDI_STATIC;
		}
		else if(irc_windows[i].hlist==msg->hwnd){
			win=&irc_windows[i];
			type=MDI_LIST;
		}
		else if(irc_windows[i].hbutton==msg->hwnd){
			win=&irc_windows[i];
			type=IDC_SWITCHBAR;
		}
		if(msg->message==WM_MOUSEWHEEL){
			if(hwnd!=0){
				if(irc_windows[i].hstatic==hwnd || irc_windows[i].hlist==hwnd){
					msg->hwnd=hwnd;
					DispatchMessage(msg);
					return TRUE;
				}
			}
		}
	}
	if(win!=0){
		switch(msg->message){
		case WM_MOUSEWHEEL:
			if(type==MDI_EDIT){
				msg->hwnd=win->hstatic;
				DispatchMessage(msg);
				return TRUE;
			}
			break;
		case WM_CHAR:
			if(type==MDI_EDIT){
				switch(msg->wParam){
				case 0x17: //ctrl-w
				case 0x12: //ctrl-R
				case 0xB: //ctrl-K
				case 0x6: //ctr-f
					msg->message=WM_APP;
					msg->hwnd=GetParent(msg->hwnd);
					DispatchMessage(msg);
					return TRUE;
				}
			}
			break;
		case WM_KEYFIRST:
			if(type==MDI_STATIC){
				SetFocus(win->hedit);
				msg->hwnd=win->hedit;
				DispatchMessage(msg);
				return TRUE;
			}
			else if(type==MDI_EDIT){
				switch(msg->wParam){
				case VK_RETURN:
					{
						MSG mymsg=*msg;
						mymsg.hwnd=GetParent(msg->hwnd);
						mymsg.message=WM_APP;
						DispatchMessage(&mymsg);
					}
					break;
				case VK_TAB:
					if(GetKeyState(VK_CONTROL)&0x8000)
						return FALSE;
				case VK_HOME:
				case VK_END:
					if(msg->wParam!=VK_TAB)
						if(!(GetKeyState(VK_CONTROL)&0x8000))
							return FALSE;
				case VK_F2:
				case VK_F3:
				case VK_ESCAPE:
				case VK_PRIOR:
				case VK_NEXT:
				case VK_UP:
				case VK_DOWN:
					msg->message=WM_APP;
					msg->hwnd=GetParent(msg->hwnd);
					DispatchMessage(msg);
					return TRUE;
				}
			}
			break;
		case WM_MOUSEFIRST:
			if(type==MDI_STATIC){
				DispatchMessage(msg);
				return TRUE;
			}
			break;
		case WM_MBUTTONDOWN:
			break;
		case WM_LBUTTONDBLCLK:
			if(type==IDC_SWITCHBAR){
				msg->message=WM_LBUTTONDOWN;
				DispatchMessage(msg);
				return TRUE;
			}
			break;
		case WM_LBUTTONDOWN:
			if(type==MDI_STATIC){
				CHARRANGE cr={0};
				int index,line;
				win->selecting=TRUE;
				//this keeps the window current scroll position when first clicking on the edit window when its out of focus
				//other wise the edit will bring the caret position into view which could be anywhere
				line=SendMessage(msg->hwnd,EM_GETFIRSTVISIBLELINE,0,0);
				index=SendMessage(msg->hwnd,EM_LINEINDEX,line+1,0);
				cr.cpMin=cr.cpMax=index;
				SendMessage(msg->hwnd,EM_EXSETSEL,0,&cr);
				//this allows you to begin block selection immediately in rich edit control when its not in focus
				mbutton_down=TRUE;
				DispatchMessage(msg);
				msg->message=WM_LBUTTONUP;
				DispatchMessage(msg);
				msg->message=WM_LBUTTONDOWN;
				DispatchMessage(msg);
				return TRUE;
			}
			break;
		case WM_LBUTTONUP:
			if(mbutton_down){
				win->selecting=FALSE;
				mbutton_down=FALSE;
				DispatchMessage(msg);
				msg->hwnd=win->hstatic;
				msg->message=WM_APP;
				msg->lParam=MAKELPARAM(msg->pt.x,msg->pt.y);
				DispatchMessage(msg);
				set_scroll_lock(win->hwnd,-1);
				return TRUE;
			}
			break;
		}
	}

	return FALSE;
}

int create_popup_menus()
{
	if(list_menu!=0)DestroyMenu(list_menu);
	if(list_menu=CreatePopupMenu()){
		InsertMenu(list_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_SLAP,"slap");
		InsertMenu(list_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_PRIV_MSG,"priv msg");
		InsertMenu(list_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_WHOIS,"whois");
		InsertMenu(list_menu,0xFFFFFFFF,MF_BYPOSITION|MF_SEPARATOR,0,0);
		InsertMenu(list_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_OP,"op");
		InsertMenu(list_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_DEOP,"de-op");
		InsertMenu(list_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_VOICE,"voice");
		InsertMenu(list_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_DEVOICE,"de-voice");
		InsertMenu(list_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_KICK,"kick");
		InsertMenu(list_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_BAN,"ban");
		InsertMenu(list_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_KICKBAN,"kick-ban");
		InsertMenu(list_menu,0xFFFFFFFF,MF_BYPOSITION|MF_SEPARATOR,0,0);
		InsertMenu(list_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_REFRESH_LIST,"refresh list");
		InsertMenu(list_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_CHANGE_NICK,"change your nick");
	}
	if(static_menu!=0)DestroyMenu(static_menu);
	if(static_menu=CreatePopupMenu()){
		InsertMenu(static_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_CHANMODES,"chan modes");
		InsertMenu(static_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_TSEARCH,"text search");
		InsertMenu(static_menu,0xFFFFFFFF,MF_BYPOSITION|MF_SEPARATOR,0,0);
		InsertMenu(static_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_CLEAR,"clear buffer");
		InsertMenu(static_menu,0xFFFFFFFF,MF_BYPOSITION|MF_SEPARATOR,0,0);
		InsertMenu(static_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_CLEARTYPED,"clear typed buffer");
	}
	if(url_menu!=0)DestroyMenu(url_menu);
	if(url_menu=CreatePopupMenu()){
		InsertMenu(url_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_COPYURL,"copy url");
		InsertMenu(url_menu,0xFFFFFFFF,MF_BYPOSITION|MF_SEPARATOR,0,0);
		InsertMenu(url_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_OPENURL,"open url");
	}
	return 0;
}

int init_mdi_stuff()
{
	extern int show_joins,lua_script_enable;
	int list_width=60;
	memset(irc_windows,0,sizeof(irc_windows));
	memset(server_threads,0,sizeof(server_threads));
	memset(edit_buffer,0,sizeof(edit_buffer));
	init_log_mutex();
	init_log_files();
	get_ini_value("SETTINGS","LIST_WIDTH",&list_width);
	get_ini_value("SETTINGS","SHOW_JOINS",&show_joins);
	get_ini_value("SETTINGS","ENABLE_LUA_SCRIPT",&lua_script_enable);
	set_list_width(list_width);
	init_server_scoll_lock_size();
	create_popup_menus();
	return TRUE;
}
#include "switchbar_stuff.h"
#include "server_window_stuff.h"
#include "dcc_window_stuff.h"
#include "channel_window_stuff.h"
#include "privmsg_window_stuff.h"
#include "lua_specific_funcs.h"
#include "ircstuff.h"
