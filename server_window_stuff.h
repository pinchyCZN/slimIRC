int find_window_by_network(char *network)
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
int find_window_by_session(void *session)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].type==SERVER_WINDOW)
			if(irc_windows[i].session==session)
				return &irc_windows[i];
	}
	return 0;
}
//add more params as needed
int get_session_info(void *session,char *network,int nlen)
{
	int result=FALSE;
	IRC_WINDOW *win=0;
	if(session){
		win=find_window_by_session(session);
		if(win){
			if(network && nlen>0){
				strncpy(network,win->network,nlen);
				network[nlen-1]=0;
				result=TRUE;
			}
		}
	}
	return result;
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
	PostMessage(ghmainframe,WM_APP,MSG_UPDATE_STATUS,0);
	callbacks=malloc(sizeof(irc_callbacks_t));
	if(callbacks!=0){
		while(session=thread->session=create_session(callbacks)){
			tick=GetTickCount();
			win=find_window_by_network(thread->network);
			if(win!=0){
				win->session=session;
				thread->disconnected=FALSE;
				irc_connect_run(session,thread->server,thread->port,thread->nick,thread->password,thread->user);
				EnterCriticalSection(&mutex);
					win=find_window_by_network(thread->network);
					if(win!=0)
						win->session=0;
					irc_destroy_session(session);
					erase_all_sessions(session);
					thread->disconnected=TRUE;
				LeaveCriticalSection(&mutex);
				printf("disconnected\n");
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
			while((GetTickCount()-tick)<5000){
				Sleep(500);
				if(win!=0 && (win->disconnect || (win->hwnd==0)))
					break;
			}
		};
		free(callbacks);
	}
	printf("thread ending\n");
	win=find_window_by_network(thread->network);
	erase_server_thread(thread);
	if(win!=0 && win->hwnd!=0)
		PostMessage(win->hwnd,WM_CLOSE,0,0);
	PostMessage(ghmainframe,WM_APP,MSG_UPDATE_STATUS,0);
	return _endthread();
}
int connect_server(HWND hmdiclient,
				   char *network,char *serv,int port,int ssl,char *password,char *user,char *nick)
{
	char server[80];
	HWND hwndmdi;
	IRC_WINDOW *win=0;
	SERVER_THREAD *thread=0;

	if(ssl && (serv[0]!='#'))
		_snprintf(server,sizeof(server),"#%s",serv);
	else
		_snprintf(server,sizeof(server),"%s",serv);
	win=find_window_by_network(network);
	if(win!=0){
		if(stricmp(serv,win->server)!=0){
			char str[256]={0};
			_snprintf(str,sizeof(str),"Already connected to %s via [%s]\r\nDisconnect from that server and try again",network,win->server);
			show_messagebox(hmdiclient,str,"Warning",MB_OK);
			return FALSE;
		}
		bring_window_top(win->hwnd);
		handle_switch_button(win->hbutton,FALSE);
		return TRUE;
	}
	thread=acquire_network_thread(network,server,port,password,user,nick);
	if(thread==0)
		return FALSE;
	win=acquire_network_window(network,server,port,password);
	if(win!=0){
		if(hwndmdi=create_window_type(hmdiclient,win,SERVER_WINDOW,NULL)){
			win->hwnd=hwndmdi;
			strncpy(win->password,password,sizeof(win->password));
			strncpy(win->nick,"slimirc",sizeof(win->nick));
			get_ini_str("SETTINGS","NICK",win->nick,sizeof(win->nick));
			if(thread->nick[0]==0)
				strncpy(thread->nick,win->nick,sizeof(thread->nick));
			else
				strncpy(win->nick,thread->nick,sizeof(win->nick));
			if(win->hbutton==0)
				SendMessage(ghswitchbar,WM_APP,MSG_ADD_BUTTON,win->hwnd);
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
		"this is one hell of a bog standard quit message"
	};
	if(win!=0){
		if(win->session!=0){
			char quitmsg[255]={0};
			get_ini_str("SETTINGS","QUIT_MSG",quitmsg,sizeof(quitmsg));
			if(quitmsg[0]==0){
				int rnd=0;
				get_ini_value("SETTINGS","RND_MSG_NUM",&rnd);
				rnd=rnd%(sizeof(randoms)/sizeof(char*));
				_snprintf(quitmsg,sizeof(quitmsg),randoms[rnd]);
				rnd++;
				write_ini_value("SETTINGS","RND_MSG_NUM",rnd);
			}
			quitmsg[sizeof(quitmsg)-1]=0;
			win->disconnect=TRUE;
			if(irc_is_connected(win->session)){
				irc_cmd_quit(win->session,quitmsg);
			}
		}
		SendMessage(ghswitchbar,WM_APP,MSG_DEL_BUTTON,win->hbutton);
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
static int waiting_disconnect=FALSE;
int process_msgs(CRITICAL_SECTION *mutex)
{
	extern HWND ghmdiclient;
	MSG msg;
	int result;
	result=PeekMessage(&msg,NULL,0,0,PM_REMOVE);
	if(result!=0){
		EnterCriticalSection(mutex);
		if(!custom_dispatch(&msg))
			if(!TranslateMDISysAccel(ghmdiclient, &msg)){
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		LeaveCriticalSection(mutex);
		return TRUE;
	}
	return FALSE;
}
int wait_for_disconnect(CRITICAL_SECTION *mutex)
{
	int i,result=TRUE;
	waiting_disconnect=TRUE;
	for(i=0;i<sizeof(server_threads)/sizeof(SERVER_THREAD);i++){
		if(server_threads[i].session!=0){
			int timeout=0;
			while(server_threads[i].session!=0){
				timeout++;
				Sleep(10);
				//process_msgs(mutex);
				if(timeout>75){
					result=FALSE;
					break;
				}
			}
		}
	}
	waiting_disconnect=FALSE;
	return result;
}
int wait_for_all()
{
	int i=0;
	while(TRUE){
		if(waiting_disconnect==FALSE)
			break;
		Sleep(1);
		i++;
		if(i>500)
			break;
	}
	return TRUE;
}