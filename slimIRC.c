#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#include "resource.h"
#include "Commctrl.h"

HINSTANCE ghinstance=0;
HWND ghmainframe=0,ghmdiclient=0,ghswitchbar=0;
static HMENU ghmenu=0;
static int mousex=0,mousey=0;
static int lmb_down=FALSE;
static int main_drag=FALSE;
int switch_height=40;
HWND ghlistview=0;
CRITICAL_SECTION mutex;
int auto_connect(HWND hmdi);

int list_edit=FALSE;
BOOL CALLBACK add_server(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	extern short add_server_anchors[];
	static HWND grippy=0;
	static int entry_num=-1;
	static char old_server[80]={0};
	static int help_active=FALSE;
	switch(msg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd,IDC_NETWORK,EM_LIMITTEXT,80,0);
		SendDlgItemMessage(hwnd,IDC_SERVER,EM_LIMITTEXT,80,0);
		SendDlgItemMessage(hwnd,IDC_PORTS,EM_LIMITTEXT,80,0);
		SendDlgItemMessage(hwnd,IDC_PASSWORD,EM_LIMITTEXT,20,0);
		grippy=create_grippy(hwnd);
		if(list_edit){
			populate_add_server_win(ghlistview,hwnd);
			old_server[0]=0;
			GetWindowText(GetDlgItem(hwnd,IDC_SERVER),old_server,sizeof(old_server));
		}
		else
			SetDlgItemText(hwnd,IDC_NETWORK,"default");
		help_active=FALSE;
		break;
	case WM_SIZE:
		grippy_move(hwnd,grippy);
		reposition_controls(hwnd,add_server_anchors);
		break;
	case WM_HELP:
		if(!help_active){
			help_active=TRUE;
			MessageBox(hwnd,"use # in front of server name for SSL, ## to verify certs","Help",MB_OK);
			help_active=FALSE;
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		case WM_DESTROY:
			EndDialog(hwnd,0);
			break;
		case IDOK:
			if(GetFocus()==GetDlgItem(hwnd,IDOK)){
				save_server_entry(hwnd,list_edit,old_server);
				load_ini_servers(ghlistview);
				EndDialog(hwnd,0);
			}
			else
				PostMessage(hwnd,WM_NEXTDLGCTL,0,0);
			break;
		}
		break;
	case WM_CLOSE:
	case WM_QUIT:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}

BOOL CALLBACK server_dlg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	extern short server_list_anchors[];
	static int dir=FALSE,col=0;
	static DWORD tick;
	static HWND grippy=0,server_dlg_menu=0;
	if(FALSE)
	if(msg!=WM_MOUSEFIRST&&msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE/*&&msg!=WM_NOTIFY*/)
	//if(msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE)
	{
		if((GetTickCount()-tick)>500)
			printf("--\n");
		print_msg(msg,lparam,wparam,hwnd);
		tick=GetTickCount();
	}
	switch(msg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hwnd,IDC_JOIN,"Connect");
		grippy=create_grippy(hwnd);
		ghlistview=create_listview(hwnd,ghinstance);
		load_ini_server_listview(ghlistview);
		load_ini_servers(ghlistview);
		sort_listview(ghlistview,dir,col);
		if(server_dlg_menu==0)
			server_dlg_menu=create_server_dlg_menu(hwnd);
		break;
	case WM_SIZE:
		grippy_move(hwnd,grippy);
		reposition_controls(hwnd,server_list_anchors);
		break;
	case WM_NOTIFY:
		{
			NMHDR *nmh=lparam;
			NMITEMACTIVATE *nmitem=lparam;
			if(nmh->hwndFrom==ghlistview){
				switch(nmh->code){
				case NM_RCLICK:
					if(server_dlg_menu!=0 && get_selected_count(ghlistview)>0){
						POINT p;
						GetCursorPos(&p);
						TrackPopupMenu(server_dlg_menu,TPM_LEFTALIGN,p.x,p.y,0,hwnd,NULL);
					}
					break;
				case NM_DBLCLK:
					if(nmitem->iItem<0)
						break;
					if((GetKeyState(VK_CONTROL)&0x8000) || (GetKeyState(VK_SHIFT)&0x8000)){
						list_edit=TRUE;
						DialogBox(ghinstance,MAKEINTRESOURCE(IDD_SERVER),hwnd,add_server);
						sort_listview(ghlistview,dir,col);
					}
					else{
						int item;
						char server[80]={0},network[80]={0},port[20]={0},ssl[20]={0},password[20]={0};
connect_channel:
						item=get_focused_item(ghlistview);
						if(item>=0){
							ListView_GetItemText(ghlistview,item,0,network,sizeof(network));
							ListView_GetItemText(ghlistview,item,1,server,sizeof(server));
							ListView_GetItemText(ghlistview,item,2,port,sizeof(port));
							ListView_GetItemText(ghlistview,item,4,ssl,sizeof(ssl));
							ListView_GetItemText(ghlistview,item,5,password,sizeof(password));
							connect_server(ghmdiclient,network,server,atoi(port),ssl[0]!=0,password);
							EndDialog(hwnd,0);
						}
					}
					break;
				case LVN_KEYDOWN:
					{
						LV_KEYDOWN *key=lparam;
						switch(key->wVKey){
						case VK_SPACE:
							list_edit=TRUE;
							DialogBox(ghinstance,MAKEINTRESOURCE(IDD_SERVER),hwnd,add_server);
							sort_listview(ghlistview,dir,col);
							break;
						case VK_INSERT:
							list_edit=FALSE;
							DialogBox(ghinstance,MAKEINTRESOURCE(IDD_SERVER),hwnd,add_server);
							sort_listview(ghlistview,dir,col);
							break;
						case VK_DELETE:
							if(get_selected_count(ghlistview)>1){
								if(show_messagebox(hwnd,"delete selected items?","Delete",MB_OKCANCEL)!=IDOK)
									break;
							}
							do_server_deletion(ghlistview);
							sort_listview(ghlistview,dir,col);
							break;
						case 'A':
							if(GetKeyState(VK_CONTROL)&0x8000)
								list_select_all(ghlistview);
							break;
						}
					}
					break;
				case  LVN_COLUMNCLICK:
					{
						NMLISTVIEW *nmlv=lparam;
						col=nmlv->iSubItem;
						dir=!dir;
						sort_listview(ghlistview,dir,col);

					}
					break;

				}

			}
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam)){
		default:
			if(server_dlg_popup_cmd(LOWORD(wparam),ghlistview))
				sort_listview(ghlistview,dir,col);
			break;
		case IDC_ADD:
			list_edit=FALSE;
			DialogBox(ghinstance,MAKEINTRESOURCE(IDD_SERVER),hwnd,add_server);
			sort_listview(ghlistview,dir,col);
			break;
		case IDC_EDIT:
			list_edit=TRUE;
			DialogBox(ghinstance,MAKEINTRESOURCE(IDD_SERVER),hwnd,add_server);
			sort_listview(ghlistview,dir,col);
			break;
		case IDC_DELETE:
			if(get_selected_count(ghlistview)>1){
				if(show_messagebox(hwnd,"delete selected items?","Delete",MB_OKCANCEL)!=IDOK)
					break;
			}
			do_server_deletion(ghlistview);
			sort_listview(ghlistview,dir,col);
			SetFocus(ghlistview);
			break;
		case IDC_JOIN:
			goto connect_channel;
			break;
		case WM_DESTROY:
			save_ini_server_listview(ghlistview);
			EndDialog(hwnd,0);
			break;
		}
		break;
	case WM_CLOSE:
	case WM_QUIT:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}
//************************************************************************//
BOOL CALLBACK add_channel(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	extern short channel_list_anchors[];
	static HWND grippy=0;
	static int entry_num=-1;
	static char old_channel[80]={0};
	switch(msg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd,IDC_CHANNEL,EM_LIMITTEXT,80,0);
		SendDlgItemMessage(hwnd,IDC_PASSWORD,EM_LIMITTEXT,20,0);
		grippy=create_grippy(hwnd);
		if(list_edit){
			populate_add_channel_win(ghlistview,hwnd);
			old_channel[0]=0;
			GetWindowText(GetDlgItem(hwnd,IDC_CHANNEL),old_channel,sizeof(old_channel));
		}
		else{
			populate_channel_networks(hwnd);
			SendDlgItemMessage(hwnd,IDC_NETWORK,CB_SETCURSEL,0,0);
		}
		break;
	case WM_SIZE:
		grippy_move(hwnd,grippy);
		reposition_controls(hwnd,channel_list_anchors);
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		case WM_DESTROY:
			EndDialog(hwnd,0);
			break;
		case IDOK:
			if(GetFocus()==GetDlgItem(hwnd,IDOK)){
				save_channel_entry(hwnd,list_edit,old_channel);
				load_ini_channels(ghlistview);
				EndDialog(hwnd,0);
			}
			else
				PostMessage(hwnd,WM_NEXTDLGCTL,0,0);
			break;
		}
		break;
	case WM_CLOSE:
	case WM_QUIT:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}
BOOL CALLBACK channel_dlg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	extern short server_list_anchors[];
	static int dir=FALSE,col=0;
	static DWORD tick;
	static HWND grippy=0,chan_dlg_menu=0;
	if(FALSE)
	if(msg!=WM_MOUSEFIRST&&msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE/*&&msg!=WM_NOTIFY*/)
	//if(msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE)
	{
		if((GetTickCount()-tick)>500)
			printf("--\n");
		print_msg(msg,lparam,wparam,hwnd);
		tick=GetTickCount();
	}
	switch(msg)
	{
	case WM_INITDIALOG:
		SetWindowText(hwnd,"Channels");
		grippy=create_grippy(hwnd);
		ghlistview=create_listview(hwnd,ghinstance);
		load_ini_channel_listview(ghlistview);
		load_ini_channels(ghlistview);
		sort_listview(ghlistview,dir,col);
		if(chan_dlg_menu==0)
			chan_dlg_menu=create_channel_dlg_menu(hwnd);
		break;
	case WM_SIZE:
		grippy_move(hwnd,grippy);
		reposition_controls(hwnd,server_list_anchors);
		break;
	case WM_NOTIFY:
		{
			NMHDR *nmh=lparam;
			NMITEMACTIVATE *nmitem=lparam;
			if(nmh->hwndFrom==ghlistview){
				switch(nmh->code){
				case NM_RCLICK:
					if(chan_dlg_menu!=0 && get_selected_count(ghlistview)>0){
						POINT p;
						GetCursorPos(&p);
						TrackPopupMenu(chan_dlg_menu,TPM_LEFTALIGN,p.x,p.y,0,hwnd,NULL);
					}
					break;
				case NM_DBLCLK:
					if(nmitem->iItem<0)
						break;
					if((GetKeyState(VK_CONTROL)&0x8000) || (GetKeyState(VK_SHIFT)&0x8000)){
						list_edit=TRUE;
						DialogBox(ghinstance,MAKEINTRESOURCE(IDD_CHANNEL),hwnd,add_channel);
						sort_listview(ghlistview,dir,col);
					}
					else{
						int item;
						char network[80]={0},channel[80],password[20]={0};
join_channel:
						item=get_focused_item(ghlistview);
						if(item>=0){
							ListView_GetItemText(ghlistview,item,0,channel,sizeof(channel));
							ListView_GetItemText(ghlistview,item,1,network,sizeof(network));
							ListView_GetItemText(ghlistview,item,3,password,sizeof(password));
							join_channel(ghmdiclient,network,channel,password);
							EndDialog(hwnd,0);
						}

					}
					break;
				case LVN_KEYDOWN:
					{
						LV_KEYDOWN *key=lparam;
						switch(key->wVKey){
						case VK_SPACE:
							list_edit=TRUE;
							DialogBox(ghinstance,MAKEINTRESOURCE(IDD_CHANNEL),hwnd,add_channel);
							sort_listview(ghlistview,dir,col);
							break;
						case VK_INSERT:
							list_edit=FALSE;
							DialogBox(ghinstance,MAKEINTRESOURCE(IDD_CHANNEL),hwnd,add_channel);
							sort_listview(ghlistview,dir,col);
							break;
						case VK_DELETE:
							if(get_selected_count(ghlistview)>1){
								if(show_messagebox(hwnd,"delete selected items?","Delete",MB_OKCANCEL)!=IDOK)
									break;
							}
							do_channel_deletion(ghlistview);
							sort_listview(ghlistview,dir,col);
							break;
						case 'A':
							if(GetKeyState(VK_CONTROL)&0x8000)
								list_select_all(ghlistview);
							break;
						}
					}
					break;
				case  LVN_COLUMNCLICK:
					{
						NMLISTVIEW *nmlv=lparam;
						col=nmlv->iSubItem;
						dir=!dir;
						sort_listview(ghlistview,dir,col);
					}
					break;
				}

			}
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		default:
			if(channel_dlg_popup_cmd(LOWORD(wparam),ghlistview))
				sort_listview(ghlistview,dir,col);
			break;
		case IDC_JOIN:
			goto join_channel;
			break;
		case IDC_ADD:
			list_edit=FALSE;
			DialogBox(ghinstance,MAKEINTRESOURCE(IDD_CHANNEL),hwnd,add_channel);
			sort_listview(ghlistview,dir,col);
			break;
		case IDC_DELETE:
			if(get_selected_count(ghlistview)>1){
				if(show_messagebox(hwnd,"delete selected items?","Delete",MB_OKCANCEL)!=IDOK)
					break;
			}
			do_channel_deletion(ghlistview);
			sort_listview(ghlistview,dir,col);
			SetFocus(ghlistview);
			break;
		case IDC_EDIT:
			list_edit=TRUE;
			DialogBox(ghinstance,MAKEINTRESOURCE(IDD_CHANNEL),hwnd,add_channel);
			sort_listview(ghlistview,dir,col);
			break;
		case WM_DESTROY:
			save_ini_channel_listview(ghlistview);
			EndDialog(hwnd,0);
			break;
		}
		break;
	case WM_CLOSE:
	case WM_QUIT:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}
/**********************************************************************************************/
BOOL CALLBACK settings_dlg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	extern short settings_list_anchors[];
	extern int show_joins,log_enable,lua_script_enable,debug_level;
	static HWND grippy=0;
	static int help_active=FALSE;
	int debug;
	char str[255];

	switch(msg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd,IDC_NICK,EM_LIMITTEXT,80,0);
		SendDlgItemMessage(hwnd,IDC_QUIT_MSG,EM_LIMITTEXT,128,0);
		SendDlgItemMessage(hwnd,IDC_USERNAME,EM_LIMITTEXT,80,0);
		SendDlgItemMessage(hwnd,IDC_REALNAME,EM_LIMITTEXT,80,0);
		SendDlgItemMessage(hwnd,IDC_DEBUG_LEVEL,EM_LIMITTEXT,16,0);
		str[0]=0;
		if(get_ini_str("SETTINGS","NICK",str,sizeof(str)))
			SetWindowText(GetDlgItem(hwnd,IDC_NICK),str);
		str[0]=0;
		if(get_ini_str("SETTINGS","QUIT_MSG",str,sizeof(str)))
			SetWindowText(GetDlgItem(hwnd,IDC_QUIT_MSG),str);
		str[0]=0;
		if(get_ini_str("SETTINGS","user_name",str,sizeof(str)))
			SetWindowText(GetDlgItem(hwnd,IDC_USERNAME),str);
		str[0]=0;
		if(get_ini_str("SETTINGS","real_name",str,sizeof(str)))
			SetWindowText(GetDlgItem(hwnd,IDC_REALNAME),str);

		get_ini_value("SETTINGS","SHOW_JOINS",&show_joins);
		CheckDlgButton(hwnd,IDC_SHOWJOINS,show_joins);
		get_ini_value("SETTINGS","ENABLE_LOG",&log_enable);
		CheckDlgButton(hwnd,IDC_ENABLELOG,log_enable);
		get_ini_value("SETTINGS","ENABLE_LUA_SCRIPT",&lua_script_enable);
		CheckDlgButton(hwnd,IDC_LUA_SCRIPT,lua_script_enable);
		debug=0;
		get_ini_value("SETTINGS","DEBUG",&debug);
		if(debug!=0){
			CheckDlgButton(hwnd,IDC_DEBUG,BST_CHECKED);
			_snprintf(str,sizeof(str),"%i",debug_level);
			SetWindowText(GetDlgItem(hwnd,IDC_DEBUG_LEVEL),str);
		}
		else
			ShowWindow(GetDlgItem(hwnd,IDC_DEBUG_LEVEL),SW_HIDE);
		grippy=create_grippy(hwnd);
		reposition_controls(hwnd,settings_list_anchors);
		break;
	case WM_SIZE:
		grippy_move(hwnd,grippy);
		reposition_controls(hwnd,settings_list_anchors);
		break;
	case WM_HELP:
		if(!help_active){
			help_active=TRUE;
			MessageBox(hwnd,"ini SETTINGS->SLAPMSG for setting default slap message","SETTINGS INFO",MB_OK);
			help_active=FALSE;
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		case IDC_DEBUG_LEVEL:
			if(HIWORD(wparam)==EN_CHANGE){
				str[0]=0;
				GetWindowText(lparam,str,sizeof(str));
				if(str[0]!=0){
					debug_level=atoi(str);
					printf("debug_level=%i\n",debug_level);
				}
			}
			break;
		case IDC_DEBUG:
			ShowWindow(GetDlgItem(hwnd,IDC_DEBUG_LEVEL),IsDlgButtonChecked(hwnd,IDC_DEBUG));
			break;
		case IDC_OPENINI:
			open_ini(hwnd);
			break;
		case WM_DESTROY:
			EndDialog(hwnd,0);
			break;
		case IDOK:
			if(GetFocus()==GetDlgItem(hwnd,IDOK)){
				str[0]=0;
				GetWindowText(GetDlgItem(hwnd,IDC_NICK),str,sizeof(str));
				write_ini_str("SETTINGS","NICK",str);
				str[0]=0;
				GetWindowText(GetDlgItem(hwnd,IDC_QUIT_MSG),str,sizeof(str));
				write_ini_str("SETTINGS","QUIT_MSG",str);
				str[0]=0;
				GetWindowText(GetDlgItem(hwnd,IDC_USERNAME),str,sizeof(str));
				write_ini_str("SETTINGS","user_name",str);
				str[0]=0;
				GetWindowText(GetDlgItem(hwnd,IDC_REALNAME),str,sizeof(str));
				write_ini_str("SETTINGS","real_name",str);
				if(IsDlgButtonChecked(hwnd,IDC_SHOWJOINS)==BST_CHECKED)
					show_joins=1;
				else
					show_joins=0;
				write_ini_value("SETTINGS","SHOW_JOINS",show_joins);

				if(IsDlgButtonChecked(hwnd,IDC_ENABLELOG)==BST_CHECKED)
					log_enable=1;
				else
					log_enable=0;
				write_ini_value("SETTINGS","SHOW_JOINS",lua_script_enable);

				if(IsDlgButtonChecked(hwnd,IDC_LUA_SCRIPT)==BST_CHECKED)
					lua_script_enable=1;
				else
					lua_script_enable=0;
				write_ini_value("SETTINGS","ENABLE_LUA_SCRIPT",lua_script_enable);

				if(IsDlgButtonChecked(hwnd,IDC_DEBUG)==BST_CHECKED){
					open_console();
					debug=1;
				}
				else{
					hide_console();
					debug=0;
				}
				write_ini_value("SETTINGS","DEBUG",debug);
				EndDialog(hwnd,0);
			}
			else
				PostMessage(hwnd,WM_NEXTDLGCTL,0,0);
			break;
		}
		break;
	case WM_CLOSE:
	case WM_QUIT:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}
int load_icon(HWND hwnd)
{
	HICON hIcon = LoadIcon(ghinstance,MAKEINTRESOURCE(IDI_ICON));
    if(hIcon){
		SendMessage(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hIcon);
		SendMessage(hwnd,WM_SETICON,ICON_BIG,(LPARAM)hIcon);
		return TRUE;
	}
	return FALSE;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static DWORD tick=0;
	RECT rect;
	if(control_debug("main",0))
	if(msg!=WM_MOUSEFIRST&&msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE&&msg!=WM_NOTIFY)
	//if(msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE)
	{
		if((GetTickCount()-tick)>500)
			printf("--\n");
		print_msg(msg,lparam,wparam,hwnd);
		tick=GetTickCount();
	}
    switch(msg)
    {
	case WM_CREATE:
		{
			int width,height,x=0,y=0;
			load_icon(hwnd);
			if(get_ini_value("SETTINGS","main_dlg_width",&width)&&
				get_ini_value("SETTINGS","main_dlg_height",&height)){
				SetWindowPos(hwnd,NULL,0,0,width,height,SWP_NOZORDER|SWP_NOMOVE);
			}
			get_ini_value("SETTINGS","main_dlg_xpos",&x);
			get_ini_value("SETTINGS","main_dlg_ypos",&y);
			if(GetWindowRect(GetDesktopWindow(),&rect)!=0){
				if(x<-32 || y<=-32)
					break;
				if(x<((rect.right-rect.left)-50))
					if(y<((rect.bottom-rect.top)-50))
						if(SetWindowPos(hwnd,HWND_TOP,x,y,0,0,SWP_NOZORDER|SWP_NOSIZE)!=0)
							return TRUE;
			}
		}
		break;
	case WM_USER:
		update_status_window(hwnd,ghmenu);
		break;
	case WM_ACTIVATE:
		break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
		ReleaseCapture();
		main_drag=FALSE;
		write_ini_value("SETTINGS","SWITCH_HEIGHT",switch_height);
		break;
	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		SetCursor(LoadCursor(NULL,IDC_SIZENS));
		main_drag=TRUE;
		break;
	case WM_MOUSEFIRST:
		{
			int y=HIWORD(lparam);
			int x=LOWORD(lparam);
			SetCursor(LoadCursor(NULL,IDC_SIZENS));
			if(main_drag){
				RECT rect;
				GetClientRect(ghmainframe,&rect);
				if((rect.bottom-y)>5){
					switch_height=rect.bottom-y;
					//SendMessage(hwnd,WM_SIZE,0,0);
					resize_switchbar(ghmainframe,ghmdiclient,ghswitchbar,switch_height);
				}
			}
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam)){
		case ID_SERVERS:
			DialogBox(ghinstance,MAKEINTRESOURCE(IDD_LIST),hwnd,server_dlg);
			break;
		case ID_CHANNELS:
			DialogBox(ghinstance,MAKEINTRESOURCE(IDD_LIST),hwnd,channel_dlg);
			break;
		case ID_SETTINGS:
			DialogBox(ghinstance,MAKEINTRESOURCE(IDD_SETTINGS),hwnd,settings_dlg);
			break;
		case ID_CONNECT:
			if(!thread_status(FALSE,FALSE))
				_beginthread(auto_connect,0,ghmdiclient);
			break;
		case ID_DISCONNECT:
			exit_irc(TRUE);
			break;
		case ID_WINDOW_LIST:
			printf("made it\n");
			break;
		}
		break;
	case WM_KILLFOCUS:
		break;
	case WM_NCCALCSIZE:
		break;
	case WM_SIZE:
		resize_switchbar(hwnd,ghmdiclient,ghswitchbar,switch_height);
		return 0;
		break;
	case WM_ENDSESSION:
		if(!wparam)
			break;
		if(lparam!=0)
			break;
	case WM_CLOSE:
		exit_irc(FALSE);
        break;
	case WM_DESTROY:
		{
			RECT rect;
			GetWindowRect(hwnd,&rect);
			write_ini_value("SETTINGS","main_dlg_width",rect.right-rect.left);
			write_ini_value("SETTINGS","main_dlg_height",rect.bottom-rect.top);
			write_ini_value("SETTINGS","main_dlg_xpos",rect.left);
			write_ini_value("SETTINGS","main_dlg_ypos",rect.top);
		}
		PostQuitMessage(0);
        break;
    }
	return DefFrameProc(hwnd, ghmdiclient, msg, wparam, lparam);
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
    INITCOMMONCONTROLSEX ctrls;
	WSADATA wsaData;
	int debug=0;
#ifdef _DEBUG
	debug=1;
#endif

	ghinstance=hInstance;
	init_ini_file();
	get_ini_value("SETTINGS","DEBUG",&debug);
	if(debug!=0){
		open_console();
		move_console();
	}
	init_mdi_stuff();
	get_ini_value("SETTINGS","SWITCH_HEIGHT",&switch_height);
	LoadLibrary("RICHED20.DLL");

	ctrls.dwSize=sizeof(ctrls);
    ctrls.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&ctrls);
	WSAStartup(MAKEWORD(1,1),&wsaData);
	
	InitializeCriticalSection(&mutex);

	setup_mdi_classes(ghinstance);
	
	ghmenu=LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));
	ghmainframe=create_mainwindow(&WndProc,ghmenu,hInstance);

	ghmdiclient=create_mdiclient(ghmainframe,ghmenu,ghinstance);
	ghswitchbar=create_switchbar(ghmainframe,ghinstance);

	ShowWindow(ghmainframe,nCmdShow);
	UpdateWindow(ghmainframe);

	_beginthread(auto_connect,0,ghmdiclient);

    while(GetMessage(&msg,NULL,0,0)){
		static DWORD tick=0;
		if(control_debug("x",0))
		//if(msg!=WM_MOUSEFIRST&&msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE&&msg!=WM_NOTIFY)
		if(msg.message!=0x118&&msg.message!=WM_NCHITTEST&&msg.message!=WM_SETCURSOR&&msg.message!=WM_ENTERIDLE)
		{
			if((GetTickCount()-tick)>500)
				printf("--\n");
			printf("x");
			print_msg(msg.message,msg.lParam,msg.wParam,msg.hwnd);
			tick=GetTickCount();
		}
		EnterCriticalSection(&mutex);
		if(!custom_dispatch(&msg))
		if(!TranslateMDISysAccel(ghmdiclient, &msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		LeaveCriticalSection(&mutex);
    }
	wait_for_disconnect();
	DeleteCriticalSection(&mutex);
    return msg.wParam;
	
}



