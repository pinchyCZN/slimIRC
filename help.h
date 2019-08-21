BOOL CALLBACK help_dlg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static IRC_WINDOW *win=0;
	static int cur_sel=0;
	static char *help_strs[]={
		"/debug console [on|off]] [debug ?]",
		"F2 view ascii art",
		"list current channels"
	};
	if(FALSE){
		static DWORD tick=0;
		if((GetTickCount()-tick)>500)
			printf("--\n");
		printf("stat");
		print_msg(msg,lparam,wparam,hwnd);
		tick=GetTickCount();
	}
	switch(msg){
	case WM_INITDIALOG:
		{
			int i;
			HWND hlist=GetDlgItem(hwnd,IDC_LIST);
			win=lparam;
			for(i=0;i<100;i++){
				const char *cmd=0,*desc=0;
				char tmp[80];
				if(!get_cmd_info(i,&cmd,&desc))
					break;
				_snprintf(tmp,sizeof(tmp),"/%s %s",cmd,desc);
				tmp[sizeof(tmp)-1]=0;
				SendMessage(hlist,LB_ADDSTRING,0,tmp);
			}
			for(i=0;i<sizeof(help_strs)/sizeof(char *);i++){
				SendMessage(hlist,LB_ADDSTRING,0,help_strs[i]);
			}
			SendMessage(hlist,LB_SETCURSEL,cur_sel,0);
			if(win!=0){
				RECT rect={0};
				int w,h;
				char str[80];
				GetClientRect(win->hstatic,&rect);
				w=rect.right-rect.left;
				h=rect.bottom-rect.top;
				SetWindowPos(hwnd,NULL,0,0,w,h,SWP_NOMOVE|SWP_NOZORDER);
				_snprintf(str,sizeof(str),"Help - %s %s",win->network,win->channel);
				str[sizeof(str)-1]=0;
				SetWindowText(hwnd,str);
			}
		}
	case WM_SIZE:
		{
			RECT rect={0};
			int w,h;
			GetClientRect(hwnd,&rect);
			w=rect.right-rect.left;
			h=rect.bottom-rect.top;
			SetWindowPos(GetDlgItem(hwnd,IDC_LIST),NULL,0,0,w,h,SWP_NOMOVE|SWP_NOZORDER);

		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam)){
		case IDC_LIST:
			if(HIWORD(wparam)==LBN_SELCHANGE){
				int sel=SendMessage(lparam,LB_GETCURSEL,0,0);
				if(sel>=0)
					cur_sel=sel;
				break;
			}
			if(HIWORD(wparam)!=LBN_DBLCLK)
				break;
		case IDOK:
			if(win!=0){
				int sel;
				HWND hlist=GetDlgItem(hwnd,IDC_LIST);
				sel=SendMessage(hlist,LB_GETCURSEL,0,0);
				if(sel>=0){
					int len;
					char str[80]={0};
					len=SendMessage(hlist,LB_GETTEXTLEN,sel,0);
					if(len< sizeof(str)){
						SendMessage(hlist,LB_GETTEXT,sel,str);
						str[sizeof(str)-1]=0;
						if(str[0]=='/'){
							char *start=strchr(str,'{');
							if(start){
								char *end=strchr(start,'}');
								if(end){
									SIZE_T x=start-str;
									if(x<sizeof(str)){
										x=sizeof(str)-(start-str);
										strncpy(start,end+1,x);
									}
								}
							}
							SetWindowText(win->hedit,str);
							SendMessage(win->hedit,EM_SETSEL,len,len);
						}
						else if(strcmp(str,"list current channels")==0){
							int i;
							SendMessage(hlist,LB_RESETCONTENT,0,0);
							SetWindowText(hwnd,"current channel list");
							for(i=0;i<sizeof(irc_windows)/sizeof(IRC_WINDOW);i++){
								IRC_WINDOW *w;
								w=&irc_windows[i];
								if(w && w->hwnd){
									_snprintf(str,sizeof(str),"%s,%s,%s",w->network,w->channel,w->nick);
									str[sizeof(str)-1]=0;
									SendMessage(hlist,LB_ADDSTRING,0,str);
								}
							}
							break;
						}
						else if(strnicmp(str,"F2",2)==0){
							if(win)
								PostMessage(win->hwnd,WM_APP,VK_F2,0);
						}
					}
				}
			}
		case IDCANCEL:
			EndDialog(hwnd,0);
			break;
		}
		break;
		case WM_HELP:
		case WM_CLOSE:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}
int show_help(IRC_WINDOW *win)
{
	HWND hparent=0;
	if(win!=0)
		hparent=win->hwnd;
	return DialogBoxParam(ghinstance,MAKEINTRESOURCE(IDD_HELP),hparent,help_dlg,win);
}