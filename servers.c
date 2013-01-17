#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#include "Commctrl.h"
#include "resource.h"
#define MAX_NETWORKS 100
#define MAX_SERVERS 15
#define MAX_CHANNELS 100
typedef struct {
	char *name;
	int width;
}COLUMN;

COLUMN server_columns[]={
	{"network",60},
	{"server",100},
	{"port",30},
	{"connect_on_startup",100},
	{"ssl",30},
	{"password",20},
};
int add_list(HWND hlist,char *entry)
{
	if(SendMessage(hlist,LB_FINDSTRINGEXACT,-1,entry)==LB_ERR)
		if(SendMessage(hlist,LB_ADDSTRING,0,entry)!=LB_ERR)
			return TRUE;
	return FALSE;
}
int set_ini_entry(char *section,int num,char *str)
{
	char key[20];
	_snprintf(key,sizeof(key),"ENTRY%i",num);
	if(write_ini_str(section,key,str))
		return TRUE;
	else
		return FALSE;
}
int get_ini_entry(char *section,int num,char *str,int len)
{
	char key[20];
	_snprintf(key,sizeof(key),"ENTRY%i",num);
	if(get_ini_str(section,key,str,len))
		return TRUE;
	else
		return FALSE;
}
int get_ini_entry_num(char *section,char *key)
{
	int i;
	char str[255];
	for(i=0;i<MAX_NETWORKS;i++){
		str[0]=0;
		if(get_ini_entry(section,i,str,sizeof(str))){
			if(str[0]!=0 && stricmp(str,key)==0)
				return i;
		}
	}
	return -1;
}


int fix_chan_name(char *chan,int size)
{
	char str[40];
	if(chan[0]!='#'){
		strncpy(str,chan,sizeof(str));
		_snprintf(chan,size,"#%s",str);
	}
	return TRUE;
}
int trim_str(char *str)
{
	char s[512];
	int start=FALSE;
	int i,index=0,len=strlen(str);
	if(len>=sizeof(s))
		len=sizeof(s)-1;
	if(len>0 && (str[0]<=' ' || str[len-1]<=' ')){
		for(i=0;i<len;i++){
			if(str[i]>' ')
				start=TRUE;
			if(start)
				s[index++]=str[i];
		}
		s[index++]=0;
		len=strlen(s);
		for(i=len-1;i>=0;i--){
			if(s[i]<=' ')
				s[i]=0;
			else
				break;
		}
		len=strlen(str);
		strncpy(str,s,len);
	}
	return TRUE;
}

int get_all_networks(HWND hwnd,int network_ctl)
{
	int i;
	char str[255],srv[255];
	for(i=0;i<MAX_NETWORKS;i++){
		str[0]=0;srv[0]=0;
		_snprintf(str,sizeof(str),"ENTRY%i",i);
		if(get_ini_str("SERVERS",str,srv,sizeof(srv))){

		}
	}
	return TRUE;
}
int find_server_string(char *network,char *out,int len)
{
	int i;
	char tmp[256],str[80];
	for(i=0;i<MAX_NETWORKS;i++){
		tmp[0]=0;
		if(get_ini_entry("SERVERS",i,tmp,sizeof(tmp))){
			str[0]=0;
		}
	}
	return FALSE;
}

int get_selected_count(HWND hlistview)
{
	int i,count,sel=0;
	count=ListView_GetItemCount(hlistview);
	for(i=0;i<count;i++){
		if(ListView_GetItemState(hlistview,i,LVIS_SELECTED)==LVIS_SELECTED)
			sel++;
	}
	return sel;
}
int get_focused_item(HWND hlistview)
{
	int i,count;
	count=ListView_GetItemCount(hlistview);
	for(i=0;i<count;i++){
		if(ListView_GetItemState(hlistview,i,LVIS_FOCUSED)==LVIS_FOCUSED)
			return i;
	}
	return -1;
}
int do_server_deletion(HWND hlistview)
{
	int count,item=get_focused_item(hlistview);
	delete_selected_server(hlistview);
	load_ini_servers(hlistview);
	count=ListView_GetItemCount(hlistview);
	if(item>=count)
		item-=1;
	ListView_SetItemState(hlistview,item,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
	return TRUE;
}
int delete_selected_server(HWND hlistview)
{
	int i,count,result=FALSE;
	count=ListView_GetItemCount(hlistview);
	for(i=0;i<count;i++){
		if(ListView_GetItemState(hlistview,i,LVIS_SELECTED)==LVIS_SELECTED){
			LV_ITEM lvi={0};
			char server[80]={0};
			lvi.mask=LVIF_TEXT;
			lvi.iItem=i;
			lvi.iSubItem=1;
			lvi.pszText=server;
			lvi.cchTextMax=sizeof(server);
			ListView_GetItem(hlistview,&lvi);
			if(server[0]!=0){
				int j;
				char key[20]={0},s[80]={0};
				delete_ini_section(server);
				for(j=0;j<MAX_SERVERS;j++){
					_snprintf(key,sizeof(key),"ENTRY%i",j);
					if(get_ini_str("SERVERS",key,s,sizeof(s)-1))
						if(s[0]!=0 && stricmp(s,server)==0){
							write_ini_str("SERVERS",key,"");
							result=TRUE;
							break;
						}
				}
			}
		}
	}
	return result;
}
int populate_add_server_win(HWND hlistview,HWND hwnd)
{
	char str[100]={0};
	LV_ITEM lvi={0};
	int item=get_focused_item(hlistview);
	lvi.mask=LVIF_TEXT;
	lvi.iItem=item;
	lvi.iSubItem=0;
	lvi.pszText=str;
	lvi.cchTextMax=sizeof(str);
	if(ListView_GetItem(hlistview,&lvi))
		SetDlgItemText(hwnd,IDC_NETWORK,str);
	str[0]=0;
	lvi.iSubItem=1;
	if(ListView_GetItem(hlistview,&lvi))
		SetDlgItemText(hwnd,IDC_SERVER,str);
	str[0]=0;
	lvi.iSubItem=2;
	if(ListView_GetItem(hlistview,&lvi))
		SetDlgItemText(hwnd,IDC_PORTS,str);
	str[0]=0;
	lvi.iSubItem=3;
	if(ListView_GetItem(hlistview,&lvi)){
		if(str[0]!=0)
			CheckDlgButton(hwnd,IDC_CONNECT_STARTUP,TRUE);
	}
	str[0]=0;
	lvi.iSubItem=4;
	if(ListView_GetItem(hlistview,&lvi)){
		if(str[0]!=0)
			CheckDlgButton(hwnd,IDC_SSL,TRUE);
	}
	str[0]=0;
	lvi.iSubItem=5;
	if(ListView_GetItem(hlistview,&lvi))
		SetDlgItemText(hwnd,IDC_PASSWORD,str);
	return TRUE;
}


/*
int get_column_index(COLUMN *c,int size,char *name,int *index)
{
	int i;
	for(i=0;i<size;i++){
		if(stricmp(c[i].name,name)==0){
			*index=i;
			return TRUE;
		}
	}
	return FALSE;
}
*/
int save_ini_server_listview(HWND hlistview)
{
	int i;
	char key[255];
	LV_COLUMN col;
	RECT rect;
	for(i=0;i<sizeof(server_columns)/sizeof(COLUMN);i++){
		_snprintf(key,sizeof(key),"server_width_%s",server_columns[i].name);
		col.mask=LVCF_WIDTH;
		col.cx=server_columns[i].width;
		ListView_GetColumn(hlistview,i,&col);
		write_ini_value("SETTINGS",key,col.cx);
	}
	if(GetWindowRect(GetParent(hlistview),&rect)){
		write_ini_value("SETTINGS","server_dlg_width",rect.right-rect.left);
		write_ini_value("SETTINGS","server_dlg_height",rect.bottom-rect.top);
	}
	return TRUE;
}
int load_ini_server_listview(HWND hlistview)
{
	int i,width,height;
	char key[255];
	LV_COLUMN col;
	for(i=0;i<sizeof(server_columns)/sizeof(COLUMN);i++){
		sprintf(key,"server_width_%s",server_columns[i].name);
		get_ini_value("SETTINGS",key,&server_columns[i].width);
	}
	for(i=0;i<sizeof(server_columns)/sizeof(COLUMN);i++){
		col.mask = LVCF_WIDTH|LVCF_TEXT;
		col.cx = server_columns[i].width;
		col.pszText = server_columns[i].name;
		ListView_InsertColumn(hlistview,i,&col);
	}
	if(get_ini_value("SETTINGS","server_dlg_width",&width)&&
		get_ini_value("SETTINGS","server_dlg_height",&height)){
		SetWindowPos(GetParent(hlistview),NULL,0,0,width,height,SWP_NOZORDER|SWP_NOMOVE);
	}
	return TRUE;
}
int load_ini_servers(HWND hlistview)
{
	int i,count=0;
	char key[255],server[255];
	ListView_DeleteAllItems(hlistview);
	for(i=0;i<MAX_SERVERS;i++){
		_snprintf(key,sizeof(key),"ENTRY%i",i);
		server[0]=0;
		if(get_ini_str("SERVERS",key,server,sizeof(server))){
			char network[80]={0};
			if(server[0]!=0 && get_ini_str(server,"NETWORK",network,sizeof(network))){
				int ssl=0,connect_startup=0;
				char password[20]={0},port[20]={0};
				int index=0,item=0x80000000-1; //add to end of list
				LVITEM listItem;
				_snprintf(port,sizeof(port),"6667");
				get_ini_str(server,"PORT",port,sizeof(port));
				get_ini_value(server,"SSL",&ssl);
				get_ini_value(server,"CONNECT_STARTUP",&connect_startup);
				get_ini_str(server,"PASSWORD",password,sizeof(password));
				listItem.mask = LVIF_TEXT|LVIF_PARAM;
				listItem.pszText = network;
				listItem.iItem = item;
				listItem.iSubItem =0;
				listItem.lParam = count++;

				item=ListView_InsertItem(hlistview,&listItem);
				if(item>=0){
					int j;
					for(j=1;j<sizeof(server_columns)/sizeof(COLUMN);j++){
						char *text=0;
						if(strstri(server_columns[j].name,"SERVER")!=0)
							text=server;
						else if(stricmp(server_columns[j].name,"PORT")==0)
							text=port;
						else if(stricmp(server_columns[j].name,"SSL")==0)
							text=ssl!=0?"SSL":"";
						else if(strstri(server_columns[j].name,"STARTUP")!=0)
							text=connect_startup!=0?"connect on startup":"";
						else if(strstri(server_columns[j].name,"PASSWORD")!=0)
							text=password;
						if(text!=0)
							ListView_SetItemText(hlistview,item,j,text);
					}
				}
			}
		}
	}
	return TRUE;
}
int save_server_entry(HWND hwnd,int edit_entry,char *old_server)
{
	int i,empty,match,SSL=FALSE,connect_start=FALSE;
	char network[80]={0},server[80]={0},ports[80]={0},password[20]={0};
	GetDlgItemText(hwnd,IDC_NETWORK,network,sizeof(network));
	GetDlgItemText(hwnd,IDC_SERVER,server,sizeof(server));
	GetDlgItemText(hwnd,IDC_PORTS,ports,sizeof(ports));
	GetDlgItemText(hwnd,IDC_PASSWORD,password,sizeof(password));
	if(IsDlgButtonChecked(hwnd,IDC_SSL)==BST_CHECKED)
		SSL=TRUE;
	if(IsDlgButtonChecked(hwnd,IDC_CONNECT_STARTUP)==BST_CHECKED)
		connect_start=TRUE;
	trim_str(network);
	trim_str(server);
	trim_str(ports);
	trim_str(password);
	if(stricmp(server,"SETTINGS")==0 || stricmp(server,"SERVERS")==0 ||
		stricmp(server,"CHANNELS")==0){
		MessageBox(hwnd,"invalid server name\r\nnothing saved","error",MB_OK);
		return FALSE;
	}
	if(strlen(network)==0 || strlen(server)==0)
		return FALSE;
	if(strlen(ports)==0)
		_snprintf(ports,sizeof(ports),"6667");

	empty=-1;match=-1;
	for(i=MAX_SERVERS-1;i>=0;i--){
		char key[20]={0};
		char str[80]={0};
		_snprintf(key,sizeof(key),"ENTRY%i",i);
		get_ini_str("SERVERS",key,str,sizeof(str));
		if(str[0]==0)
			empty=i;
		if(edit_entry && stricmp(str,old_server)==0){
			delete_ini_section(old_server);
			match=i;
		}
		else if(str[0]!=0 && stricmp(str,server)==0)
			match=i;
	}
	if((match>=0) || (empty>=0)){
		char key[20]={0};
		int num=-1;
		if(match>=0)
			num=match;
		else if(empty>=0)
			num=empty;
		_snprintf(key,sizeof(key),"ENTRY%i",num);
		if(write_ini_str("SERVERS",key,server)){
			write_ini_str(server,"NETWORK",network);
			write_ini_str(server,"PORT",ports);
			write_ini_str(server,"SSL",SSL?"1":"0");
			write_ini_str(server,"CONNECT_STARTUP",connect_start?"1":"0");
			write_ini_str(server,"PASSWORD",password);
		}
	}
	return TRUE;
}
COLUMN channel_columns[]={
	{"channel",60},
	{"network",100},
	{"join_on_connect",100},
	{"password",30},
};
int load_ini_channels(HWND hlistview)
{
	int i,count=0;
	char key[255],chan_section[80]={0};
	ListView_DeleteAllItems(hlistview);
	for(i=0;i<MAX_CHANNELS;i++){
		_snprintf(key,sizeof(key),"ENTRY%i",i);
		chan_section[0]=0;
		if(get_ini_str("CHANNELS",key,chan_section,sizeof(chan_section))){
			char channel[80]={0};
			if(chan_section[0]!=0 && get_ini_str(chan_section,"NAME",channel,sizeof(channel))){
				char network[80]={0};
				int join_connect=0;
				char password[20]={0};
				int index=0,item=0x80000000-1; //add to end of list
				LVITEM listItem;
				get_ini_str(chan_section,"NETWORK",network,sizeof(network));
				get_ini_value(chan_section,"JOIN_CONNECT",&join_connect);
				get_ini_str(chan_section,"PASSWORD",password,sizeof(password));
				listItem.mask = LVIF_TEXT|LVIF_PARAM;
				listItem.pszText = channel;
				listItem.iItem = item;
				listItem.iSubItem =0;
				listItem.lParam = count++;

				item=ListView_InsertItem(hlistview,&listItem);
				if(item>=0){
					int j;
					for(j=1;j<sizeof(channel_columns)/sizeof(COLUMN);j++){
						char *text=0;
						if(strstri(channel_columns[j].name,"network")!=0)
							text=network;
						else if(strstri(channel_columns[j].name,"connect")!=0)
							text=join_connect!=0?"join on connect":"";
						else if(strstri(channel_columns[j].name,"password")!=0)
							text=password;
						if(text!=0)
							ListView_SetItemText(hlistview,item,j,text);
					}
				}
			}
		}
	}
	return TRUE;
}
int populate_channel_networks(HWND hwnd)
{
	int i;
	for(i=0;i<MAX_CHANNELS;i++){
		char server[80]={0};
		if(get_ini_entry("SERVERS",i,server,sizeof(server))){
			char network[80]={0};
			if(get_ini_str(server,"NETWORK",network,sizeof(network))){
				if(SendDlgItemMessage(hwnd,IDC_NETWORK,CB_FINDSTRINGEXACT,-1,network)==CB_ERR)
					SendDlgItemMessage(hwnd,IDC_NETWORK,CB_ADDSTRING,0,network);
			}
		}
	}
	return TRUE;
}
int populate_add_channel_win(HWND hlistview,HWND hwnd)
{
	char str[100]={0};
	LV_ITEM lvi={0};
	int item=get_focused_item(hlistview);
	lvi.mask=LVIF_TEXT;
	lvi.iItem=item;
	lvi.iSubItem=0;
	lvi.pszText=str;
	lvi.cchTextMax=sizeof(str);
	if(ListView_GetItem(hlistview,&lvi))
		SetDlgItemText(hwnd,IDC_CHANNEL,str);

	populate_channel_networks(hwnd);
	str[0]=0;
	lvi.iSubItem=1;
	if(ListView_GetItem(hlistview,&lvi)){
		SendDlgItemMessage(hwnd,IDC_NETWORK,CB_SELECTSTRING,-1,str);
	}
	str[0]=0;
	lvi.iSubItem=2;
	if(ListView_GetItem(hlistview,&lvi)){
		if(str[0]!=0)
			CheckDlgButton(hwnd,IDC_JOIN_CONNECT,TRUE);
	}
	str[0]=0;
	lvi.iSubItem=3;
	if(ListView_GetItem(hlistview,&lvi))
		SetDlgItemText(hwnd,IDC_PASSWORD,str);
	return TRUE;
}
int save_channel_entry(HWND hwnd,int edit_entry,char *old_channel)
{
	int i,empty,match,join_connect=FALSE;
	char network[80]={0},channel[80]={0},password[20]={0};
	char chan_section[80]={0},chan_section_old[80]={0};
	GetDlgItemText(hwnd,IDC_NETWORK,network,sizeof(network));
	GetDlgItemText(hwnd,IDC_CHANNEL,channel,sizeof(channel));
	GetDlgItemText(hwnd,IDC_PASSWORD,password,sizeof(password));
	if(IsDlgButtonChecked(hwnd,IDC_JOIN_CONNECT)==BST_CHECKED)
		join_connect=TRUE;
	trim_str(channel);
	if(strlen(channel)==0)
		return FALSE;
	fix_chan_name(channel,sizeof(channel));
	trim_str(network);
	if(strlen(network)==0)
		return FALSE;
	trim_str(password);
	if(network[0]==0)
		_snprintf(network,sizeof(network),"default");
/*
	if(stricmp(channel,"SETTINGS")==0 || stricmp(channel,"SERVERS")==0 ||
		stricmp(channel,"CHANNELS")==0){
		MessageBox(hwnd,"invalid channel name\r\nnothing saved","error",MB_OK);
		return FALSE;
	}
*/
	_snprintf(chan_section,sizeof(chan_section),"%s%s",network,channel);
	_snprintf(chan_section_old,sizeof(chan_section_old),"%s%s",network,old_channel);
	empty=-1;match=-1;
	for(i=MAX_CHANNELS-1;i>=0;i--){
		char key[20]={0};
		char str[80]={0};
		_snprintf(key,sizeof(key),"ENTRY%i",i);
		get_ini_str("CHANNELS",key,str,sizeof(str));
		if(str[0]==0)
			empty=i;
		if(edit_entry && stricmp(str,chan_section_old)==0){
			delete_ini_section(chan_section_old);
			match=i;
		}
		else if(str[0]!=0 && stricmp(str,chan_section)==0)
			match=i;
	}
	if((match>=0) || (empty>=0)){
		char key[20]={0};
		int num=-1;
		if(match>=0)
			num=match;
		else if(empty>=0)
			num=empty;
		_snprintf(key,sizeof(key),"ENTRY%i",num);
		if(write_ini_str("CHANNELS",key,chan_section)){
			write_ini_str(chan_section,"NAME",channel);
			write_ini_str(chan_section,"NETWORK",network);
			write_ini_str(chan_section,"JOIN_CONNECT",join_connect?"1":"0");
			write_ini_str(chan_section,"PASSWORD",password);
		}
	}
	return TRUE;
}
int load_ini_channel_listview(HWND hlistview)
{
	int i,width,height;
	char key[255];
	LV_COLUMN col;
	for(i=0;i<sizeof(channel_columns)/sizeof(COLUMN);i++){
		sprintf(key,"channel_width_%s",channel_columns[i].name);
		get_ini_value("SETTINGS",key,&channel_columns[i].width);
	}
	for(i=0;i<sizeof(channel_columns)/sizeof(COLUMN);i++){
		col.mask = LVCF_WIDTH|LVCF_TEXT;
		col.cx = channel_columns[i].width;
		col.pszText = channel_columns[i].name;
		ListView_InsertColumn(hlistview,i,&col);
	}
	if(get_ini_value("SETTINGS","channel_dlg_width",&width)&&
		get_ini_value("SETTINGS","channel_dlg_height",&height)){
		SetWindowPos(GetParent(hlistview),NULL,0,0,width,height,SWP_NOZORDER|SWP_NOMOVE);
	}
	return TRUE;
}
int save_ini_channel_listview(HWND hlistview)
{
	int i;
	char key[255];
	LV_COLUMN col;
	RECT rect;
	for(i=0;i<sizeof(channel_columns)/sizeof(COLUMN);i++){
		_snprintf(key,sizeof(key),"channel_width_%s",channel_columns[i].name);
		col.mask=LVCF_WIDTH;
		col.cx=channel_columns[i].width;
		ListView_GetColumn(hlistview,i,&col);
		write_ini_value("SETTINGS",key,col.cx);
	}
	if(GetWindowRect(GetParent(hlistview),&rect)){
		write_ini_value("SETTINGS","channel_dlg_width",rect.right-rect.left);
		write_ini_value("SETTINGS","channel_dlg_height",rect.bottom-rect.top);
	}
	return TRUE;
}
int do_channel_deletion(HWND hlistview)
{
	int count,item=get_focused_item(hlistview);
	delete_selected_channel(hlistview);
	load_ini_channels(hlistview);
	count=ListView_GetItemCount(hlistview);
	if(item>=count)
		item-=1;
	ListView_SetItemState(hlistview,item,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
	return TRUE;
}
int delete_selected_channel(HWND hlistview)
{
	int i,count,result=FALSE;
	count=ListView_GetItemCount(hlistview);
	for(i=0;i<count;i++){
		if(ListView_GetItemState(hlistview,i,LVIS_SELECTED)==LVIS_SELECTED){
			LV_ITEM lvi={0};
			char channel[80]={0},network[80]={0};
			lvi.mask=LVIF_TEXT;
			lvi.iItem=i;
			lvi.iSubItem=0;
			lvi.pszText=channel;
			lvi.cchTextMax=sizeof(channel);
			ListView_GetItem(hlistview,&lvi);
			lvi.iSubItem=1;
			lvi.pszText=network;
			lvi.cchTextMax=sizeof(network);
			ListView_GetItem(hlistview,&lvi);
			if(channel[0]!=0 && network[0]!=0){
				int j;
				char key[20]={0},chan_section[80]={0};
				_snprintf(chan_section,sizeof(chan_section),"%s%s",network,channel);
				delete_ini_section(chan_section);
				for(j=0;j<MAX_SERVERS;j++){
					char chan[80]={0};
					_snprintf(key,sizeof(key),"ENTRY%i",j);
					if(get_ini_str("CHANNELS",key,chan,sizeof(chan)-1))
						if(chan[0]!=0 && stricmp(chan,chan_section)==0){
							write_ini_str("CHANNELS",key,"");
							result=TRUE;
							break;
						}
				}
			}
		}
	}
	return result;
}
enum{
	CMD_JOIN_CONNECT_SET=1300,
	CMD_JOIN_CONNECT_UNSET,
	CMD_CONNECT_STARTUP_SET,
	CMD_CONNECT_STARTUP_UNSET,
};
int create_channel_dlg_menu(HWND hwnd)
{
	HWND chan_dlg_menu=0;
	if(chan_dlg_menu=CreatePopupMenu()){
		InsertMenu(chan_dlg_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_JOIN_CONNECT_SET,"set join on connect");
		InsertMenu(chan_dlg_menu,0xFFFFFFFF,MF_BYPOSITION|MF_SEPARATOR,0,0);
		InsertMenu(chan_dlg_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_JOIN_CONNECT_UNSET,"unset join on connect");
	}
	return chan_dlg_menu;
}
int create_server_dlg_menu(HWND hwnd)
{
	HWND server_dlg_menu=0;
	if(server_dlg_menu=CreatePopupMenu()){
		InsertMenu(server_dlg_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_CONNECT_STARTUP_SET,"set connect on startup");
		InsertMenu(server_dlg_menu,0xFFFFFFFF,MF_BYPOSITION|MF_SEPARATOR,0,0);
		InsertMenu(server_dlg_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_CONNECT_STARTUP_UNSET,"unset connect on startup");
	}
	return server_dlg_menu;
}

int channel_dlg_popup_cmd(int cmd,HWND hlistview)
{
	int i,count,result=FALSE;
	if(cmd!=CMD_JOIN_CONNECT_SET && cmd!=CMD_JOIN_CONNECT_UNSET)
		return result;
	count=ListView_GetItemCount(hlistview);
	for(i=0;i<count;i++){
		if(ListView_GetItemState(hlistview,i,LVIS_SELECTED)==LVIS_SELECTED){
			LV_ITEM lvi={0};
			char channel[80]={0},network[80]={0};
			lvi.mask=LVIF_TEXT;
			lvi.iItem=i;
			lvi.iSubItem=0;
			lvi.pszText=channel;
			lvi.cchTextMax=sizeof(channel);
			ListView_GetItem(hlistview,&lvi);
			lvi.iSubItem=1;
			lvi.pszText=network;
			lvi.cchTextMax=sizeof(network);
			ListView_GetItem(hlistview,&lvi);
			if(channel[0]!=0 && network[0]!=0){
				int join_connect=FALSE;
				char key[20]={0},chan_section[80]={0},name[80]={0};
				_snprintf(chan_section,sizeof(chan_section),"%s%s",network,channel);
				if(cmd==CMD_JOIN_CONNECT_SET)
					join_connect=TRUE;
				if(get_ini_str(chan_section,"NAME",name,sizeof(name))){
					if(stricmp(name,channel)==0)
						if(write_ini_str(chan_section,"JOIN_CONNECT",join_connect?"1":"0"))
							result=TRUE;
				}
			}
		}
	}
	if(result)
		load_ini_channels(hlistview);
	return result;
}
int server_dlg_popup_cmd(int cmd,HWND hlistview)
{
	int i,count,result=FALSE;
	if(cmd!=CMD_CONNECT_STARTUP_SET && cmd!=CMD_CONNECT_STARTUP_UNSET)
		return result;
	count=ListView_GetItemCount(hlistview);
	for(i=0;i<count;i++){
		if(ListView_GetItemState(hlistview,i,LVIS_SELECTED)==LVIS_SELECTED){
			LV_ITEM lvi={0};
			char server[80]={0};
			lvi.mask=LVIF_TEXT;
			lvi.iItem=i;
			lvi.iSubItem=1;
			lvi.pszText=server;
			lvi.cchTextMax=sizeof(server);
			ListView_GetItem(hlistview,&lvi);
			if(server[0]!=0){
				char network[80]={0};
				int connect_startup=FALSE;
				if(cmd==CMD_CONNECT_STARTUP_SET)
					connect_startup=TRUE;
				if(get_ini_str(server,"NETWORK",network,sizeof(network))){
					if(network[0]!=0)
						if(write_ini_str(server,"CONNECT_STARTUP",connect_startup?"1":"0"))
							result=TRUE;
				}
			}
		}
	}
	if(result)
		load_ini_servers(hlistview);
	return result;
}
int list_select_all(HWND hlistview)
{
	int i,count;
	count=ListView_GetItemCount(hlistview);
	for(i=0;i<count;i++){
		ListView_SetItemState(hlistview,i,LVIS_SELECTED,LVIS_SELECTED);
	}
	return count;
}
struct find_helper{
	int dir;
	int col;
	HWND hlistview;
};
int CALLBACK compare_func(LPARAM lparam1, LPARAM lparam2,struct find_helper *fh)
{
	LV_FINDINFO find1,find2;
	char str1[80]={0},str2[80]={0};
	int index1,index2;
	find1.flags=LVFI_PARAM;
	find1.lParam=lparam1;
	find2.flags=LVFI_PARAM;
	find2.lParam=lparam2;
	index1=ListView_FindItem(fh->hlistview,-1,&find1);
	index2=ListView_FindItem(fh->hlistview,-1,&find2);
	if(index1>=0 && index2>=0){
		int result;
		ListView_GetItemText(fh->hlistview,index1,fh->col,str1,sizeof(str1));
		ListView_GetItemText(fh->hlistview,index2,fh->col,str2,sizeof(str2));
		result=strcmp(str1,str2);
		if(fh->dir)
			result=-result;
		return result;
	}
	return 0;
}

int sort_listview(HWND hlistview,int dir,int column)
{
	struct find_helper fh;
	fh.hlistview=hlistview;
	fh.dir=dir;
	fh.col=column;
	ListView_SortItems(hlistview,compare_func,&fh);
	return TRUE;
}
int create_listview(HWND hwnd,HWND hinstance)
{
	HWND hlistview;
    RECT rclient;
    GetClientRect(hwnd,&rclient); 
    hlistview = CreateWindow(WC_LISTVIEW, 
                                     "",
                                     WS_TABSTOP|WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_SHOWSELALWAYS|LVS_SORTASCENDING,
                                     2, 2,
                                     rclient.right-85,
                                     rclient.bottom-4,
                                     hwnd,
                                     IDC_LISTVIEW,
                                     (HINSTANCE)hinstance,
                                     NULL);
	ListView_SetExtendedListViewStyle(hlistview,
		ListView_GetExtendedListViewStyle(hlistview)|LVS_EX_FULLROWSELECT);
	return hlistview;
}

int thread_status(int set,int busy)
{
	static int thread_busy=FALSE;
	if(set)
		thread_busy=busy;
	return thread_busy;
}
int auto_connect(HWND hmdi)
{
	extern HWND ghmainframe;
	int i,count=0;
	thread_status(TRUE,TRUE);
	for(i=0;i<MAX_SERVERS;i++){
		char server[80]={0};
		if(get_ini_entry("SERVERS",i,server,sizeof(server))){
			int port=6667,connect=0,ssl=0;
			char network[80]={0},password[20]={0};
			get_ini_str(server,"NETWORK",network,sizeof(network));
			get_ini_str(server,"PASSWORD",password,sizeof(password));
			get_ini_value(server,"CONNECT_STARTUP",&connect);
			get_ini_value(server,"SSL",&ssl);
			get_ini_value(server,"PORT",&port);
			if(network[0]!=0 && connect!=0){
				connect_server(hmdi,network,server,port,ssl,password);
				count++;
			}
		}
	}
	if(count==0)
		PostMessage(ghmainframe,WM_COMMAND,ID_SERVERS,0);
	thread_status(TRUE,FALSE);
	_endthread();
	return TRUE;
}