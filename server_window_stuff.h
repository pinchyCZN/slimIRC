int find_server_by_network(char *network)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].type==SERVER_WINDOW)
			if(irc_windows[i].network[0]!=0)
				if(stricmp(irc_windows[i].network,network)==0)
					return &irc_windows[i];
	}
	return 0;
}
int find_server_window(char *server)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].type==SERVER_WINDOW)
			if(irc_windows[i].server[0]!=0)
				if(stricmp(irc_windows[i].server,server)==0)
					return &irc_windows[i];
	}
	return 0;
}
int find_server_by_session(void *session)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].type==SERVER_WINDOW)
			if(irc_windows[i].session==session)
				return &irc_windows[i];
	}
	return 0;
}
int echo_server_window(void *session,const char *format,...)
{
	int i;
	static void *lastsess=0;
	static int lastindex=0;
	if(lastsess!=session){
		lastsess=lastindex=0;
		for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
			if(irc_windows[i].type==SERVER_WINDOW)
				if(irc_windows[i].session==session){
					lastsess=session;
					lastindex=i;
					break;
				}
		}
	}
	if((lastsess!=0) && (irc_windows[lastindex].hstatic!=0)){
		char buf[1024];
		va_list va_alist;
		va_start(va_alist,format);
		_vsnprintf(buf,sizeof(buf),format,va_alist);
		va_end(va_alist);
		add_line_mdi(&irc_windows[lastindex],buf);
		highlight_button_text(&irc_windows[lastindex]);
	}
	return 0;

}

int server_thread(SERVER_THREAD *thread)
{
	extern CRITICAL_SECTION mutex;
	DWORD tick;
	IRC_WINDOW *win=0;
	irc_session_t *session;
	irc_callbacks_t *callbacks;
	printf("irc thread started\n");
	PostMessage(ghmainframe,WM_USER,0,0);
	callbacks=malloc(sizeof(irc_callbacks_t));
	if(callbacks!=0){
		while(session=thread->session=create_session(callbacks)){
			tick=GetTickCount();
			win=find_server_window(thread->server);
			if(win!=0){
				win->session=session;
				thread->disconnected=FALSE;
				irc_connect_run(session,thread->server,thread->port,thread->nick,thread->password);
				EnterCriticalSection(&mutex);
					win->session=0;
					irc_destroy_session(session);
					erase_all_sessions(session);
					thread->disconnected=TRUE;
				LeaveCriticalSection(&mutex);
				printf("disconnected\n");
				win=find_server_window(thread->server);
				if(win==0 || win->hwnd==0 || win->disconnect)
					goto quit;
				else
					add_line_mdi(win,"disconnected, attempting reconnect");
			}
			else{
				irc_destroy_session(session);
				erase_all_sessions(session);
quit:
				session=thread->session=0;
				break;
			}
			if(GetTickCount()-tick<5000)
				Sleep(5000);
		};
		free(callbacks);
	}
	printf("thread ending\n");
	win=find_server_window(thread->server);
	erase_server_thread(thread);
	if(win!=0 && win->hwnd!=0)
		PostMessage(win->hwnd,WM_CLOSE,0,0);
	PostMessage(ghmainframe,WM_USER,0,0);
	return _endthread();
}
int connect_server(HWND hmdiclient,char *network,char *serv,int port,int ssl,char *password)
{
	char server[80];
	HWND hwndmdi;
	IRC_WINDOW *win=0;
	SERVER_THREAD *thread=0;

	if(ssl && (serv[0]!='#'))
		_snprintf(server,sizeof(server),"#%s",serv);
	else
		_snprintf(server,sizeof(server),"%s",serv);
	win=find_server_window(server);
	if(win!=0){
		BringWindowToTop(win->hwnd);
		handle_switch_button(win->hbutton,FALSE);
		return TRUE;
	}
	thread=acquire_server_thread(network,server,port,password);
	if(thread==0)
		return FALSE;
	win=acquire_server_window(network,server,port,password);
	if(win!=0){
		if(hwndmdi=create_server_window(hmdiclient,win)){
			win->hwnd=hwndmdi;
			strncpy(win->password,password,sizeof(win->password));
			strncpy(win->nick,"slimirc",sizeof(win->nick));
			get_ini_str("SETTINGS","NICK",win->nick,sizeof(win->nick));
			strncpy(thread->nick,win->nick,sizeof(thread->nick));
			if(win->hbutton==0)
				SendMessage(ghswitchbar,WM_USER,win->hwnd,0);
			if(!thread->thread_started){
				thread->thread_started=TRUE;
				_beginthread(server_thread,0,thread);
			}
			else if(!thread->disconnected){
				win->session=thread->session;
				update_chat_sessions(win->session);
			}
			return TRUE;
		}
	}
	else
		erase_server_thread(thread);
	return FALSE;
}
int create_server_window(HWND hmdiclient,IRC_WINDOW *win)
{ 
	int maximized=0,style,handle;
	MDICREATESTRUCT cs;
	char title[256]={0};
	get_ini_value("SETTINGS","MDI_MAXIMIZED",&maximized);
	if(maximized!=0)
		style = WS_MAXIMIZE|MDIS_ALLCHILDSTYLES;
	else
		style = MDIS_ALLCHILDSTYLES;
	cs.cx=cs.cy=cs.x=cs.y=CW_USEDEFAULT;
	cs.szClass="serverwindow";
	_snprintf(title,sizeof(title),"%s %s",win->network,win->server);
	cs.szTitle=title;
	cs.style=style;
	cs.hOwner=ghinstance;
	cs.lParam=win;
	handle=SendMessage(hmdiclient,WM_MDICREATE,0,&cs);
	return handle;
}
int update_status_window(HWND hwnd,HMENU hmenu)
{
	int i,count=0;
	for(i=0;i<sizeof(server_threads)/sizeof(SERVER_THREAD);i++){
		if(server_threads[i].thread_started!=0)
			count++;
	}
	if(count>0){
		char str[100];
		_snprintf(str,sizeof(str),"(%i threads)",count);
		DeleteMenu(hmenu,ID_DISCONNECT+1,MF_BYCOMMAND);
		InsertMenu(hmenu,ID_DISCONNECT+1,MF_BYCOMMAND|MF_STRING,ID_DISCONNECT+1,str);
	}
	else
		DeleteMenu(hmenu,ID_DISCONNECT+1,MF_BYCOMMAND);
	DrawMenuBar(hwnd);
	return count;
}

int disconnect_server(IRC_WINDOW *win)
{
	static char *randoms[]={
		"quiters gonna quit",
		"be seein ya",
		"kids come running for the great taste of sampo",
		"i didnt do it, it was the one armed man",
		"for all the kings horse and all the no carrier",
		"what happen capn?",
		"",

	};
	if(win!=0){
		if(win->session!=0){
			char quitmsg[255];
			int rnd,timeout;
			srand(GetTickCount()&0x1FFF);
			rnd=rand();
			rnd=rnd%(sizeof(randoms)/sizeof(char*));
			_snprintf(quitmsg,sizeof(quitmsg),randoms[rnd]);
			if(randoms[rnd][0]==0){
				_snprintf(quitmsg,sizeof(quitmsg),"this is one hell of a bog standard quit message");
				get_ini_str("SETTINGS","QUIT_MSG",quitmsg,sizeof(quitmsg));
			}
			win->disconnect=TRUE;
			if(irc_is_connected(win->session))
				irc_cmd_quit(win->session,quitmsg);
			timeout=0;
		}
		SendMessage(ghswitchbar,WM_USER+1,win->hbutton,0);
		erase_irc_window(win->hwnd);
	}
	return TRUE;
}

int exit_irc(int notify)
{
	int i,count=0;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].type==SERVER_WINDOW)
			if(irc_windows[i].hwnd!=0){
				if(notify){
					SendMessage(irc_windows[i].hwnd,WM_CLOSE,0,0);
					count++;
				}
				else
					disconnect_server(&irc_windows[i]);
			}
	}
	if(notify && count==0) //force disconnect
		disconnect_all_threads();
	return TRUE;
}
int wait_for_disconnect()
{
	int i,result=TRUE;
	for(i=0;i<sizeof(server_threads)/sizeof(SERVER_THREAD);i++){
		if(server_threads[i].session!=0){
			int timeout=0;
			while(server_threads[i].session!=0){
				timeout++;
				Sleep(50);
				if(timeout>75){
					result=FALSE;
					break;
				}
			}
		}
	}
	return result;
}