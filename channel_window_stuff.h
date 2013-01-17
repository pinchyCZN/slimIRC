int find_channel_window(char *session,char *channel)
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

int update_chat_sessions(void *session)
{
	int i;
	IRC_WINDOW *win;
	win=find_server_by_session(session);
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
int post_message(HWND hwnd,char *str)
{
	int i;
	IRC_WINDOW *win=0;
	win=find_window_by_hwnd(hwnd);
	if(win!=0){
		if(win->session==0 || (!irc_is_connected(win->session)))
			return FALSE;
		handle_debug(str);
		if(win->type==SERVER_WINDOW){
			irc_send_raw(win->session,str);
			add_history(str);
		}
		else{
			int start,len,index,lines;
			char msg[512+20],tmp[512];
			char channel[40]={0};
			if(win->session==0)
				return FALSE;
			if(win->type==PRIVMSG_WINDOW)
				extract_nick(win->channel,channel,sizeof(channel));
			else
				strncpy(channel,win->channel,sizeof(channel));
			len=strlen(str);
			if(len>4 && str[0]=='/' && str[1]!='/'){
				char notice[256];
				if(strnicmp(str,"/me",3)==0){
					irc_cmd_me(win->session,channel,str+4);
					_snprintf(notice,sizeof(notice),"* %s %s",win->nick,str+4);
					add_line_mdi(win,notice);
				}
				else if(strnicmp(str,"/discon",sizeof("/discon")-1)==0){
					IRC_WINDOW *ser=find_server_window(win->server);
					if(ser!=0){
						ser->disconnect=TRUE;
						irc_disconnect(ser->session);
					}
				}
				else if(strnicmp(str,"/recon",sizeof("/recon")-1)==0){
					irc_disconnect(win->session);
				}
				else
					irc_send_raw(win->session,str+1);
				add_history(str);
				return TRUE;
			}

			start=FALSE;index=0;lines=0;
			for(i=0;i<len+1;i++){
				if(start){
					if((index>=sizeof(tmp)-1) || str[i]=='\r' || str[i]=='\n' || str[i]==0){
						tmp[index++]=0;
						_snprintf(msg,sizeof(msg),"<%s> %s",win->nick,tmp);
						add_line_mdi(win,msg);
						add_history(tmp);
						irc_cmd_msg(win->session,channel,tmp);
						start=FALSE;
						index=0;
						lines++;
					}else
						tmp[index++]=str[i];
				}
				else{
					if(!(str[i]=='\r' || str[i]=='\n')){
						tmp[index++]=str[i];
						start=TRUE;
					}
				}
				if(lines>10)
					break;

			}
		}
	}
	return TRUE;
}
int create_channel_window(HWND hmdiclient,IRC_WINDOW *win)
{
	int maximized=0,style,handle;
	MDICREATESTRUCT cs;
	get_ini_value("SETTINGS","MDI_MAXIMIZED",&maximized);
	if(maximized!=0)
		style = WS_MAXIMIZE|MDIS_ALLCHILDSTYLES;
	else
		style = MDIS_ALLCHILDSTYLES;
	cs.cx=cs.cy=cs.x=cs.y=CW_USEDEFAULT;
	cs.szClass="channelwindow";
	cs.szTitle=win->channel;
	cs.style=style;
	cs.hOwner=ghinstance;
	cs.lParam=win;
	handle=SendMessage(hmdiclient,WM_MDICREATE,0,&cs);
	return handle;
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
int join_channel_event(void *session,char *origin,char *channel)
{
	IRC_WINDOW *server_win,*channel_win;
	server_win=find_server_by_session(session);
	if(server_win!=0){
		channel_win=acquire_channel_window(server_win->network,channel,CHANNEL_WINDOW);
		if(channel_win!=0){
			HWND hwnd=channel_win->hwnd;
			channel_win->session=session;
			strncpy(channel_win->server,server_win->server,sizeof(channel_win->server));
			strncpy(channel_win->nick,server_win->nick,sizeof(channel_win->nick));
			if(hwnd==0)
				hwnd=create_channel_window(ghmdiclient,channel_win);
			if(hwnd!=0){
				char nick[20]={0};
				channel_win->hwnd=hwnd;
				if(channel_win->hbutton==0)
					SendMessage(ghswitchbar,WM_USER,channel_win->hwnd,0);
				extract_nick(origin,nick,sizeof(nick));
				if(stricmp(nick,channel_win->nick)==0){
					BringWindowToTop(channel_win->hwnd);
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
	char str[256]={0};
	IRC_WINDOW *server_win,*channel_win;
	server_win=find_server_by_network(network);
	if(server_win!=0){
		channel_win=find_channel_window(server_win->session,channel);
		if(channel_win==0)
			irc_cmd_join(server_win->session,channel,password);
		else{
			BringWindowToTop(channel_win->hwnd);
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
				win=find_server_by_session(session);
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
int part_channel_event(void *session,char *origin,char *channel,char *msg)
{
	IRC_WINDOW *win;
	win=find_channel_window(session,channel);
	if(win!=0){
		char quitmsg[256]={0};
		char nick[20]={0};
		extract_nick(origin,nick,sizeof(nick));
		update_nick_in_list(win->hlist,nick,0);
		_snprintf(quitmsg,sizeof(quitmsg),"* %s has left %s%s%s",nick,channel,strlen(msg)>0?" saying:":"",msg);
		add_line_mdi(win,quitmsg);
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
int list_names_event(void *session,char *channel,char *names)
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
				if(names[i]>' '){
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

int get_subitem(char *list,char *out,int max,int item)
{
	int i,len,index=0,word=0,white=TRUE;
	int found=FALSE;
	len=strlen(list);
	for(i=0;i<len;i++){
		if(list[i]>' '){
			if(word==item){
				out[index++]=list[i];
				found=TRUE;
			}
			white=FALSE;
		}
		else{ //whitespace
			if(white==FALSE)
				word++;
			white=TRUE;
		}
		if(word>item)
			break;
		if(index>=max-1)
			break;
	}
	if(found)
		out[index++]=0;
	return found;
}

int update_nick_list(IRC_WINDOW *win,char **params,int count)
{
	int i,len,mode,set=FALSE;
	char nick[20]={0};
	//#1|+o|ptest123
	if(count<3)
		return FALSE;
	if(win!=0){
		char *m=params[1];
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
				char fullnick[20],*pre="",*nick=params[index];
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

int channel_msg_event(void *session,char *origin,char *channel,char *msg,int type)
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
		handle_word_watch(win,nick,channel,msg);
	}
	return TRUE;
}


int update_channel_topic(void *session,char *nick,char *channel,char *topic,int echo_channel)
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
int handle_word_watch(IRC_WINDOW *win,char *nick,char *channel,char *msg)
{
	int i,index,len,start=FALSE;
	char word[80];
	len=strlen(word_watch);
	if(len>sizeof(word_watch))
		len=sizeof(word_watch);
	index=0;
	for(i=0;i<len;i++){
		if((index<(sizeof(word)-1)) && word_watch[i]!=','){
			word[index++]=word_watch[i];
			if(word_watch[i+1]==0)
				goto doword;
		}
		else{
doword:
			word[index++]=0;
			index=0;
			if(strstri(msg,word)!=0){
				if(win->session!=0){
					if(irc_is_connected(win->session)){
						IRC_WINDOW *privmsg_win;
						char str[256];
						privmsg_win=find_msg_window(win->session,win->nick);
						if(privmsg_win!=0){
							_snprintf(str,sizeof(str),"<%s> %s * %s %s",win->nick,nick,channel,msg);
							str[sizeof(str)-1]=0;
							add_line_mdi(privmsg_win,str);
							highlight_button_text(privmsg_win);
						}
						else{
							_snprintf(str,sizeof(str),"%s * %s %s",nick,channel,msg);
							str[sizeof(str)-1]=0;
							privmsg_event(win->session,win->nick,win->nick,str,0);
						}
					}
				}
				break;
			}
		}
	}
	return TRUE;
}
int update_word_watch(char *str)
{
	strncpy(word_watch,str,sizeof(word_watch));
	word_watch[sizeof(word_watch)-1]=0;
	return TRUE;
}