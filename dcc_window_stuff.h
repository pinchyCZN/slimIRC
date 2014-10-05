int find_dcc_window(void *session,int dccid)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].type==DCC_WINDOW)
			if(irc_windows[i].session==session)
				if(irc_windows[i].dccid==dccid)
						return &irc_windows[i];
	}
	return 0;

}
int acquire_dcc_window(IRC_WINDOW *server_win,int dccid)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].network[0]==0){
			strncpy(irc_windows[i].network,server_win->network,sizeof(irc_windows[i].network));
			strncpy(irc_windows[i].channel,server_win->channel,sizeof(irc_windows[i].channel));
			irc_windows[i].type=DCC_WINDOW;
			irc_windows[i].dccid=dccid;
			return &irc_windows[i];
		}
	}
	return 0;
}
int dcc_chat_event(void *session,int dccid,const char *origin,const char *addr,void **ctx)
{
	int result=FALSE;
	IRC_WINDOW *server_win,*dcc_win;
	char nick[20]={0};
	extract_nick(origin,nick,sizeof(nick));

	dcc_win=find_dcc_window(session,dccid);
	if(dcc_win==0){
//		if(IDOK!=show_messagebox(ghmainframe,"Do want to accept DCC from","Accept DCC?",MB_OKCANCEL))
//			return FALSE;
		server_win=find_server_by_session(session);
		if(server_win!=0){
			dcc_win=acquire_dcc_window(server_win,dccid);
			if(dcc_win!=0){
				HWND hwnd=dcc_win->hwnd;
				dcc_win->session=session;
				strncpy(dcc_win->server,server_win->server,sizeof(dcc_win->server));
				strncpy(dcc_win->nick,server_win->nick,sizeof(dcc_win->nick));
				strncpy(dcc_win->channel,nick,sizeof(dcc_win->channel));
				if(hwnd==0){
					hwnd=create_window_type(ghmdiclient,dcc_win,DCC_WINDOW,origin);
				}
				if(hwnd!=0){
					dcc_win->hwnd=hwnd;
					if(dcc_win->hbutton==0)
						SendMessage(ghswitchbar,WM_APP,MSG_ADD_BUTTON,dcc_win->hwnd);
				}
			}
		}
	}
	if(dcc_win!=0 && dcc_win->hwnd!=0){
		if(ctx)
			*ctx=dcc_win;
		result=TRUE;
		BringWindowToTop(dcc_win->hwnd);
	}
	return result;
}
int print_nick_msg(const char *nick,const char *msg,char *str,int len)
{
	_snprintf(str,len,"<%s> %s",nick,msg);
	if(len>0)
		str[len-1]=0;
	return TRUE;
}
int dcc_callback(void *session,int status,IRC_WINDOW *win,const char *data,int length)
{
	if(session){
		if(win && win->hwnd){
			char str[512+20];
			if(status==0){
				if(length>0){
					print_nick_msg(win->channel,data,str,sizeof(str));
					add_line_mdi_nolog(win,str);
				}
			}
			else if(status==LIBIRC_ERR_CLOSED){
				add_line_mdi_nolog(win,"*ERROR DCC CLOSED");
			}
			else{
				print_nick_msg(win->channel,"ERROR DCC SESSION",str,sizeof(str));
				add_line_mdi_nolog(win,str);
			}
			dprintf(0,"dcc %i %s\n",status,data);
		}
	}
	return TRUE;
}
int post_dcc_msg(IRC_WINDOW *win,char *msg)
{
	if(win && win->session){
		if(0==irc_dcc_msg(win->session,win->dccid,msg)){
			char str[512+20];
			print_nick_msg(win->nick,msg,str,sizeof(str));
			add_line_mdi_nolog(win,str);
		}
	}
	return TRUE;
}
int dcc_close_window(IRC_WINDOW *win)
{
	if(win){
		if(win->session)
			irc_dcc_destroy(win->session,win->dccid);
		part_channel(win);
	}
	return TRUE;
}