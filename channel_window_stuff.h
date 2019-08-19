int find_channel_window(void *session,const char *channel)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].session==session)
			if(irc_windows[i].type==CHANNEL_WINDOW)
				if(_stricmp(irc_windows[i].channel,channel)==0)
					return &irc_windows[i];
	}
	return 0;
}
int find_privmsg_window(void *session,const char *nick)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].session==session)
			if(irc_windows[i].type==PRIVMSG_WINDOW)
				if(_stricmp(irc_windows[i].nick,nick)==0)
					return &irc_windows[i];
	}
	return 0;
}
int update_chat_sessions(void *session)
{
	int i;
	IRC_WINDOW *win;
	win=find_window_by_session(session);
	if(win==0)
		return FALSE;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].type==PRIVMSG_WINDOW && irc_windows[i].session==0)
			if(_stricmp(irc_windows[i].network,win->network)==0)
				irc_windows[i].session=session;
		if(irc_windows[i].type==CHANNEL_WINDOW && irc_windows[i].session==0)
			if(_stricmp(irc_windows[i].network,win->network)==0)
				irc_cmd_join(session,irc_windows[i].channel,irc_windows[i].password);
	}
	return TRUE;
}
int trim_return(char *str)
{
	int i,len;
	len=strlen(str);
	for(i=len-1;i>0;i--){
		if((unsigned char)str[i]>=' ')
			break;
		else
			str[i]=0;
	}
	return TRUE;
}
int valid_text(char *str)
{
	int i,len;
	len=strlen(str);
	for(i=0;i<len;i++){
		if((unsigned char)str[i]>=' ')
			return TRUE;
	}
	return FALSE;
}
struct POST_MSG{
	IRC_WINDOW *win;
	char *msg;
};
static int post_thread_busy=0;
#ifdef _DEBUG
static int post_speed=100;
#else
static int post_speed=1000;
#endif
static int post_stop=0;
unsigned __stdcall post_msg_thread(void *param)
{
	struct POST_MSG *pmsg=(struct POST_MSG *)param;
	IRC_WINDOW *win;
	DWORD tick,delta;
	int timeout=0;
	char channel[40];
	int lines,count;
	unsigned int i,len;
	char *str;
	char msg[512];

	post_stop=0;
	if(0==pmsg)
		goto EXIT_PMSG;
	if(0==pmsg->msg)
		goto EXIT_PMSG;
	if(0==pmsg->win)
		goto EXIT_PMSG;

	win=pmsg->win;
	str=pmsg->msg;
	len=strlen(str);

	if(win->type==PRIVMSG_WINDOW)
		extract_nick(win->channel,channel,sizeof(channel));
	else
		strncpy(channel,win->channel,sizeof(channel));


	lines=count=0;
	for(i=0;i<len;i++){
		char a=str[i];
		int is_cr;
		is_cr='\n'==a;
		if('\r'==a)
			continue;
		if(!is_cr){
			if(count<sizeof(msg))
				msg[count++]=a;
		}
		if(i==(len-1) || is_cr || count>=sizeof(msg)){
			char mdi_msg[512+20];
			if(count<sizeof(msg))
				msg[count]=0;
			msg[sizeof(msg)-1]=0;
			if(valid_text(msg)){
				tick=GetTickCount();
				while(1){
					if(post_stop)
						break;
					if(0==win->hwnd || 0==win->session)
						break;
					if(0==irc_cmd_msg(win->session,channel,msg)){
						_snprintf(mdi_msg,sizeof(mdi_msg),"<%s> %s",win->nick,msg);
						mdi_msg[sizeof(mdi_msg)-1]=0;
						add_line_mdi(win,mdi_msg);
						Sleep(post_speed);
						break;
					}else{
						Sleep(50);
					}
					delta=GetTickCount()-tick;
					if(delta>7000){
						const char *tmsg="timeout posting long message";
						add_line_mdi_nolog(win,tmsg);
						printf("%s\n",tmsg);
						timeout=1;
						break;
					}
				}
			}
			count=0;
		}
		if(is_cr)
			lines++;
		if(lines>1000)
			break;
		if(timeout)
			break;
		if(post_stop){
			add_line_mdi_nolog(win,"post thread stopped");
			break;
		}
	}

EXIT_PMSG:
	if(pmsg){
		if(pmsg->msg)
			free(pmsg->msg);
		free(pmsg);
	}
	post_thread_busy=0;
	_endthreadex(0);
	return 0;
}
int long_msg_warning(HWND hwnd,char *str)
{
	unsigned int i,len;
	int count,line_index,lines;
	char msg[512];
	char tmp[40];
	const char *hfmt="len=%-10i lines=%-10i busy=%i\r\n";
	len=strlen(str);
	_snprintf(msg,sizeof(msg),hfmt,0,0,0);
	lines=count=line_index=0;
	for(i=0;i<len;i++){
		char a=str[i];
		int is_cr;
		is_cr='\n'==a;
		if('\r'==a)
			continue;
		if(is_cr && i<(len-1)){
			lines++;
		}
		if(count<sizeof(tmp)){
			if(!is_cr){
				tmp[count++]=a;
			}
		}
		if(is_cr)
			line_index=0;
		else
			line_index++;
		if(i==(len-1) || is_cr || sizeof(msg)==line_index){
			if(lines<5 && count>0){
				if(count<sizeof(tmp))
					tmp[count]=0;
				tmp[sizeof(tmp)-1]=0;
				_snprintf(msg,sizeof(msg),"%s%s%s\r\n",msg,tmp,count>=sizeof(tmp)?"...":"");
				count=0;
				line_index=0;
			}
		}
	}
	_snprintf(tmp,sizeof(tmp),hfmt,len,lines+1,post_thread_busy);
	tmp[sizeof(tmp)-1]=0;
	for(i=0;i<sizeof(tmp);i++){
		char a;
		a=tmp[i];
		if(0==a)
			break;
		if(i<sizeof(msg))
			msg[i]=a;
	}
	msg[sizeof(msg)-1]=0;
	return MessageBoxA(hwnd,msg,"Warning: Ok to post?",MB_OKCANCEL);
}
int post_long_message(HWND hwnd,char *str,int show_warning)
{
	IRC_WINDOW *win=0;

	if(show_warning){
		if(IDOK!=long_msg_warning(hwnd,str))
			return FALSE;
	}
	win=find_window_by_hwnd(hwnd);
	if(win!=0){
		if(win->session==0 || (!irc_is_connected(win->session)))
			return FALSE;
	}
	if(post_thread_busy){
		add_line_mdi_nolog(win,"posting thread busy");
	}else{
		void *tmp;
		struct POST_MSG *pmsg;
		pmsg=malloc(sizeof(struct POST_MSG));
		if(pmsg){
			pmsg->msg=strdup(str);
			pmsg->win=win;
			post_thread_busy=1;
			tmp=_beginthreadex(NULL,0,post_msg_thread,pmsg,0,NULL);
			if(0==tmp){
				add_line_mdi_nolog(win,"failed to create posting thread");
				post_thread_busy=0;
				if(pmsg->msg)
					free(pmsg->msg);
				free(pmsg);
			}
		}
	}
	return TRUE;
}
int cmd_me(IRC_WINDOW *win,char *str)
{
	char channel[40],notice[256];
	if(win->type==PRIVMSG_WINDOW)
		extract_nick(win->channel,channel,sizeof(channel));
	else
		strncpy(channel,win->channel,sizeof(channel));

	if(0==irc_cmd_me(win->session,channel,str+4)){
		_snprintf(notice,sizeof(notice),"* %s %s",win->nick,str+4);
		notice[sizeof(notice)-1]=0;
		add_line_mdi(win,notice);
	}
	return 0;
}
int cmd_ctcp(IRC_WINDOW *win,char *str)
{
	char channel[40],msg[512];
	sscanf(str+sizeof("/ctcp ")-1,"%39s %511[^\n\r]s",channel,msg);
	channel[sizeof(channel)-1]=0;msg[sizeof(msg)-1]=0;
	irc_cmd_ctcp_request(win->session,channel,msg);
	echo_server_window(win->session,"PRIVMSG %s :%s",channel,msg);
	return 0;
}
int cmd_dcc(IRC_WINDOW *win,char *str)
{
	IRC_WINDOW *dcc_win=0;
	char tmp[512];
	tmp[0]=0;
	sscanf(str+sizeof("/dcc ")-1,"%32s",tmp);
	dcc_chat_init(win->session,tmp,&dcc_win);
	if(dcc_win){
		extern void dcc_event_callback(irc_session_t * session, irc_dcc_t id, int status, void * ctx, const char * data, unsigned int length);
		if(0!=irc_dcc_chat(dcc_win->session,dcc_win,tmp,dcc_event_callback,&dcc_win->dccid))
			add_line_mdi_nolog(dcc_win,"DCC CHAT FAILED");
	}
	echo_server_window(win->session,"DCC CHAT :%s",tmp);
	return 0;
}
int cmd_msg(IRC_WINDOW *win,char *str)
{
	char channel[40],msg[512];
	channel[0]=0;msg[0]=0;
	sscanf(str+sizeof("/msg ")-1,"%39s %511[^\n\r]s",channel,msg);
	channel[sizeof(channel)-1]=0;msg[sizeof(msg)-1]=0;
	if(msg[0]!=0){
		irc_cmd_msg(win->session,channel,msg);
		echo_server_window(win->session,"PRIVMSG %s :%s",channel,msg);
	}
	if(channel[0]!='#')
		initiate_privmsg(win->hwnd,channel);
	return 0;
}
int cmd_discon(IRC_WINDOW *win,char *str)
{
	IRC_WINDOW *ser=find_window_by_network(win->network);
	if(ser!=0){
		ser->disconnect=TRUE;
		irc_disconnect(ser->session);
	}
	return 0;
}
int cmd_recon(IRC_WINDOW *win,char *str)
{
	irc_disconnect(win->session);
	return 0;
}
int cmd_help(IRC_WINDOW *win,char *str)
{
	char *tmp=str+sizeof("/help")-1;
	if(strstri(tmp,"lua"))
		lua_help(add_line_mdi_nolog,win);
	else
		show_help(win);
	return 0;
}
int cmd_lua(IRC_WINDOW *win,char *str)
{
	char arg[20];
	char *tmp=str+sizeof("/lua")-1;
	arg[0]=0;
	sscanf(tmp,"%19s",arg);
	if(strstri(arg,"-create")){
		lua_create_default_file(add_line_mdi_nolog,win);
	}else{
		char *params[3]={win->channel,str+sizeof("/lua ")-1,(char*)win};
		lua_process_event(win->session,"USER_CALLED",win->nick,&params,3);
	}
	return 0;
}
int cmd_flushlogs(IRC_WINDOW *win,char *str)
{
	flush_all_logs();
	add_line_mdi_nolog(win,"log files flushed");
	return 0;
}
int cmd_stop(IRC_WINDOW *win,char *str)
{
	post_stop=1;
	return 0;
}
int cmd_speed(IRC_WINDOW *win,char *str)
{
	char *tmp=str+sizeof("/speed")-1;
	char arg[20];
	unsigned int val;
	arg[0]=0;
	sscanf(tmp,"%19s",arg);
	if(arg[0]!=0){
		val=strtoul(arg,0,10);
		if(val>=100 && val<=10000){
			post_speed=val;
			goto PRINT;
		}
	}else{
PRINT:
		_snprintf(arg,sizeof(arg),"speed=%i",post_speed);
		arg[sizeof(arg)-1]=0;
		add_line_mdi_nolog(win,arg);
	}
	return 0;
}
struct COMMAND{
	const char *cmd;
	const char *desc;
	int arg_count;
	int (*func)(IRC_WINDOW *win,char *cline);
};
struct COMMAND commands[]={
	{"me","",1,cmd_me},
	{"ctcp","nick VERSION|FINGER|PING|TIME",1,cmd_ctcp},
	{"dcc","",1,cmd_dcc},
	{"msg","",1,cmd_msg},
	{"discon","",0,cmd_discon},
	{"recon","",0,cmd_recon},
	{"help","[lua]",-1,cmd_help},
	{"lua","[-create] make default file [userfunc]",1,cmd_lua},
	{"flushlogs","",0,cmd_flushlogs},
	{"stop","stop posting long message",0,cmd_stop},
	{"speed","milliseconds delay",-1,cmd_speed},
};
int get_cmd_info(int index,const char **cmd,const char **desc)
{
	int size=sizeof(commands)/sizeof(struct COMMAND);
	if(index>=size)
		return FALSE;
	if(cmd)
		*cmd=commands[index].cmd;
	if(desc)
		*desc=commands[index].desc;
	return TRUE;
}
int post_message(HWND hwnd,char *str)
{
	IRC_WINDOW *win=0;
	win=find_window_by_hwnd(hwnd);
	if(win!=0){
		if(win->session==0 || (!irc_is_connected(win->session)))
			return FALSE;
		if(win->type==DCC_WINDOW){
			post_dcc_msg(win,str);
			return TRUE;
		}
		if(!valid_text(str))
			return FALSE;
		trim_return(str);
		handle_debug(str);
		{
			unsigned int i,len;
			char msg[512+20],tmp[512];
			char channel[80]={0};

			if('/'==str[0]){
				for(i=0;i<sizeof(commands)/sizeof(struct COMMAND);i++){
					int match=FALSE;
					char *tmp=str+1;
					struct COMMAND *cmd=&commands[i];
					len=strlen(cmd->cmd);
					if(0==strnicmp(cmd->cmd,tmp,len)){
						char a=tmp[len];
						match=TRUE;
						if(cmd->arg_count<=0){
							if(!(' '==a || 0==a))
								match=FALSE;
						}else if(!(' '==a)){
							match=FALSE;
						}
					}
					if(!match)
						continue;
					cmd->func(win,str);
					return TRUE;
				}
			}

			if(SERVER_WINDOW==win->type || '/'==str[0]){
				char *tmp=str;
				if('/'==str[0])
					tmp++;
				irc_send_raw(win->session,tmp);
				return TRUE;
			}else if(PRIVMSG_WINDOW==win->type)
				extract_nick(win->channel,channel,sizeof(channel));
			else{
				strncpy(channel,win->channel,sizeof(channel));
				channel[sizeof(channel)-1]=0;
			}

			len=strlen(str);
			for(i=0;i<len;i++){
				char a=str[i];
				if('\r'==a)
					continue;
				if('\n'==a)
					a=' ';
				if(i>=sizeof(tmp))
					break;
				tmp[i]=a;
			}
			if(i<sizeof(tmp))
				tmp[i]=0;
			tmp[sizeof(tmp)-1]=0;
			_snprintf(msg,sizeof(msg),"<%s> %s",win->nick,tmp);
			msg[sizeof(msg)-1]=0;
			if(0==irc_cmd_msg(win->session,channel,tmp))
				add_line_mdi(win,msg);
			else
				add_line_mdi_nolog(win,"failed to post message");
		}
	}
	return TRUE;
}
int add_nick(IRC_WINDOW *win,char *nick)
{
	int i,index=LB_ERR;
	HWND hwnd=win->hlist;
	char tmp[25]={0};
	char pre[3]={'@','+',0};
	char *n=nick;
	if(nick[0]=='+' || nick[0]=='@')
		n=nick+1;
	for(i=0;i<sizeof(pre);i++){
		if(pre[i]!=0)
			_snprintf(tmp,sizeof(tmp),"%c%s",pre[i],n);
		else
			strncpy(tmp,n,sizeof(tmp));
		index=SendMessage(hwnd,LB_FINDSTRINGEXACT,-1,tmp);
		if(index!=LB_ERR)
			SendMessage(hwnd,LB_DELETESTRING,index,0);
	}
	index=SendMessage(hwnd,LB_ADDSTRING,0,nick);
	return index;
}
int show_joins=FALSE;
int join_channel_event(void *session,const char *origin,const char *channel)
{
	IRC_WINDOW *server_win,*channel_win;
	server_win=find_window_by_session(session);
	if(server_win!=0){
		channel_win=acquire_channel_window(server_win->network,channel,CHANNEL_WINDOW);
		if(channel_win!=0){
			HWND hwnd=channel_win->hwnd;
			channel_win->session=session;
			strncpy(channel_win->server,server_win->server,sizeof(channel_win->server));
			strncpy(channel_win->nick,server_win->nick,sizeof(channel_win->nick));
			if(hwnd==0)
				hwnd=create_window_type(ghmdiclient,channel_win,CHANNEL_WINDOW,NULL);
			if(hwnd!=0){
				char nick[20]={0};
				channel_win->hwnd=hwnd;
				if(channel_win->hbutton==0)
					SendMessage(ghswitchbar,WM_APP,MSG_ADD_BUTTON,channel_win->hwnd);
				extract_nick(origin,nick,sizeof(nick));
				if(stricmp(nick,channel_win->nick)==0){
					bring_window_top(channel_win->hwnd);
					SendDlgItemMessage(channel_win->hlist,MDI_LIST,LB_RESETCONTENT,0,0);
				}
				else{
					char str[80];
					add_nick(channel_win,nick);
					_snprintf(str,sizeof(str),"* %s has joined %s",nick,channel);
					if(show_joins)
						add_line_mdi(channel_win,str);
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}
int join_channel(HWND hmdiclient,char *network,char *channel,char *password)
{
	IRC_WINDOW *network_win,*channel_win;
	network_win=find_window_by_network(network);
	if(network_win!=0){
		channel_win=find_channel_window(network_win->session,channel);
		if(channel_win==0)
			irc_cmd_join(network_win->session,channel,password);
		else{
			bring_window_top(channel_win->hwnd);
			handle_switch_button(channel_win->hbutton,FALSE);
		}
		return TRUE;		
	}

	return FALSE;
}
int autojoin_channels(void *session)
{
	int i;
	for(i=0;i<100;i++){
		char chan_section[80]={0};
		if(get_ini_entry("CHANNELS",i,chan_section,sizeof(chan_section))){
			int autojoin=0;
			get_ini_value(chan_section,"JOIN_CONNECT",&autojoin);
			if(autojoin!=0){
				IRC_WINDOW *win;
				char channel[80]={0},network[80]={0},password[20]={0};
				get_ini_str(chan_section,"NETWORK",network,sizeof(network));
				win=find_window_by_session(session);
				if(win!=0 && network[0]!=0 && stricmp(win->network,network)==0){
					get_ini_str(chan_section,"NAME",channel,sizeof(channel));
					get_ini_str(chan_section,"PASSWORD",password,sizeof(password));
					irc_cmd_join(session,channel,password);
				}
			}
		}
	}
	return TRUE;
}
int quit_irc_event(void *session,char *nick)
{
	int i;
	IRC_WINDOW *win=0;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].session==session)
			if(irc_windows[i].type==CHANNEL_WINDOW){
				win=&irc_windows[i];
				update_nick_in_list(win->hlist,nick,0);
			}
	}
	return TRUE;
}
int show_parts=FALSE;
int part_channel_event(void *session,const char *origin,const char *channel,const char *msg)
{
	IRC_WINDOW *win;
	win=find_channel_window(session,channel);
	if(win!=0){
		char quitmsg[256]={0};
		char nick[20]={0};
		extract_nick(origin,nick,sizeof(nick));
		update_nick_in_list(win->hlist,nick,0);
		if(show_parts){
			_snprintf(quitmsg,sizeof(quitmsg),"* %s has left %s%s%s",nick,channel,strlen(msg)>0?" saying:":"",msg);
			add_line_mdi(win,quitmsg);
		}
		return TRUE;
	}
	return FALSE;
}
int part_channel(IRC_WINDOW *win)
{
	if(win!=0){
		if(win->type==CHANNEL_WINDOW && win->session)
			irc_cmd_part(win->session,win->channel);
		DestroyWindow(win->hbutton);
		erase_irc_window(win->hwnd);
		return TRUE;
	}
	return FALSE;
}
int list_names_event(void *session,const char *channel,const char *names)
{
	IRC_WINDOW *win;
	win=find_channel_window(session,channel);
	if(win!=0){
		HWND hwnd=win->hwnd;
		if(hwnd!=0){
			int i,len,index=0;
			char n[20];
			len=strlen(names);
			for(i=0;i<=len;i++){
				BYTE a;
				a=names[i];
				if(!isspace(a)){
					if(index<sizeof(n)-1)
						n[index++]=names[i];
				}
				else{
					n[index++]=0;
					index=0;
					if(strlen(n)>0){
						add_nick(win,n);
					}
				}
			}

		}
	}
	return TRUE;
}


int update_nick_list(IRC_WINDOW *win,const char **params,int count)
{
	int i,len,mode,set=FALSE;
	//#1|+o|ptest123
	if(count<3)
		return FALSE;
	if(win!=0){
		const char *m=params[1];
		int index=2; //start of nick list
		len=strlen(m);
		for(i=0;i<len;i++){
			mode=0;
			switch(m[i]){
			case '+':set=TRUE;break;
			case '-':set=FALSE;break;
			case 'o':mode=m[i];break;
			case 'v':mode=m[i];break;
			}
			if(mode!=0 && (index<count)){
				char fullnick[20],*pre="";
				const char *nick=params[index];
				if(set && mode=='v'){ //dont change @ to +
					int row;
					_snprintf(fullnick,sizeof(fullnick),"@%s",nick);
					row=find_nick_in_list(win->hlist,fullnick);
					if(row<0)
						goto addnick;
				}
				else{
addnick:
					if(set){
						if(mode=='o')
							pre="@";
						else if(mode=='v')
							pre="+";
					}
					_snprintf(fullnick,sizeof(fullnick),"%s%s",pre,nick);
					add_nick(win,fullnick);
				}
			}
			if(mode!=0)
				index++;
		}
	}
	return TRUE;
}

int channel_msg_event(void *session,const char *origin,const char *channel,const char *msg,int type)
{
	IRC_WINDOW *win;
	char nick[20]={0};
	char str[1024];
	win=find_channel_window(session,channel);
	if(win!=0){
		extract_nick(origin,nick,sizeof(nick));
		if(type=='N')
			_snprintf(str,sizeof(str),"* %s %s",nick,msg);
		else
			_snprintf(str,sizeof(str),"<%s> %s",nick,msg);
		add_line_mdi(win,str);
		highlight_button_text(win);
	}
	return TRUE;
}


int update_channel_topic(void *session,const char *nick,const char *channel,const char *topic,int echo_channel)
{
	IRC_WINDOW *win;
	win=find_channel_window(session,channel);
	if(win!=0){
		char str[512]={0};
		_snprintf(str,sizeof(str),"%s %s",channel,topic);
		SetWindowText(win->hwnd,str);
		if(echo_channel){
			_snprintf(str,sizeof(str),"* %s set topic to: %s",nick,topic);
			add_line_mdi(win,str);
			highlight_button_text(win);
		}
	}
	return TRUE;
}

int handle_channel_menu(HWND hwnd,int wparam)
{
	IRC_WINDOW *win;
	char str[MAX_PATH+40];
	win=find_window_by_hwnd(hwnd);
	if(win==0)
		return FALSE;
	switch(wparam){
	case MDI_MENU_OPENLOG:
		str[0]=0;
		get_ini_path(str,sizeof(str));
		if(GetKeyState(VK_CONTROL)&0x8000){
			_snprintf(str,sizeof(str),"%slogs\\",str);
			ShellExecute(hwnd,"explore",str,NULL,NULL,SW_SHOWNORMAL);
		}
		else{
			_snprintf(str,sizeof(str),"%slogs\\%s.%s.log",str,win->channel,win->network);
			GetContextMenu(hwnd,str);
		}
		break;
	}
	return TRUE;
}