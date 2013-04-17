LRESULT CALLBACK switchbar_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static HBRUSH hbrush=0;
	static DWORD tick=0;
	if(control_debug("switch",0))
	if(/*msg!=WM_NCMOUSEMOVE&&*/msg!=WM_MOUSEFIRST&&msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE&&msg!=WM_NOTIFY)
		//if(msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE)
	{
		if((GetTickCount()-tick)>500)
			printf("--\n");
		printf("s");
		print_msg(msg,lparam,wparam,hwnd);
		tick=GetTickCount();
	}
	switch(msg){
	case WM_DRAWITEM:
		draw_button(hwnd,(LPDRAWITEMSTRUCT)lparam);
		return TRUE; 
	case WM_USER: //create button mdi hwnd in wparam
		{
		HWND hbutton=add_button(hwnd,wparam);
		if(hbutton!=0)
			SendMessage(hbutton,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),0);
		}
		break;
	case WM_USER+1: //destroy button hwnd in wparam
		DestroyWindow(wparam);
		break;
	case WM_USER+2:
		resize_buttons(ghswitchbar);
		break;
	case WM_PARENTNOTIFY:
		if(LOWORD(wparam)==WM_DESTROY)
			PostMessage(hwnd,WM_USER+2,0,0); //resize buttons after ones destroy
		break;
	case WM_CREATE:
		/*
		hbrush=CreateHatchBrush(HS_BDIAGONAL,RGB(100,100,100));
		button=CreateWindow("BUTTON","",WS_CHILD|WS_VISIBLE, //|BS_AUTOCHECKBOX|BS_PUSHLIKE	,
			0,0,40,50,hwnd,IDC_SWITCHBUTTON+20000,ghinstance,0);
		SetWindowText(button,"werwr");
		*/
		break;
	case WM_COMMAND:
		{
			int ID=LOWORD(wparam);
			if(ID>=IDC_SWITCHBUTTON+20000){
				if(HIWORD(wparam)==BN_CLICKED){
					handle_switch_button(lparam,TRUE);
				}
			}
		}
		break;
	case WM_SIZE:
		return 0;
		break;
	}
	return DefWindowProc(hwnd,msg,wparam,lparam);
	//return DefFrameProc(hwnd, ghmdiclient, msg, wparam, lparam);
}
int create_switchbar(HWND hwnd,HINSTANCE hinstance)
{
	//extern LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	WNDCLASS wndclass;
	HWND hswitch=0;
	memset(&wndclass,0,sizeof(wndclass));
	wndclass.lpfnWndProc=switchbar_proc;
	wndclass.hCursor=LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground=COLOR_BTNFACE+1;
	wndclass.lpszClassName="switchbar";
	wndclass.style=CS_HREDRAW|CS_VREDRAW;
	if(RegisterClass(&wndclass)!=0){
		hswitch=CreateWindow("switchbar","switchbar",
			WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_VISIBLE|WS_BORDER,
			0,0,
			0,0,
			hwnd,
			IDC_SWITCHBAR, //ID
			hinstance,
			NULL);
	}
	return hswitch;
}
int resize_switchbar(HWND hwnd,HWND mdi,HWND switchbar,int switch_height)
{
	RECT rect;
	if(GetClientRect(hwnd,&rect)!=0){
		int gap=4;
		int ypos=rect.bottom-switch_height;
		if(ypos<0)
			return TRUE;
//		printf("ypos=%3i rect.bottom=%3i\n",ypos,rect.bottom);
		MoveWindow(mdi,rect.left,rect.top,rect.right,ypos,TRUE);
		MoveWindow(switchbar,0,ypos+gap,rect.right,switch_height-gap,TRUE);
	}
	resize_buttons(switchbar);
	return TRUE;
}
int get_button_name(HWND hwnd,char *name,int len)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].hwnd==hwnd){
			if(irc_windows[i].type==SERVER_WINDOW)
				strncpy(name,irc_windows[i].server,len);
			else
				strncpy(name,irc_windows[i].channel,len);
			return TRUE;
		}
	}
	return FALSE;
}
int get_button_hwnd(HWND hwnd)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].hwnd==hwnd){
			return irc_windows[i].hbutton;
		}
	}
	return 0;
}
int add_button_to_list(HWND hwnd,HWND hbutton,int button_id)
{
	int i;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].hwnd!=0){
			if(irc_windows[i].hwnd==hwnd){
				irc_windows[i].hbutton=hbutton;
				irc_windows[i].button_id=button_id;
				irc_windows[i].pressed=TRUE;
				//SendMessage(irc_windows[i].hbutton,BM_SETSTATE,TRUE,0);
			}
			else{
				irc_windows[i].pressed=FALSE;
				//SendMessage(irc_windows[i].hbutton,BM_SETSTATE,FALSE,0);
			}
		}
	}
	return TRUE;
}
int add_button(HWND hswitch,HWND hclient)
{
	HWND hbutton;
	char name[40]={0};
	int button_id=acquire_button_id(hclient);
	get_button_name(hclient,name,sizeof(name));
	hbutton=CreateWindow("BUTTON",name,WS_CHILD|WS_VISIBLE|BS_OWNERDRAW|BS_AUTOCHECKBOX|BS_PUSHLIKE, //BS_AUTOCHECKBOX|BS_PUSHLIKE,
		0,0,0,0,hswitch,button_id,ghinstance,0);
	add_button_to_list(hclient,hbutton,button_id);
	resize_buttons(hswitch);
	return hbutton;
}
int acquire_button_id(HWND hwnd)
{
	int i,count=0;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].hwnd==hwnd)
			return IDC_SWITCHBUTTON+20000+i;
	}
	return IDC_SWITCHBUTTON+20000+i;
}
int get_button_count()
{
	int i,count=0;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].hbutton!=0)
			count++;
	}
	return count;
}
int button_sort(const IRC_WINDOW **win1,const IRC_WINDOW **win2)
{
	if(*win1!=0 && *win2!=0)
		return stricmp((**win1).channel,(**win2).channel);
	else
		return -1;
}
int resize_buttons(HWND hswitch)
{
	RECT rect;
	int i,count;
	int width,height;
	IRC_WINDOW *windows[sizeof(irc_windows)/sizeof(IRC_WINDOW)];
	count=get_button_count();
	if(count==0)
		return TRUE;
	GetClientRect(hswitch,&rect);
	width=(int)ceil(((double)rect.right/(double)count)-0.5);
	height=rect.bottom;
	if(width<20)
		width=20;
	if(height<20)
		height=20;
	count=0;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].hbutton!=0){
			windows[count]=&irc_windows[i];
			count++;
		}
	}
	qsort(windows,count,sizeof(IRC_WINDOW *),button_sort);
	for(i=0;i<count;i++){
		if(windows[i]!=0 && windows[i]->hbutton!=0)
			MoveWindow(windows[i]->hbutton,i*width,0,width,height,TRUE);
	}
	return TRUE;
}
WNDPROC *old_mdiclient=0;
LRESULT CALLBACK mdiclient_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifdef _DEBUG123
	static DWORD tick=0;
	if(msg!=WM_MOUSEFIRST&&msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE&&msg!=WM_NOTIFY)
	//if(msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE)
	{
		if((GetTickCount()-tick)>500)
			printf("--\n");
		print_msg(msg,lparam,wparam,hwnd);
		tick=GetTickCount();
	}
#endif
	if(msg==WM_MDINEXT){
		IRC_WINDOW *windows[sizeof(irc_windows)/sizeof(IRC_WINDOW)];
		HWND hwndChild=wparam;
		int fnext=lparam;
		int i,num_win=0,index=0;

		for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
			if(irc_windows[i].hwnd!=0){
				windows[index]=&irc_windows[i];
				index++;
			}
		}
		num_win=index;
		qsort(windows,num_win,sizeof(IRC_WINDOW *),button_sort);
		for(i=0;i<num_win;i++){
			if(windows[i]!=0 && windows[i]->hwnd!=0){
				if(windows[i]->hwnd==hwndChild){
					index=i;
					if(!fnext){
						if(i==num_win-1)
							index=0;
						else
							index=i+1;
					}
					else{
						if(i==0)
							index=num_win-1;
						else
							index=i-1;
					}
					SendMessage(hwnd,WM_MDIACTIVATE,windows[index]->hwnd,0);
					return 0;
					break;
				}
			}
		}
	}
	return CallWindowProc(old_mdiclient,hwnd,msg,wparam,lparam);
}
int subclass_mdi_client(HWND hwnd)
{
	old_mdiclient=SetWindowLong(hwnd,GWL_WNDPROC,mdiclient_proc);
	return old_mdiclient;
}
int handle_switch_button(HWND hbutton,int top)
{
	int i,count=0;
	if(hbutton==0)
		return FALSE;
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].hbutton!=0)
			count++;
	}
	if(count==1){
		irc_windows[i].pressed=TRUE;
		RedrawWindow(irc_windows[i].hbutton,NULL,NULL,RDW_INVALIDATE);
		return TRUE;
	}
	for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
		if(irc_windows[i].hbutton!=0){
			if(irc_windows[i].hbutton!=hbutton){
				irc_windows[i].pressed=FALSE;
//		printf("handle 2\n");
				RedrawWindow(irc_windows[i].hbutton,NULL,NULL,RDW_INVALIDATE);
			}else{
				if(top)
					BringWindowToTop(irc_windows[i].hwnd);
				irc_windows[i].pressed=TRUE;
//		printf("handle 3\n");
				RedrawWindow(irc_windows[i].hbutton,NULL,NULL,RDW_INVALIDATE);
			}
		}

	}
	return FALSE;
}
int highlight_button_text(IRC_WINDOW *win)
{
	if(!win->pressed){
		win->activity=TRUE;
		RedrawWindow(win->hbutton,NULL,NULL,RDW_INVALIDATE);
	}
	else
		win->activity=FALSE;
	return TRUE;
}

int draw_button(HWND hwnd,DRAWITEMSTRUCT *di)
{
	int i,count=0;
	IRC_WINDOW *win=0;
	static DWORD tick=0;
	int rnd=rand();
	IRC_WINDOW *bring_top=0,*redraw=0;
	static IRC_WINDOW *last_win=0;
	char *text=0;
//	if((GetTickCount()-tick)>500)
//		printf("---\n");
	tick=GetTickCount();
//	printf("%08X state=%08X\n",rnd,di->itemState);
	if(di->CtlID!=0){
		for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
			if(irc_windows[i].button_id==di->CtlID){
				win=&irc_windows[i];
				if(di->itemState==(ODS_FOCUS|ODS_SELECTED)){
					//irc_windows[i].pressed=TRUE;
					irc_windows[i].pressed=!irc_windows[i].pressed;
					irc_windows[i].activity=FALSE;
//					printf("%08X draw bring to top\n",rnd);
					if(irc_windows[i].pressed)
						bring_top=&irc_windows[i];
					else
						bring_top=last_win;
				}
			}
			if(irc_windows[i].pressed)
				count++;
		}
		if(count>1){
//			printf("%08X count=%i\n",rnd,count);
			for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
				if(irc_windows[i].hwnd!=0)
					if(irc_windows[i].button_id!=di->CtlID){
						if(irc_windows[i].pressed){
							last_win=&irc_windows[i];
							//RedrawWindow(irc_windows[i].hbutton,NULL,NULL,RDW_INVALIDATE);
							redraw=&irc_windows[i];
						}
						irc_windows[i].pressed=FALSE;
					}
			}
		}
		else{
//			printf("%08X only one\n",rnd);
		}
	}
	if(win!=0){
		int len,style;
		SIZE size;
		//CreateHatchBrush
		if(win->pressed){
			FillRect(di->hDC,&di->rcItem,(HBRUSH)(COLOR_BTNHIGHLIGHT+1));
			DrawEdge(di->hDC,&di->rcItem,EDGE_SUNKEN,BF_RECT);
		}
		else{
			FillRect(di->hDC,&di->rcItem,(HBRUSH)(COLOR_BTNFACE+1));
			DrawEdge(di->hDC,&di->rcItem,EDGE_RAISED,BF_RECT);
		}
		switch(win->type){
		default:
		case SERVER_WINDOW:text=win->server;break;
		case CHANNEL_WINDOW:text=win->channel;break;
		case PRIVMSG_WINDOW:text=win->channel;break;
		}
		if(win->activity && (!win->pressed))
			SetTextColor(di->hDC,(0xFFFFFF^GetSysColor(COLOR_BTNTEXT)));
		else
			SetTextColor(di->hDC,GetSysColor(COLOR_BTNTEXT));
		SetBkMode(di->hDC,TRANSPARENT);
		len=strlen(text);
		GetTextExtentPoint32(di->hDC,text,len,&size);
		if(size.cx>=di->rcItem.right)
			style=DT_LEFT;
		else
			style=DT_CENTER;
		style|=DT_VCENTER|DT_SINGLELINE;
		DrawText(di->hDC,text,-1,&di->rcItem,style);
	}
	if(redraw!=0)
		RedrawWindow(redraw->hbutton,NULL,NULL,RDW_INVALIDATE);
	if(bring_top!=0)
		BringWindowToTop(bring_top->hwnd);

	return TRUE;
}