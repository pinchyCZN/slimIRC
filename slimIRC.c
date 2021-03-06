#define WINVER 0x500
#define _WIN32_WINNT 0x500
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

int restore_list_pos(HWND hlistview,RECT *rect,int item)
{
	RECT newrect={0};
	int dx,dy,count;
	if(hlistview==0 || rect==0)
		return FALSE;
	ListView_GetItemRect(hlistview,0,&newrect,LVIR_BOUNDS);
	dx=-rect->left+newrect.left;
	dy=-rect->top+newrect.top;
	if(dx!=0 || dy!=0)
		ListView_Scroll(hlistview,dx,dy);
	count=ListView_GetItemCount(hlistview);
	if(item>=count && count!=0)
		item=count-1;
	if(item>=0)
		ListView_SetItemState(hlistview,item,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
	return TRUE;
}

BOOL CALLBACK add_server(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static HWND grippy=0;
	static char old_server[160]={0};
	static int help_active=FALSE;
	static int list_edit=FALSE;
	switch(msg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd,IDC_NETWORK,EM_LIMITTEXT,80,0);
		SendDlgItemMessage(hwnd,IDC_SERVER,EM_LIMITTEXT,80,0);
		SendDlgItemMessage(hwnd,IDC_PORTS,EM_LIMITTEXT,40,0);
		SendDlgItemMessage(hwnd,IDC_PASSWORD,EM_LIMITTEXT,40,0);
		SendDlgItemMessage(hwnd,IDC_USER,EM_LIMITTEXT,40,0);
		SendDlgItemMessage(hwnd,IDC_NICK,EM_LIMITTEXT,20,0);
		grippy=create_grippy(hwnd);
		list_edit=lparam;
		if(list_edit){
			int len;
			populate_add_server_win(ghlistview,hwnd);
			old_server[0]=0;
			GetWindowText(GetDlgItem(hwnd,IDC_NETWORK),old_server,sizeof(old_server));
			len=strlen(old_server);
			if(len>0 && len<(sizeof(old_server)-2)){
				char *s=old_server+len;
				s[0]='|';s[1]=0;
				GetWindowText(GetDlgItem(hwnd,IDC_SERVER),s+1,
					sizeof(old_server)-len-1);
			}
		}
		else
			SetDlgItemText(hwnd,IDC_NETWORK,"default");
		help_active=FALSE;
		break;
	case WM_SIZE:
		grippy_move(hwnd,grippy);
		reposition_add_server(hwnd);
		break;
	case WM_HELP:
		if(!help_active){
			help_active=TRUE;
			MessageBox(hwnd,"add \"IPV6:\" in front of server name to use ipv6\r\n"
				"## to verify SSL certs\r\n"
				"[user|]password - user name optional, seperate user/pass with pipe (|) symbol","Help",MB_OK);
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
	static HWND grippy=0;
	static HMENU server_dlg_menu=0;
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
		SetFocus(GetDlgItem(hwnd,IDC_ADD));
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
						PostMessage(hwnd,WM_COMMAND,IDC_EDIT,0);
					}
					else{
						int item;
						char server[80]={0},network[80]={0},port[40]={0},ssl[20]={0},password[40]={0};
						char user[40]={0},nick[20]={0};
connect_channel:
						item=get_focused_item(ghlistview);
						if(item>=0){
							ListView_GetItemText(ghlistview,item,0,network,sizeof(network));
							ListView_GetItemText(ghlistview,item,1,server,sizeof(server));
							ListView_GetItemText(ghlistview,item,2,port,sizeof(port));
							ListView_GetItemText(ghlistview,item,4,ssl,sizeof(ssl));
							ListView_GetItemText(ghlistview,item,5,password,sizeof(password));
							ListView_GetItemText(ghlistview,item,6,user,sizeof(user));
							ListView_GetItemText(ghlistview,item,7,nick,sizeof(nick));
							if(connect_server(ghmdiclient,network,server,atoi(port),ssl[0]!=0,password,user,nick))
								EndDialog(hwnd,0);
							else
								SetFocus(ghlistview);
						}
					}
					break;
				case LVN_KEYDOWN:
					{
						LV_KEYDOWN *key=lparam;
						switch(key->wVKey){
						case VK_INSERT:
							PostMessage(hwnd,WM_COMMAND,IDC_ADD,0);
							break;
						case VK_F2:
						case VK_SPACE:
							PostMessage(hwnd,WM_COMMAND,IDC_EDIT,0);
							break;
						case VK_DELETE:
							PostMessage(hwnd,WM_COMMAND,IDC_DELETE,0);
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
		{
			RECT rect={0};
			int list_edit,item;
			item=get_focused_item(ghlistview);
			ListView_GetItemRect(ghlistview,0,&rect,LVIR_BOUNDS);
			switch(LOWORD(wparam)){
			default:
				if(server_dlg_popup_cmd(LOWORD(wparam),ghlistview)){
					sort_listview(ghlistview,dir,col);
					restore_list_pos(ghlistview,&rect,item);
				}
				break;
			case IDC_ADD:
				list_edit=FALSE;
				DialogBoxParam(ghinstance,MAKEINTRESOURCE(IDD_SERVER),hwnd,add_server,list_edit);
				sort_listview(ghlistview,dir,col);
				restore_list_pos(ghlistview,&rect,item);
				break;
			case IDC_EDIT:
				list_edit=TRUE;
				DialogBoxParam(ghinstance,MAKEINTRESOURCE(IDD_SERVER),hwnd,add_server,list_edit);
				sort_listview(ghlistview,dir,col);
				restore_list_pos(ghlistview,&rect,item);
				break;
			case IDC_DELETE:
				if(get_selected_count(ghlistview)>1){
					if(show_messagebox(hwnd,"delete selected items?","Delete",MB_OKCANCEL)!=IDOK)
						break;
				}
				delete_selected_server(ghlistview);
				load_ini_servers(ghlistview);
				sort_listview(ghlistview,dir,col);
				restore_list_pos(ghlistview,&rect,item);
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
		}
		break;
	case WM_CLOSE:
	case WM_QUIT:
		save_ini_server_listview(ghlistview);
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
	static int network_num=0;
	static int list_edit=FALSE;
	static char old_channel[80]={0};

	switch(msg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd,IDC_CHANNEL,EM_LIMITTEXT,80,0);
		SendDlgItemMessage(hwnd,IDC_PASSWORD,EM_LIMITTEXT,20,0);
		grippy=create_grippy(hwnd);
		list_edit=lparam;
		if(list_edit){
			populate_add_channel_win(ghlistview,hwnd);
			old_channel[0]=0;
			GetWindowText(GetDlgItem(hwnd,IDC_CHANNEL),old_channel,sizeof(old_channel));
		}
		else{
			int count=0;
			populate_channel_networks(hwnd);
			count=SendDlgItemMessage(hwnd,IDC_NETWORK,CB_GETCOUNT,0,0);
			if(network_num<0 || network_num>=count)
				network_num=0;
			SendDlgItemMessage(hwnd,IDC_NETWORK,CB_SETCURSEL,network_num,0);
		}
		break;
	case WM_SIZE:
		grippy_move(hwnd,grippy);
		reposition_controls(hwnd,channel_list_anchors);
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		case IDC_NETWORK:
			switch(HIWORD(wparam)){
			case CBN_SELCHANGE:
				network_num=SendMessage(lparam,CB_GETCURSEL,0,0);
				break;
			}
			break;
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
	static HWND grippy=0;
	static HMENU chan_dlg_menu=0;
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
						PostMessage(hwnd,WM_COMMAND,IDC_EDIT,0);
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
							if(join_channel(ghmdiclient,network,channel,password))
								EndDialog(hwnd,0);
							else{
								char str[80];
								_snprintf(str,sizeof(str),"Join [%s] network from server list first",network);
								show_messagebox(hwnd,str,"Unable to join channel",MB_OK);
							}
						}
					}
					break;
				case LVN_KEYDOWN:
					{
						LV_KEYDOWN *key=lparam;
						switch(key->wVKey){
						case VK_F2:
						case VK_SPACE:
							PostMessage(hwnd,WM_COMMAND,IDC_EDIT,0);
							break;
						case VK_INSERT:
							PostMessage(hwnd,WM_COMMAND,IDC_ADD,0);
							break;
						case VK_DELETE:
							PostMessage(hwnd,WM_COMMAND,IDC_DELETE,0);
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
		{
			RECT rect={0};
			int list_edit,item;
			item=get_focused_item(ghlistview);
			ListView_GetItemRect(ghlistview,0,&rect,LVIR_BOUNDS);
			switch(LOWORD(wparam))
			{
			default:
				if(channel_dlg_popup_cmd(LOWORD(wparam),ghlistview)){
					sort_listview(ghlistview,dir,col);
					restore_list_pos(ghlistview,&rect,item);
				}
				break;
			case IDC_JOIN:
				goto join_channel;
				break;
			case IDC_ADD:
				list_edit=FALSE;
				DialogBoxParam(ghinstance,MAKEINTRESOURCE(IDD_CHANNEL),hwnd,add_channel,list_edit);
				sort_listview(ghlistview,dir,col);
				restore_list_pos(ghlistview,&rect,item);
				break;
			case IDC_DELETE:
				if(get_selected_count(ghlistview)>1){
					if(show_messagebox(hwnd,"delete selected items?","Delete",MB_OKCANCEL)!=IDOK)
						break;
				}
				delete_selected_channel(ghlistview);
				load_ini_channels(ghlistview);
				sort_listview(ghlistview,dir,col);
				restore_list_pos(ghlistview,&rect,item);
				SetFocus(ghlistview);
				break;
			case IDC_EDIT:
				list_edit=TRUE;
				DialogBoxParam(ghinstance,MAKEINTRESOURCE(IDD_CHANNEL),hwnd,add_channel,list_edit);
				sort_listview(ghlistview,dir,col);
				restore_list_pos(ghlistview,&rect,item);
				break;
			case WM_DESTROY:
				save_ini_channel_listview(ghlistview);
				EndDialog(hwnd,0);
				break;
			}
		}
		break;
	case WM_CLOSE:
	case WM_QUIT:
		save_ini_channel_listview(ghlistview);
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}
int set_single_instance(int set)
{
	static char *instname="_slimirc_ Instance";
	static HANDLE hmutex=0;
	if(set){
		SetLastError(NO_ERROR);
		if(hmutex==0)
			hmutex=CreateMutex(NULL,FALSE,instname);
		if(GetLastError()==ERROR_ALREADY_EXISTS)
			return FALSE;
		else
			return TRUE;
	}
	else{
		if(hmutex!=0){
			if(WaitForSingleObject(hmutex,0)==WAIT_OBJECT_0)
				if(ReleaseMutex(hmutex)!=0)
					if(CloseHandle(hmutex)!=0){
						hmutex=0;
						return TRUE;
					}
		}
	}
	return FALSE;
}
/**********************************************************************************************/
BOOL CALLBACK settings_dlg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	extern short settings_list_anchors[];
	extern int show_joins,show_parts,log_enable,lua_script_enable;
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
		get_ini_value("SETTINGS","SHOW_PARTS",&show_parts);
		CheckDlgButton(hwnd,IDC_SHOWPARTS,show_parts);
		get_ini_value("SETTINGS","ENABLE_LOG",&log_enable);
		CheckDlgButton(hwnd,IDC_ENABLELOG,log_enable);
		{
			int val=0;
			get_ini_value("SETTINGS","SINGLE_INSTANCE",&val);
			CheckDlgButton(hwnd,IDC_SINGLE_INSTANCE,val);
		}
		get_ini_value("SETTINGS","ENABLE_LUA_SCRIPT",&lua_script_enable);
		CheckDlgButton(hwnd,IDC_LUA_SCRIPT,lua_script_enable);
		debug=0;
		get_ini_value("SETTINGS","DEBUG",&debug);
		if(debug!=0){
			CheckDlgButton(hwnd,IDC_DEBUG,BST_CHECKED);
			_snprintf(str,sizeof(str),"%i",get_irc_debug_level());
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
					set_irc_debug_level(atoi(str));
					printf("debug_level=%i\n",get_irc_debug_level());
				}
			}
			break;
		case IDC_DEBUG:
			ShowWindow(GetDlgItem(hwnd,IDC_DEBUG_LEVEL),IsDlgButtonChecked(hwnd,IDC_DEBUG));
			_snprintf(str,sizeof(str),"%i",get_irc_debug_level());
			SetWindowText(GetDlgItem(hwnd,IDC_DEBUG_LEVEL),str);
			break;
		case IDC_OPEN_INI_FOLDER:
			open_ini(hwnd,TRUE);
			break;
		case IDC_OPENINI:
			open_ini(hwnd,FALSE);
			break;
		case IDC_LUASCRIPT:
			{
				char fname[MAX_PATH]={0};
				get_lua_script_fname(fname,sizeof(fname));
				GetContextMenu(hwnd,fname);
			}
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

				if(IsDlgButtonChecked(hwnd,IDC_SHOWPARTS)==BST_CHECKED)
					show_parts=1;
				else
					show_parts=0;
				write_ini_value("SETTINGS","SHOW_PARTS",show_parts);

				if(IsDlgButtonChecked(hwnd,IDC_ENABLELOG)==BST_CHECKED)
					log_enable=1;
				else{
					log_enable=0;
					log_str(0,0,0);
				}
				write_ini_value("SETTINGS","ENABLE_LOG",log_enable);

				if(IsDlgButtonChecked(hwnd,IDC_LUA_SCRIPT)==BST_CHECKED)
					lua_script_enable=1;
				else{
					lua_script_enable=0;
					hide_tooltip(); //clear lua script not found message
				}
				write_ini_value("SETTINGS","ENABLE_LUA_SCRIPT",lua_script_enable);

				{
					int val=0;
					if(IsDlgButtonChecked(hwnd,IDC_SINGLE_INSTANCE)==BST_CHECKED)
						val=1;
					write_ini_value("SETTINGS","SINGLE_INSTANCE",val);
					set_single_instance(val);
				}

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
int save_window_position(HWND hwnd)
{
	RECT rect={0};
	WINDOWPLACEMENT wp;
	int w=0,h=0,maximized=0;
	if(GetKeyState(VK_SHIFT)&0x8000)
		return FALSE;
	wp.length=sizeof(wp);
	if(GetWindowPlacement(hwnd,&wp)!=0){
		rect=wp.rcNormalPosition;
		if(wp.flags&WPF_RESTORETOMAXIMIZED)
			maximized=1;
		w=rect.right-rect.left;
		h=rect.bottom-rect.top;
		write_ini_value("SETTINGS","main_dlg_width",w);
		write_ini_value("SETTINGS","main_dlg_height",h);
		write_ini_value("SETTINGS","main_dlg_xpos",rect.left);
		write_ini_value("SETTINGS","main_dlg_ypos",rect.top);
		write_ini_value("SETTINGS","main_dlg_maximized",maximized);
		return TRUE;
	}
	return FALSE;
}
int snap_window(HWND hwnd,RECT *rect)
{
	if(hwnd && rect){
		HMONITOR hmon;
		MONITORINFO mi;
		hmon=MonitorFromRect(rect,MONITOR_DEFAULTTONEAREST);
		mi.cbSize=sizeof(mi);
		if(GetMonitorInfo(hmon,&mi)){
			long d_top,d_bottom,d_left,d_right;
			d_right=mi.rcWork.right-rect->right;
			if(d_right<=8 && d_right>=-4){
				rect->right=mi.rcWork.right;
				rect->left+=d_right;
			}
			d_left=rect->left-mi.rcWork.left;
			if(d_left<=8 && d_left>=-4){
				rect->left=mi.rcWork.left;
				rect->right-=d_left;
			}
			d_top=rect->top-mi.rcWork.top;
			if(d_top<=8 && d_top>=-4){
				rect->top=mi.rcWork.top;
				rect->bottom-=d_top;
			}
			d_bottom=mi.rcWork.bottom-rect->bottom;
			if(d_bottom<=8 && d_bottom>=-4){
				rect->bottom=mi.rcWork.bottom;
				rect->top+=d_bottom;
			}
		}
	}
	return 0;
}
int snap_sizing(HWND hwnd,RECT *rect,int side)
{
	int result=FALSE;
	if(hwnd && rect){
		HMONITOR hmon;
		MONITORINFO mi;
		hmon=MonitorFromRect(rect,MONITOR_DEFAULTTONEAREST);
		mi.cbSize=sizeof(mi);
		if(GetMonitorInfo(hmon,&mi)){
			RECT *rwork=&mi.rcWork;
			const int snap_size=10;
			if(side==WMSZ_TOP || side==WMSZ_TOPLEFT || side==WMSZ_TOPRIGHT){
				if(abs(rect->top - rwork->top)<snap_size){
					rect->top=rwork->top;
					result=TRUE;
				}
			}
			if(side==WMSZ_BOTTOM || side==WMSZ_BOTTOMLEFT || side==WMSZ_BOTTOMRIGHT){
				if(abs(rect->bottom - rwork->bottom)<snap_size){
					rect->bottom=rwork->bottom;
					result=TRUE;
				}
			}
			if(side==WMSZ_LEFT || side==WMSZ_TOPLEFT || side==WMSZ_BOTTOMLEFT){
				if(abs(rect->left - rwork->left)<snap_size){
					rect->left=rwork->left;
					result=TRUE;
				}
			}
			if(side==WMSZ_RIGHT || side==WMSZ_TOPRIGHT || side==WMSZ_BOTTOMRIGHT){
				if(abs(rect->right - rwork->right)<snap_size){
					rect->right=rwork->right;
					result=TRUE;
				}
			}
		}
	}
	return result;
}
int clamp_window_size(int *x,int *y,int *width,int *height,RECT *monitor)
{
	int mwidth,mheight;
	mwidth=monitor->right-monitor->left;
	mheight=monitor->bottom-monitor->top;
	if(mwidth<=0)
		return FALSE;
	if(mheight<=0)
		return FALSE;
	if(*x<monitor->left)
		*x=monitor->left;
	if(*width>mwidth)
		*width=mwidth;
	if(*x+*width>monitor->right)
		*x=monitor->right-*width;
	if(*y<monitor->top)
		*y=monitor->top;
	if(*height>mheight)
		*height=mheight;
	if(*y+*height>monitor->bottom)
		*y=monitor->bottom-*height;
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if(control_debug(IDC_TOP_WNDPROC,0,0))
	//if(msg!=WM_MOUSEFIRST&&msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE&&msg!=WM_NOTIFY)
	//if(msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE)
	{
		static DWORD tick=0;
		if((GetTickCount()-tick)>500)
			printf("--\n");
		print_msg(msg,lparam,wparam,hwnd);
		tick=GetTickCount();
	}
    switch(msg)
    {
	case WM_CREATE:
		{
			HMONITOR hmon;
			MONITORINFO mi;
			RECT rect;
			int width=0,height=0,x=0,y=0,maximized=0;
			load_icon(hwnd);
			get_ini_value("SETTINGS","main_dlg_xpos",&x);
			get_ini_value("SETTINGS","main_dlg_ypos",&y);
			get_ini_value("SETTINGS","main_dlg_width",&width);
			get_ini_value("SETTINGS","main_dlg_height",&height);
			rect.left=x;
			rect.top=y;
			rect.right=x+width;
			rect.bottom=y+height;
			hmon=MonitorFromRect(&rect,MONITOR_DEFAULTTONEAREST);
			mi.cbSize=sizeof(mi);
			if(GetMonitorInfo(hmon,&mi)){
				int flags=SWP_NOZORDER;
				if(width<=50 || height<=50)
					flags|=SWP_NOSIZE;
				if(!clamp_window_size(&x,&y,&width,&height,&mi.rcWork))
					flags|=SWP_NOMOVE;
				SetWindowPos(hwnd,HWND_TOP,x,y,width,height,flags);
			}
			get_ini_value("SETTINGS","main_dlg_maximized",&maximized);
			if(maximized!=0)
				PostMessage(hwnd,WM_SYSCOMMAND,SC_MAXIMIZE,0);

		}
		break;
	case WM_APP:
		switch(wparam){
		case MSG_UPDATE_STATUS:
			update_status_window(hwnd,ghmenu);
			break;
		case MSG_CREATE_TOOLTIP:
			create_tooltip(hwnd,LOWORD(lparam),HIWORD(lparam));
			break;
		case MSG_DESTROY_TOOLTIP:
			destroy_tooltip();
			break;
		}
		break;
	case WM_ACTIVATEAPP: //close any tooltip on app switch
		destroy_tooltip();
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
			break;
		case MDI_MENU_OPENLOG:
			{
				HWND h=GetTopWindow(ghmdiclient);
				handle_channel_menu(h,LOWORD(wparam));
			}
			break;
		}
		break;
	case WM_KILLFOCUS:
		break;
	case WM_NCCALCSIZE:
		break;
	case WM_MOVING:
		if(GetKeyState(VK_CONTROL)&0x8000)
			snap_window(hwnd,lparam);
		break;
	case WM_SIZING:
		if(GetKeyState(VK_CONTROL)&0x8000)
			return snap_sizing(hwnd,lparam,wparam);
		break;
	case WM_SIZE:
		resize_switchbar(hwnd,ghmdiclient,ghswitchbar,switch_height);
		return 0;
		break;
	case WM_QUERYENDSESSION:
		save_window_position(hwnd);
		return 1; //ok to end session
		break;
	case WM_ENDSESSION:
		if(wparam){ //session is being ended flag
			extern int wait_for_disconnect(CRITICAL_SECTION *mutex);
			exit_irc(FALSE);
			//wait_for_disconnect(&mutex);
			//_beginthread(wait_for_disconnect,0,&mutex);
		}
		return 0;
	case WM_CLOSE:
		save_window_position(hwnd);
		exit_irc(FALSE);
        break;
	case WM_DESTROY:
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
	int first_instance=TRUE;
	int debug=0;
#ifdef _DEBUG
	debug=1;
#endif
	ghinstance=hInstance;
	init_ini_file();
	first_instance=set_single_instance(TRUE);
	{
		int val=0;
		get_ini_value("SETTINGS","SINGLE_INSTANCE",&val);
		if(val && (!first_instance)){
			HWND horig;
			char *class_name="";
			get_window_classname(&class_name);
			horig=FindWindow(class_name,NULL);
			if(horig!=0){
				int sw;
				if (IsZoomed(horig))
					sw=SW_MAXIMIZE;
				else if(IsIconic(horig))
					sw=SW_RESTORE;
				else
					sw=SW_SHOW;
				ShowWindow(horig,sw);
				SetForegroundWindow(horig);
			}
			return TRUE;
		}
		set_single_instance(val);
	}
	get_ini_value("SETTINGS","DEBUG",&debug);
	if(debug!=0){
		set_irc_debug_level(debug);
		open_console();
		move_console();
	}
	init_mdi_stuff();
	get_ini_value("SETTINGS","SWITCH_HEIGHT",&switch_height);
	LoadLibrary("RICHED20.DLL");
	CoInitialize(0);

	ctrls.dwSize=sizeof(ctrls);
    ctrls.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&ctrls);
	WSAStartup(MAKEWORD(2,2),&wsaData);
	
	InitializeCriticalSection(&mutex);

	setup_mdi_classes(ghinstance);
	
	ghmenu=LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));
	ghmainframe=create_mainwindow(&WndProc,ghmenu,hInstance);

	ghmdiclient=create_mdiclient(ghmainframe,ghmenu,ghinstance);
	ghswitchbar=create_switchbar(ghmainframe,ghinstance);
	
	subclass_mdi_client(ghmdiclient);

	ShowWindow(ghmainframe,nCmdShow);
	UpdateWindow(ghmainframe);

	if(!(GetKeyState(VK_SHIFT)&0x8000))
		_beginthread(auto_connect,0,ghmdiclient);

    while(GetMessage(&msg,NULL,0,0)){
		if(control_debug(IDC_MAIN_MSG_PUMP,0,0))
		//if(msg!=WM_MOUSEFIRST&&msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE&&msg!=WM_NOTIFY)
		if(msg.message!=0x118&&msg.message!=WM_NCHITTEST&&msg.message!=WM_SETCURSOR&&msg.message!=WM_ENTERIDLE)
		{
			static DWORD tick=0;
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
	wait_for_disconnect(&mutex);
	close_all_logs();
	CoUninitialize();
	DeleteCriticalSection(&mutex);
    return msg.wParam;
	
}



