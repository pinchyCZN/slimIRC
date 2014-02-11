BOOL CALLBACK help_dlg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static IRC_WINDOW *win=0;
	static int cur_sel=0;
	static char *help_strs[]={
		"/msg","/me","/ctcp","/discon (disconnect)","/recon (reconnect)",
		"/help lua (list lua commands)","/help ctcp","/lua -create (make new script file)",
		"/lua xzy (call lua user_function with xzy paramter)","/flushlogs (flush all open file logs)",
		"/debug console [on|off]] [debug ?]",
		"F2 view ascii art",
		"list current channels"

	};
	{
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
			for(i=0;i<sizeof(help_strs)/sizeof(char*);i++){
				SendMessage(hlist,LB_ADDSTRING,0,help_strs[i]);
			}
			SendMessage(hlist,LB_SETCURSEL,cur_sel,0);
			if(win!=0){
				RECT rect={0};
				int w,h;
				GetClientRect(win->hstatic,&rect);
				w=rect.right-rect.left;
				h=rect.bottom-rect.top;
				SetWindowPos(hwnd,NULL,0,0,w,h,SWP_NOMOVE|SWP_NOZORDER);
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
						if(str[0]!=0 && str[0]=='/'){
							int i;
							len=strlen(str);
							for(i=0;i<len;i++){
								if(str[i]=='('){
									str[i]=0;
									break;
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