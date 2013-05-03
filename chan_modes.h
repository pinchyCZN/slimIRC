HWND chan_mode_hwnd=0,chan_mode_parent=0;
char chan_mode_data[128]={0};
char chan_mode_topic[512]={0};
char chan_mode_ban_entry[80]={0};
int chan_mode_have_topic=FALSE;
struct IDC_TO_MODE{
	int idc;
	char val;
};
BOOL CALLBACK chan_modes(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	extern short chan_mode_list[];
	static HWND grippy=0;
	HWND htemp;
	IRC_WINDOW *win;
	static int have_all_info=FALSE;
	struct IDC_TO_MODE idc_modes[]={
		{IDC_ONLYOPS,'t'},
		{IDC_NOEXTERNAL,'n'},
		{IDC_INVITEONLY,'i'},
		{IDC_MODERATED,'m'},
		{IDC_REQUIRESKEY,'k'},
		{IDC_LIMITTO,'l'},
		{IDC_PRIVATE,'p'},
		{IDC_SECRET,'s'},
	};
//	print_msg(msg,lparam,wparam,hwnd);
	switch(msg){
	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd,IDC_TOPIC,EM_LIMITTEXT,512,0);
		SendDlgItemMessage(hwnd,IDC_KEY,EM_LIMITTEXT,20,0);
		SendDlgItemMessage(hwnd,IDC_USERS,EM_LIMITTEXT,4,0);
		win=find_window_by_hwnd(chan_mode_parent);
		if(win!=0 && win->session!=0 && irc_is_connected(win->session)){
			char str[256];
			_snprintf(str,sizeof(str),"Channel modes for %s %s",win->network,win->channel);
			SetWindowText(hwnd,str);
			SetDlgItemText(hwnd,IDC_TOPIC,"retrieving topic...");
			irc_send_raw(win->session,"MODE %s",win->channel);
		}
		grippy=create_grippy(hwnd);
		chan_mode_hwnd=hwnd;
		have_all_info=FALSE;
		chan_mode_have_topic=FALSE;
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam)){
		case IDOK:
			htemp=GetFocus();
			if(htemp==GetDlgItem(hwnd,IDOK) || htemp==GetDlgItem(hwnd,IDC_TOPIC)){
				char str[256]={0},pass[20]={0};
				int i,ret,key=FALSE,limit=FALSE,users=0;
				for(i=0;i<sizeof(idc_modes)/sizeof(struct IDC_TO_MODE);i++){
					if(idc_modes[i].idc==IDC_REQUIRESKEY)
						continue;
					else if(idc_modes[i].idc==IDC_LIMITTO)
						continue;
					ret=IsDlgButtonChecked(hwnd,idc_modes[i].idc);
					if(ret==BST_CHECKED)
						_snprintf(str,sizeof(str),"%s+%c",str,idc_modes[i].val);
					else if(ret==BST_UNCHECKED)
						_snprintf(str,sizeof(str),"%s-%c",str,idc_modes[i].val);
				}
				win=find_window_by_hwnd(chan_mode_parent);
				if(win!=0 && win->session!=0 && irc_is_connected(win->session)){
					if(have_all_info){
						if(key)
							_snprintf(str,sizeof(str),"%s %s",pass);
						if(limit)
							_snprintf(str,sizeof(str),"%s :%i",users);
						irc_send_raw(win->session,"MODE %s %s",win->channel,str);
					}
					str[0]=0;
					GetDlgItemText(hwnd,IDC_TOPIC,str,sizeof(str));
					if(chan_mode_have_topic && strcmp(str,chan_mode_topic)!=0)
						irc_send_raw(win->session,"TOPIC %s :%s",win->channel,str);
				}
				goto exit;
			}
			else
				PostMessage(hwnd,WM_NEXTDLGCTL,0,0);
			break;
		case IDCANCEL:
			goto exit;
			break;
		}
		break;
	case WM_VKEYTOITEM:
		{
			int key=LOWORD(wparam);
			int item=HIWORD(wparam);
			if(key==VK_DELETE){
				char str[80]={0};
				if(SendMessage(lparam,LB_GETTEXTLEN,item,0)<sizeof(str)){
					if(SendMessage(lparam,LB_GETTEXT,item,str)!=LB_ERR){
						win=find_window_by_hwnd(chan_mode_parent);
						if(win!=0 && win->session!=0 && irc_is_connected(win->session))
							irc_send_raw(win->session,"MODE %s -b %s",win->channel,str);
					}
				}
				SendMessage(lparam,LB_DELETESTRING,item,0);
				return -2;
			}
		}
		break;
	case WM_USER:
		switch(wparam){
		case MSG_BANLIST:
			SendDlgItemMessage(hwnd,IDC_BANLIST,LB_ADDSTRING,0,chan_mode_ban_entry);
			break;
		case MSG_TOPIC:
			chan_mode_have_topic=TRUE;
			SetDlgItemText(hwnd,IDC_TOPIC,chan_mode_topic);
			win=find_window_by_hwnd(chan_mode_parent);
			if(win!=0 && win->session!=0 && irc_is_connected(win->session))
				irc_send_raw(win->session,"MODE %s +b",win->channel); //get the ban list
			break;
		case MSG_MODE:
			{
				int i,set=BST_UNCHECKED,key=FALSE,limit=FALSE,part=0,index=0,len=strlen(chan_mode_data);
				char param1[40]={0},param2[40]={0};
				for(i=0;i<len;i++){
					if(chan_mode_data[i]=='|'){
						if(part==1)
							param1[index++]=0;
						else if(part==2)
							param2[index++]=0;
						index=0;
						part++;
					}
					else if(part==0){
						int j;
						if(chan_mode_data[i]=='+')
							set=BST_CHECKED;
						else if(chan_mode_data[i]=='-')
							set=BST_UNCHECKED;
						for(j=0;j<sizeof(idc_modes)/sizeof(struct IDC_TO_MODE);j++){
							if(idc_modes[j].val==tolower(chan_mode_data[i])){
								CheckDlgButton(hwnd,idc_modes[j].idc,set);
								if(idc_modes[j].idc==IDC_REQUIRESKEY)
									key=TRUE;
								else if(idc_modes[j].idc==IDC_LIMITTO)
									limit=TRUE;
								break;
							}
						}
					}
					else if(part==1){
						if(index<(sizeof(param1)-1))
							param1[index++]=chan_mode_data[i];
					}
					else if(part==2){
						if(index<(sizeof(param2)-1))
							param2[index++]=chan_mode_data[i];
					}
				}
				param1[sizeof(param1)-1]=0;
				param2[sizeof(param2)-1]=0;
				trim_str(param1);
				trim_str(param2);
				if(limit && key){
					SetDlgItemText(hwnd,IDC_KEY,param2);
					SetDlgItemText(hwnd,IDC_USERS,param1);
				}
				else if(limit)
					SetDlgItemText(hwnd,IDC_USERS,param1);
				else if(key)
					SetDlgItemText(hwnd,IDC_KEY,param1);
				win=find_window_by_hwnd(chan_mode_parent);
				if(win!=0 && win->session!=0 && irc_is_connected(win->session))
					irc_send_raw(win->session,"TOPIC %s",win->channel);
				have_all_info=TRUE;
			}
			break;
		}
	case WM_SIZE:
		grippy_move(hwnd,grippy);
		reposition_controls(hwnd,chan_mode_list);
		break;
	case WM_CLOSE:
	case WM_DESTROY:
exit:
		chan_mode_hwnd=0;
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}
int update_chan_mode_dlg(int event,const char *origin,const char **params,int count)
{
	
	switch(event){
	case 367: //ban entry u00|#1|*!*@127.0.0.1|pinchy|123456778
		if(count>=3 && chan_mode_hwnd!=0){
			_snprintf(chan_mode_ban_entry,sizeof(chan_mode_ban_entry),"%s",params[2]);
			SendMessage(chan_mode_hwnd,WM_USER,MSG_BANLIST,0);
		}
		break;
	case 324: //mode pinchy|#thistest|+spmtinlk|1337|thisisakey
		if(count>=3 && chan_mode_hwnd!=0){
			_snprintf(chan_mode_data,sizeof(chan_mode_data),"%s|%s|%s|",params[2],
				count>3?params[3]:"",
				count>4?params[4]:"");
			SendMessage(chan_mode_hwnd,WM_USER,MSG_MODE,0);
		}
		break;
	case 331: //no topic set
		chan_mode_topic[0]=0;
		SendMessage(chan_mode_hwnd,WM_USER,MSG_TOPIC,0);
		break;
	case 332: //topic pinchy|#1|blah
		if(count>=3 && chan_mode_hwnd!=0){
			_snprintf(chan_mode_topic,sizeof(chan_mode_topic),"%s",params[2]);
			chan_mode_topic[sizeof(chan_mode_topic)-1]=0;
			SendMessage(chan_mode_hwnd,WM_USER,MSG_TOPIC,0);
		}
		break;
	}
	return TRUE;
}