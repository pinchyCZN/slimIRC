int acquire_privmsg_window(char *network,char *channel)
{
	return acquire_channel_window(network,channel,PRIVMSG_WINDOW);
}
int find_msg_window(void *session,const char *nick)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].type==PRIVMSG_WINDOW)
			if(irc_windows[i].session==session)
				if(_stricmp(irc_windows[i].channel,nick)==0)
						return &irc_windows[i];
	}
	return 0;
}
/*
Event "PRIVMSG", origin: "ertert!234234@127.0.0.1", params: 2 [slimicy|678]
'ertert!234234@127.0.0.1' said me (slimicy): 678
*/
int privmsg_event(void *session,const char *origin,const char *mynick,const char *msg,int type)
{
	IRC_WINDOW *server_win,*privmsg_win;
	char nick[20]={0};
	extract_nick(origin,nick,sizeof(nick));

	privmsg_win=find_msg_window(session,nick);
	if(privmsg_win==0){
		server_win=find_window_by_session(session);
		if(server_win!=0){
			privmsg_win=acquire_privmsg_window(server_win->network,nick);
			if(privmsg_win!=0){
				HWND hwnd=privmsg_win->hwnd;
				privmsg_win->session=session;
				strncpy(privmsg_win->server,server_win->server,sizeof(privmsg_win->server));
				strncpy(privmsg_win->nick,mynick,sizeof(privmsg_win->nick));
				if(hwnd==0){
					hwnd=create_window_type(ghmdiclient,privmsg_win,PRIVMSG_WINDOW,nick);
					bring_window_top(hwnd);
				}
				if(hwnd!=0){
					privmsg_win->hwnd=hwnd;
					if(privmsg_win->hbutton==0)
						SendMessage(ghswitchbar,WM_APP,MSG_ADD_BUTTON,privmsg_win->hwnd);
				}
			}
		}
	}
	if(privmsg_win!=0){
		char str[512+20]={0};
		char nick[20]={0};
		if(strlen(msg)>0){
			extract_nick(origin,nick,sizeof(nick));
			if(type=='N')
				_snprintf(str,sizeof(str),"* %s %s",nick,msg);
			else
				_snprintf(str,sizeof(str),"<%s> %s",nick,msg);
			add_line_mdi(privmsg_win,str);
			highlight_button_text(privmsg_win);
		}
		else
			bring_window_top(privmsg_win->hwnd);
	}
	return TRUE;
}
int initiate_privmsg(HWND hwnd,char *nick)
{
	IRC_WINDOW *win;
	win=find_window_by_hwnd(hwnd);
	if(win!=0){
		privmsg_event(win->session,nick,win->nick,"",0);
	}
	return TRUE;
}