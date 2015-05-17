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
	{"network",0},
	{"server",0},
	{"port",0},
	{"connect_on_startup",0},
	{"ssl",0},
	{"password",0},
	{"user",0},
	{"nick",0},
};
int get_str_width(HWND hwnd,char *str,int wide_char)
{
	if(hwnd!=0 && str!=0){
		SIZE size={0};
		HDC hdc;
		hdc=GetDC(hwnd);
		if(hdc!=0){
			HFONT hfont;
			hfont=SendMessage(hwnd,WM_GETFONT,0,0);
			if(hfont!=0){
				HGDIOBJ hold=0;
				hold=SelectObject(hdc,hfont);
				if(wide_char)
					GetTextExtentPoint32W(hdc,str,wcslen(str),&size);
				else
					GetTextExtentPoint32(hdc,str,strlen(str),&size);
				if(hold!=0)
					SelectObject(hdc,hold);
			}
			else{
				if(wide_char)
					GetTextExtentPoint32W(hdc,str,wcslen(str),&size);
				else
					GetTextExtentPoint32(hdc,str,strlen(str),&size);
			}
			ReleaseDC(hwnd,hdc);
			return size.cx;
		}
	}
	return 0;
}
int get_ini_entry(char *section,int num,char *str,int len)
{
	char key[20];
	_snprintf(key,sizeof(key),"ENTRY%i",num);
	return get_ini_str(section,key,str,len);
}
int set_ini_entry(char *section,int num,char *str)
{
	char key[20];
	_snprintf(key,sizeof(key),"ENTRY%i",num);
	return write_ini_str(section,key,str);
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
int trim_str(unsigned char *str)
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
int delete_selected_server(HWND hlistview)
{
	int i,count,result=FALSE;
	count=ListView_GetItemCount(hlistview);
	for(i=0;i<count;i++){
		if(ListView_GetItemState(hlistview,i,LVIS_SELECTED)==LVIS_SELECTED){
			LV_ITEM lvi={0};
			char network[80]={0};
			char server[80]={0};
			lvi.mask=LVIF_TEXT;
			lvi.iItem=i;
			lvi.iSubItem=0;
			lvi.pszText=network;
			lvi.cchTextMax=sizeof(network);
			ListView_GetItem(hlistview,&lvi);
			lvi.iSubItem=1;
			lvi.pszText=server;
			lvi.cchTextMax=sizeof(server);
			ListView_GetItem(hlistview,&lvi);
			if(network[0]!=0 && server[0]!=0){
				int j;
				char network_server[160]={0};
				_snprintf(network_server,sizeof(network_server),"%s|%s",network,server);
				delete_ini_section(network_server);
				for(j=0;j<MAX_SERVERS;j++){
					char s[160]={0};
					if(get_ini_entry("SERVERS",j,s,sizeof(s)-1))
						if(s[0]!=0 && stricmp(s,network_server)==0){
							set_ini_entry("SERVERS",j,"");
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
	int i;
	struct LV_DATA{
		int idc;
		int subitem;
		int is_check;
	};
	struct LV_DATA lv_data_list[]={
		{IDC_NETWORK,0,0},
		{IDC_SERVER,1,0},
		{IDC_PORTS,2,0},
		{IDC_CONNECT_STARTUP,3,1},
		{IDC_SSL,4,1},
		{IDC_PASSWORD,5,0},
		{IDC_USER,6,0},
		{IDC_NICK,7,0}
	};
	int item=get_focused_item(hlistview);
	if(item<0)
		return FALSE;
	for(i=0;i<sizeof(lv_data_list)/sizeof(struct LV_DATA);i++){
		char str[100]={0};
		LV_ITEM lvi={0};
		lvi.mask=LVIF_TEXT;
		lvi.iItem=item;
		lvi.iSubItem=lv_data_list[i].subitem;
		lvi.pszText=str;
		lvi.cchTextMax=sizeof(str);
		if(ListView_GetItem(hlistview,&lvi)){
			if(str[0]!=0){
				int idc=lv_data_list[i].idc;
				if(lv_data_list[i].is_check)
					CheckDlgButton(hwnd,idc,TRUE);
				else
					SetDlgItemText(hwnd,idc,str);
			}
		}

	}
	return TRUE;
}

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
		if(server_columns[i].width==0){
			server_columns[i].width=get_str_width(hlistview,server_columns[i].name,FALSE);
			server_columns[i].width+=14;
		}
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
	int i;
	ListView_DeleteAllItems(hlistview);
	for(i=0;i<MAX_SERVERS;i++){
		char network_server[160]={0};
		if(get_ini_entry("SERVERS",i,network_server,sizeof(network_server))){
			int j,count=0;
			char str[160]={0};
			struct LV_INI_DATA{
				char *col_name;
				char *ini_key;
				int is_required;
				int is_bool;
				char *bool_text;
				char *def_text;
			};
			struct LV_INI_DATA lv_ini_list[]={
				{server_columns[0].name,"NETWORK",1,0,0,0},
				{server_columns[1].name,"SERVER",1,0,0,0},
				{server_columns[2].name,"PORT",0,0,0,"6667"},
				{server_columns[3].name,"CONNECT_STARTUP",0,1,"connect on startup",0},
				{server_columns[4].name,"SSL",0,1,"SSL",0},
				{server_columns[5].name,"PASSWORD",0,0,0,0},
				{server_columns[6].name,"USER",0,0,0,0},
				{server_columns[7].name,"NICK",0,0,0,0},
			};
			int have_required=TRUE;
			for(j=0;j<sizeof(lv_ini_list)/sizeof(struct LV_INI_DATA);j++){
				if(lv_ini_list[j].is_required){
					str[0]=0;
					get_ini_str(network_server,lv_ini_list[j].ini_key,str,sizeof(str));
					if(str[0]==0){
						have_required=FALSE;
						break;
					}
				}
			}
			if(have_required){
				int item=0x80000000-1; //add to end of list
				LVITEM listItem;
				for(j=0;j<sizeof(lv_ini_list)/sizeof(struct LV_INI_DATA);j++){
					str[0]=0;
					if(lv_ini_list[j].is_bool){
						int val=0;
						get_ini_value(network_server,lv_ini_list[j].ini_key,&val);
						if(val && lv_ini_list[j].bool_text){
							_snprintf(str,sizeof(str),"%s",lv_ini_list[j].bool_text);
							str[sizeof(str)-1]=0;
						}
					}else{
						get_ini_str(network_server,lv_ini_list[j].ini_key,str,sizeof(str));
						if(str[0]==0 && lv_ini_list[j].def_text){
							_snprintf(str,sizeof(str),"%s",lv_ini_list[j].def_text);
							str[sizeof(str)-1]=0;
						}
					}
					listItem.mask = LVIF_TEXT|LVIF_PARAM;
					listItem.pszText = str;
					listItem.iItem = item;
					listItem.iSubItem = j;
					listItem.lParam = count++;
					if(j==0){
						item=ListView_InsertItem(hlistview,&listItem);
					}
					if(j>0 && item>=0){
						ListView_SetItemText(hlistview,item,j,str);
					}
				}
			}
		}
	}
	return TRUE;
}
int save_server_entry(HWND hwnd,int edit_entry,char *old_server_entry)
{
	char network[80]={0},server[80]={0},network_server[160]={0};
	struct SERVER_ENTRY{
		int idc;
		char *ini_key;
		char *invalid_names[3];
		int is_check;
		int needs_trim;
		int is_required;
		char *def_text;
	};
	struct SERVER_ENTRY slist[]={
		{IDC_NETWORK,"NETWORK",{0},0,1,1,0},
		{IDC_SERVER,"SERVER",{"SETTINGS","SERVERS","CHANNELS"},0,1,1,0},
		{IDC_PORTS,"PORT",{0},0,1,0,"6667"},
		{IDC_PASSWORD,"PASSWORD",{0},0,1,0,0},
		{IDC_USER,"USER",{0},0,1,0,0},
		{IDC_NICK,"NICK",{0},0,1,0,0},
		{IDC_SSL,"SSL",{0},1,0,0,0},
		{IDC_CONNECT_STARTUP,"CONNECT_STARTUP",{0},1,0,0,0},
	};
	int i;
	for(i=0;i<sizeof(slist)/sizeof(struct SERVER_ENTRY);i++){
		if(slist[i].is_required){
			int j;
			char str[80]={0};
			GetDlgItemText(hwnd,slist[i].idc,str,sizeof(str));
			if(slist[i].needs_trim)
				trim_str(str);
			if(str[0]==0){
				return FALSE;
			}
			if(slist[i].invalid_names){
				for(j=0;j<sizeof(slist[i].invalid_names)/sizeof(char *);j++){
					if(0!=slist[i].invalid_names[j]){
						if(0==stricmp(str,slist[i].invalid_names[j])){
							_snprintf(str,sizeof(str),"invalid named entry:%s\r\nnothing saved",
								slist[i].invalid_names[j]);
							str[sizeof(str)-1]=0;
							MessageBox(hwnd,str,"error",MB_OK);
							return FALSE;
						}
					}
				}
			}
			if(slist[i].idc==IDC_NETWORK){
				strncpy(network,str,sizeof(network));
				network[sizeof(network)-1]=0;
			}
			else if(slist[i].idc==IDC_SERVER){
				strncpy(server,str,sizeof(server));
				server[sizeof(server)-1]=0;
			}
		}

	}
	if(network[0]!=0 && server[0]!=0)
	{
		int empty=-1,match=-1;
		_snprintf(network_server,sizeof(network_server),"%s|%s",network,server);
		network_server[sizeof(network_server)-1]=0;
		for(i=MAX_SERVERS-1;i>=0;i--){
			char str[160]={0};
			get_ini_entry("SERVERS",i,str,sizeof(str));
			if(str[0]==0)
				empty=i;
			if(edit_entry && old_server_entry[0]!=0 && stricmp(str,old_server_entry)==0){
				delete_ini_section(old_server_entry);
				match=i;
			}
			else if(str[0]!=0 && stricmp(str,network_server)==0)
				match=i;
		}
		if((match>=0) || (empty>=0)){
			int num=-1;
			if(match>=0)
				num=match;
			else if(empty>=0)
				num=empty;
			set_ini_entry("SERVERS",num,network_server);
			for(i=0;i<sizeof(slist)/sizeof(struct SERVER_ENTRY);i++){
				char str[160]={0};
				if(slist[i].is_check){
					str[0]='0';str[1]=0;
					if(IsDlgButtonChecked(hwnd,slist[i].idc)==BST_CHECKED)
						str[0]='1';
				}
				else
					GetDlgItemText(hwnd,slist[i].idc,str,sizeof(str));
				if(slist[i].needs_trim)
					trim_str(str);
				if(str[0]==0 && slist[i].def_text){
					strncpy(str,slist[i].def_text,sizeof(str));
					str[sizeof(str)-1]=0;
				}
				write_ini_str(network_server,slist[i].ini_key,str);
				
			}
		}
	}
	return TRUE;
}
COLUMN channel_columns[]={
	{"channel",0},
	{"network",0},
	{"join_on_connect",0},
	{"password",0},
};
int load_ini_channels(HWND hlistview)
{
	int i,count=0;
	ListView_DeleteAllItems(hlistview);
	for(i=0;i<MAX_CHANNELS;i++){
		char chan_section[80]={0};
		if(get_ini_entry("CHANNELS",i,chan_section,sizeof(chan_section))){
			char channel[80]={0};
			if(chan_section[0]!=0 && get_ini_str(chan_section,"NAME",channel,sizeof(channel))){
				char network[80]={0};
				int join_connect=0;
				char password[20]={0};
				int item=0x80000000-1; //add to end of list
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
		char network_server[160]={0};
		if(get_ini_entry("SERVERS",i,network_server,sizeof(network_server))){
			char network[80]={0};
			if(get_ini_str(network_server,"NETWORK",network,sizeof(network))){
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
	if(item<0)
		return FALSE;
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

int test_populate_ini()
{
	int z;
	int i,empty,match,join_connect=FALSE;
	int edit_entry=FALSE;
	for(z=0;z<40;z++){
		char network[80]={0},channel[80]={0},password[20]={0};
		char chan_section[80]={0},chan_section_old[80]={0};
		char *old_channel="";
		sprintf(network,"%cdeasddsa",(rand()%26)+'a');
		sprintf(channel,"channel%i",z);
		sprintf(password,"%i",rand());

		_snprintf(chan_section,sizeof(chan_section),"%s%s",network,channel);
		_snprintf(chan_section_old,sizeof(chan_section_old),"%s%s",network,old_channel);
		empty=-1;match=-1;
		for(i=MAX_CHANNELS-1;i>=0;i--){
			char str[80]={0};
			get_ini_entry("CHANNELS",i,str,sizeof(str));
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
			int num=-1;
			if(match>=0)
				num=match;
			else if(empty>=0)
				num=empty;
			if(set_ini_entry("CHANNELS",num,chan_section)){
				write_ini_str(chan_section,"NAME",channel);
				write_ini_str(chan_section,"NETWORK",network);
				write_ini_str(chan_section,"JOIN_CONNECT",join_connect?"1":"0");
				write_ini_str(chan_section,"PASSWORD",password);
			}
		}
	}

	{
		for(z=0;z<40;z++){
			int i,empty,match,SSL=FALSE,connect_start=FALSE;
			char network[80]={0},server[80]={0},network_server[160]={0},ports[40]={0},password[40]={0};
			char *old_server_entry="";
			sprintf(network,"%c%08i",(rand()%26)+'a',z);
			sprintf(server,"server%i",z);
			sprintf(password,"%i",rand());			
			trim_str(network);
			trim_str(server);
			trim_str(ports);
			trim_str(password);
			if(stricmp(server,"SETTINGS")==0 || stricmp(server,"SERVERS")==0 ||
				stricmp(server,"CHANNELS")==0){
				MessageBox(0,"invalid server name\r\nnothing saved","error",MB_OK);
				return FALSE;
			}
			if(strlen(network)==0 || strlen(server)==0)
				return FALSE;
			_snprintf(network_server,sizeof(network_server),"%s|%s",network,server);
			if(strlen(ports)==0)
				_snprintf(ports,sizeof(ports),"6667");

			empty=-1;match=-1;
			for(i=MAX_SERVERS-1;i>=0;i--){
				char str[160]={0};
				get_ini_entry("SERVERS",i,str,sizeof(str));
				if(str[0]==0)
					empty=i;
				if(edit_entry && old_server_entry[0]!=0 && stricmp(str,old_server_entry)==0){
					delete_ini_section(old_server_entry);
					match=i;
				}
				else if(str[0]!=0 && stricmp(str,network_server)==0)
					match=i;
			}
			if((match>=0) || (empty>=0)){
				int num=-1;
				if(match>=0)
					num=match;
				else if(empty>=0)
					num=empty;
				if(set_ini_entry("SERVERS",num,network_server)){
					write_ini_str(network_server,"NETWORK",network);
					write_ini_str(network_server,"SERVER",server);
					write_ini_str(network_server,"PORT",ports);
					write_ini_str(network_server,"SSL",SSL?"1":"0");
					write_ini_str(network_server,"CONNECT_STARTUP",connect_start?"1":"0");
					write_ini_str(network_server,"PASSWORD",password);
				}
			}
		}
	}
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
		char str[80]={0};
		get_ini_entry("CHANNELS",i,str,sizeof(str));
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
		int num=-1;
		if(match>=0)
			num=match;
		else if(empty>=0)
			num=empty;
		if(set_ini_entry("CHANNELS",num,chan_section)){
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
		if(channel_columns[i].width==0){
			channel_columns[i].width=get_str_width(hlistview,channel_columns[i].name,FALSE);
			channel_columns[i].width+=14;
		}
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
				char chan_section[80]={0};
				_snprintf(chan_section,sizeof(chan_section),"%s%s",network,channel);
				delete_ini_section(chan_section);
				for(j=0;j<MAX_SERVERS;j++){
					char chan[80]={0};
					if(get_ini_entry("CHANNELS",j,chan,sizeof(chan)-1))
						if(chan[0]!=0 && stricmp(chan,chan_section)==0){
							set_ini_entry("CHANNELS",j,"");
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
	HMENU chan_dlg_menu=0;
	if(chan_dlg_menu=CreatePopupMenu()){
		InsertMenu(chan_dlg_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_JOIN_CONNECT_SET,"set join on connect");
		InsertMenu(chan_dlg_menu,0xFFFFFFFF,MF_BYPOSITION|MF_SEPARATOR,0,0);
		InsertMenu(chan_dlg_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_JOIN_CONNECT_UNSET,"unset join on connect");
	}
	return chan_dlg_menu;
}
int create_server_dlg_menu(HWND hwnd)
{
	HMENU server_dlg_menu=0;
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
				char chan_section[80]={0},name[80]={0};
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
			char network[80]={0};
			char server[80]={0};
			lvi.mask=LVIF_TEXT;
			lvi.iItem=i;
			lvi.iSubItem=0;
			lvi.pszText=network;
			lvi.cchTextMax=sizeof(network);
			ListView_GetItem(hlistview,&lvi);
			lvi.iSubItem=1;
			lvi.pszText=server;
			lvi.cchTextMax=sizeof(server);
			ListView_GetItem(hlistview,&lvi);
			if(network[0]!=0 && server[0]!=0){
				char network_server[160]={0};
				char tmp[80]={0};
				int connect_startup=FALSE;
				if(cmd==CMD_CONNECT_STARTUP_SET)
					connect_startup=TRUE;
				_snprintf(network_server,sizeof(network_server),"%s|%s",network,server);
				get_ini_str(network_server,"NETWORK",tmp,sizeof(tmp));
				if(tmp[0]!=0 && strnicmp(network_server,tmp,strlen(tmp)-1)==0){
					if(write_ini_str(network_server,"CONNECT_STARTUP",connect_startup?"1":"0"))
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
		result=_stricmp(str1,str2);
		if(fh->dir)
			result=-result;
		return result;
	}
	return 0;
}
int lv_get_column_count(HWND hlistview)
{
	HWND header;
	int count=0;
	header=SendMessage(hlistview,LVM_GETHEADER,0,0);
	if(header!=0){
		count=SendMessage(header,HDM_GETITEMCOUNT,0,0);
		if(count<0)
			count=0;
	}
	return count;
}
int update_sort_col(HWND hlistview,int dir,int column)
{
	int i,count;
	count=lv_get_column_count(hlistview);
	for(i=0;i<count;i++){
		WCHAR str[80]={0};
		LV_COLUMN col={0};
		int j,len;
		col.mask=LVCF_TEXT|LVCF_WIDTH;
		col.pszText=str;
		col.cchTextMax=sizeof(str)/sizeof(WCHAR);
		SendMessageW(hlistview,LVM_GETCOLUMNW,i,&col);
		len=wcslen(col.pszText);
		{
			int index=0;
			for(j=0;j<len;j++){
				WCHAR c=str[j];
				if(c==0x25BC || c==0x25B2)
					c=c;
				else
					str[index++]=str[j];
			}
			str[index]=0;
		}
		if(column==i){
			int width;
			if( len<=((sizeof(str)/sizeof(WCHAR))-2) ){
				str[len+1]=0;
				for(j=len;j>0;j--){
					str[j]=str[j-1];
				}
				str[0]=dir?0x25BC:0x25B2;
			}
			width=get_str_width(hlistview,str,TRUE)+14;
			if(width>col.cx)
				col.cx=width;
		}
		col.pszText=str;
		col.cchTextMax=sizeof(str)/sizeof(WCHAR);
		SendMessageW(hlistview,LVM_SETCOLUMNW,i,&col);
	}
	return TRUE;
}
int sort_listview(HWND hlistview,int dir,int column)
{
	struct find_helper fh;
	fh.hlistview=hlistview;
	fh.dir=dir;
	fh.col=column;
	ListView_SortItems(hlistview,compare_func,&fh);
	update_sort_col(hlistview,dir,column);
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
		char entry[160]={0};
		if(get_ini_entry("SERVERS",i,entry,sizeof(entry))){
			int port=6667,connect=0,ssl=0;
			char server[80]={0},network[80]={0},password[40]={0};
			char user[40]={0},nick[20]={0};
			get_ini_str(entry,"NETWORK",network,sizeof(network));
			get_ini_str(entry,"SERVER",server,sizeof(server));
			get_ini_str(entry,"PASSWORD",password,sizeof(password));
			get_ini_str(entry,"USER",user,sizeof(user));
			get_ini_str(entry,"NICK",nick,sizeof(nick));
			get_ini_value(entry,"CONNECT_STARTUP",&connect);
			get_ini_value(entry,"SSL",&ssl);
			get_ini_value(entry,"PORT",&port);
			if(network[0]!=0 && connect!=0){
				connect_server(hmdi,network,server,port,ssl,password,user,nick);
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