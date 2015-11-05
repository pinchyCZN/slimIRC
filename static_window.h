WNDPROC orig_static_proc; 
UINT last_static_msg=0;
LRESULT CALLBACK  static_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static POINTL rmb_pos;
	static int lmb_down=FALSE,mouse_wheel=0;
	static char cmd_target[sizeof(mouse_target)]={0};
	if(control_debug(MDI_STATIC,0,0))
	if(/*msg!=WM_NCMOUSEMOVE&&msg!=WM_MOUSEFIRST&&*/msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE&&msg!=WM_NOTIFY)
		//if(msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE)
	{
		static DWORD tick=0;
		if((GetTickCount()-tick)>500)
			printf("--\n");
		printf("stat");
		print_msg(msg,lparam,wparam,hwnd);
		tick=GetTickCount();
	}
	last_static_msg=msg;
	switch(msg){
	case WM_NCMOUSEMOVE:
		SetCursor(LoadCursor(NULL,IDC_ARROW));
		break;
	case WM_MOUSEMOVE:
		{
		POINTL p;
		p.y=HIWORD(lparam);
		p.x=LOWORD(lparam);
		if(!handle_static_links(hwnd,&p,0))
			SetCursor(LoadCursor(NULL,IDC_ARROW));
		}
		//return TRUE;
		break;
	case WM_SETCURSOR:
		return TRUE;
		break;
	case WM_VSCROLL:
		PostMessage(hwnd,WM_APP+1,wparam,lparam);
		break;
	case WM_MOUSEWHEEL:
		{
		int delta,key;
		mouse_wheel++;
		key=LOWORD(wparam);
		delta=(short)HIWORD(wparam);
		if(delta>0){
			if(key&MK_CONTROL)
				SendMessage(hwnd,WM_VSCROLL,SB_LINEUP,0);
			else if(key&MK_SHIFT || (LOWORD(wparam)&MK_RBUTTON))
				SendMessage(hwnd,WM_VSCROLL,SB_PAGEUP,0);
			else{
				SendMessage(hwnd,WM_VSCROLL,SB_LINEUP,0);
				SendMessage(hwnd,WM_VSCROLL,SB_LINEUP,0);
			}
		}
		else{
			if(key&MK_CONTROL)
				SendMessage(hwnd,WM_VSCROLL,SB_LINEDOWN,0);
			else if(key&MK_SHIFT || (LOWORD(wparam)&MK_RBUTTON))
				SendMessage(hwnd,WM_VSCROLL,SB_PAGEDOWN,0);
			else{
				SendMessage(hwnd,WM_VSCROLL,SB_LINEDOWN,0);
				SendMessage(hwnd,WM_VSCROLL,SB_LINEDOWN,0);
			}
		}
		}
		return 0;
	case WM_TIMER:
		break;
	case WM_SETFOCUS:
		if(!lmb_down){
			set_focus_edit(hwnd);
			return 0;
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam)){
		case CMD_CHANGE_NICK:
		case CMD_PRIV_MSG:
		case CMD_SLAP:
		case CMD_WHOIS:
		case CMD_OP:
		case CMD_DEOP:
		case CMD_VOICE:
		case CMD_DEVOICE:
		case CMD_KICK:
		case CMD_REFRESH_LIST:
			{
			IRC_WINDOW *win=find_window_by_hwnd(GetParent(hwnd));
			if(win!=0 && win->session!=0)
				handle_nick_links(win,cmd_target,LOWORD(wparam));
			}
			break;
		case CMD_TSEARCH:
			SendMessage(GetParent(hwnd),WM_APP,0x6,0);
			break;
		case CMD_TEST:
			break;
		case CMD_CLEARTYPED:
			memset(edit_buffer,0,sizeof(edit_buffer));
			buffer_pos=0;
			break;
		case CMD_CLEAR:
			SetWindowText(hwnd,"");
			break;
		case CMD_CHANMODES:
chan_modes:
			{
			IRC_WINDOW *win=find_window_by_hwnd(GetParent(hwnd));
				if(win!=0 && win->session!=0 && 
					irc_is_connected(win->session) && 
					win->type==CHANNEL_WINDOW){
					chan_mode_parent=GetParent(hwnd);
					DialogBox(ghinstance,MAKEINTRESOURCE(IDD_CHANNELMODES),hwnd,chan_modes);
				}
			}
			break;
		case CMD_OPENURL:
			ShellExecute(NULL,"open",cmd_target,NULL,NULL,SW_SHOWNORMAL);
			break;
		case CMD_COPYURL:
			copy_str_clipboard(cmd_target);
			break;
		}
		break;
	case WM_CONTEXTMENU:
		if(mouse_wheel==0)
			handle_static_links(hwnd,&rmb_pos,WM_CONTEXTMENU);
		mouse_wheel=0;
		cmd_target[sizeof(cmd_target)-1]=0;
		strncpy(cmd_target,mouse_target,sizeof(cmd_target)-1);
		return 0;
		break;
	case WM_RBUTTONDOWN:
		mouse_wheel=0;
		break;
	case WM_RBUTTONUP:
		rmb_pos.y=HIWORD(lparam);
		rmb_pos.x=LOWORD(lparam);
		break;
	case WM_LBUTTONDBLCLK:
		{
		POINTL p;
		p.y=HIWORD(lparam);
		p.x=LOWORD(lparam);
		if(!handle_static_links(hwnd,&p,WM_LBUTTONDBLCLK))
			goto chan_modes;
		}
		break;
	case WM_LBUTTONDOWN:
		lmb_down=TRUE;
		break;
	case WM_LBUTTONUP:
		lmb_down=FALSE;
		set_focus_edit(hwnd);
		break;
	case WM_APP: //release LMB and copy
		{
		CHARRANGE cr;
		HWND hedit,hparent=GetParent(hwnd);
		hedit=GetDlgItem(hparent,MDI_EDIT);
		SendMessage(hwnd,EM_EXGETSEL,0,&cr);
		if(cr.cpMax!=cr.cpMin){
			int line,pos;
			line=SendMessage(hwnd,EM_EXLINEFROMCHAR,0,cr.cpMax);
			pos=SendMessage(hwnd,EM_LINEINDEX,line,0);
			if(pos>0 && pos==cr.cpMax){
				cr.cpMax--;
				SendMessage(hwnd,EM_EXSETSEL,0,&cr);
			}
			SendMessage(hwnd,WM_COPY,0,0);
		}
		SendMessage(hwnd,EM_SETSEL,-1,0);
		SetFocus(hedit);
		}
		return 0;
	case WM_APP+1:
		set_scroll_lock(GetParent(hwnd),LOWORD(wparam));
		return 0;
		break;
	case WM_CAPTURECHANGED:
		break;

	}
    return CallWindowProc(orig_static_proc,hwnd,msg,wparam,lparam); 
}
int set_scroll_lock(HWND hwnd,int scroll_code)
{
	IRC_WINDOW *win=find_window_by_hwnd(hwnd);
	if(win!=0){
		RECT rectc={0},rectw={0};
		int scroll_on=FALSE;
		GetClientRect(win->hstatic,&rectc);
		GetWindowRect(win->hstatic,&rectw);
		if((rectw.right-rectw.left-rectc.right)>=(GetSystemMetrics(SM_CXVSCROLL)/2))
			scroll_on=TRUE;
		if(scroll_on==FALSE){
			ShowWindow(win->hscroll_lock,SW_HIDE);
			win->scroll_free=FALSE;
		}
		else{
			int scroll_lock=TRUE;
			switch(scroll_code){
			case SB_LINEUP:
			case SB_PAGEUP:
			case SB_TOP:
				scroll_lock=FALSE;
				break;
			case SB_BOTTOM:
				break;
			case SB_ENDSCROLL:
			default:
				{
					SCROLLINFO si={0};
					int pos=0;
					si.cbSize=sizeof(si);
					si.fMask=SIF_ALL;
					GetScrollInfo(win->hstatic,SB_VERT,&si);
					if(scroll_code==SB_THUMBPOSITION || scroll_code==SB_THUMBTRACK)
						pos=si.nTrackPos;
					else
						pos=si.nPos;
					if((pos+si.nPage) >= (si.nMax-1))
						scroll_lock=TRUE;
					else
						scroll_lock=FALSE;
					//printf("scrollcode %i\n",scroll_code);
					//printf("diff %i %i\n",si.nTrackPos+si.nPage,si.nMax);
					//printf("scroll:%i %i %i %i (%i) %i\n",si.nTrackPos,si.nMin,si.nMax,si.nPage,si.nMax-si.nPage,si.nPos);
				}
				break;
			}
			if(scroll_lock){
				ShowWindow(win->hscroll_lock,SW_SHOW);
				win->scroll_free=FALSE;
			}else{
				ShowWindow(win->hscroll_lock,SW_HIDE);
				win->scroll_free=TRUE;
			}
		}
	}
	return TRUE;
}
int dump_scroll_info(HWND hwnd)
{
	SCROLLINFO si={0};
	si.cbSize=sizeof(si);
	si.fMask=SIF_ALL;
	GetScrollInfo(hwnd,SB_VERT,&si);
	printf("dumpscroll:%i %i %i %i (%i) %i\n",si.nTrackPos,si.nMin,si.nMax,si.nPage,si.nMax-si.nPage,si.nPos);
	return TRUE;
}
int set_focus_edit(HWND hstatic)
{
	HWND hparent=GetParent(hstatic);
	SetFocus(GetDlgItem(hparent,MDI_EDIT));
	return TRUE;
}
int copy_str_clipboard(char *str)
{
	int len,result=FALSE;
	HGLOBAL hmem;
	char *lock;
	len=strlen(str);
	if(len==0)
		return result;
	len++;
	hmem=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,len);
	if(hmem!=0){
		lock=GlobalLock(hmem);
		if(lock!=0){
			memcpy(lock,str,len);
			GlobalUnlock(hmem);
			if(OpenClipboard(NULL)!=0){
				EmptyClipboard();
				SetClipboardData(CF_TEXT,hmem);
				CloseClipboard();
				result=TRUE;
			}
		}
		if(!result)
			GlobalFree(hmem);
	}
	return result;
}
int find_nick_in_list(HWND hlist,char *nick)
{
	int ret;
	char n2[20]={0};
	_snprintf(n2,sizeof(n2),"@%s",nick);
	ret=SendMessage(hlist,LB_FINDSTRINGEXACT,-1,n2);
	if(ret==LB_ERR){
		_snprintf(n2,sizeof(n2),"+%s",nick);
		ret=SendMessage(hlist,LB_FINDSTRINGEXACT,-1,n2);
		if(ret==LB_ERR){
			_snprintf(n2,sizeof(n2),"%s",nick);
			ret=SendMessage(hlist,LB_FINDSTRINGEXACT,-1,n2);
		}
	}
	return ret;
}
int handle_lua_web_click(HWND hwnd,char *url)
{
	extern int lua_script_enable;
	IRC_WINDOW *win;
	int result=FALSE;
	win=find_window_by_hwnd(hwnd);
	if(win!=0){
		if(lua_script_enable && is_lua_active(win->session)){
			char *params[2]={win->channel,0};
			char str[1024]={0};
			char tmp[4]={0};
			int shift,ctrl;
			shift=GetKeyState(VK_SHIFT)&0x8000;
			ctrl=GetKeyState(VK_CONTROL)&0x8000;
			_snprintf(tmp,sizeof(tmp),"%s%s%s",shift||ctrl?":":"",shift?"S":"",ctrl?"C":"");
			tmp[sizeof(tmp)-1]=0;
			_snprintf(str,sizeof(str),"%s%s=%s","URL_CLICK",tmp,url);
			str[sizeof(str)-1]=0;
			params[1]=str;
			if(0<lua_process_event(win->session,"USER_CALLED",win->nick,&params,sizeof(params)/sizeof(char*)))
				result=TRUE;
		}
	}
	return result;
}
int handle_nick_links(IRC_WINDOW *win,unsigned char *nick,int mouse_button)
{
	int i,index=0,len,ret,cursor_set=FALSE;
	char n[20]={0};
	if(win->hlist==0)
		return cursor_set;
	len=strlen(nick);
//	if(nick[0]!='<' || nick[len-1]!='>')
//		return cursor_set;
	for(i=0;i<len;i++){
		if(nick[i]=='<')
			;
		else if(nick[i]=='>')
			break;
		else
			n[index++]=nick[i];
		if(index>=sizeof(n)-1)
			break;
	}
	n[index++]=0;
	ret=find_nick_in_list(win->hlist,n);
	/*
	_snprintf(n2,sizeof(n2),"%s",n);
	ret=SendMessage(win->hlist,LB_FINDSTRINGEXACT,-1,n2);
	if(ret==LB_ERR){
		_snprintf(n2,sizeof(n2),"@%s",n);
		ret=SendMessage(win->hlist,LB_FINDSTRINGEXACT,-1,n2);
		if(ret==LB_ERR){
			_snprintf(n2,sizeof(n2),"+%s",n);
			ret=SendMessage(win->hlist,LB_FINDSTRINGEXACT,-1,n2);
		}
	}
	*/
	if(ret!=LB_ERR){
		char str[256];
		POINT screen={0};
		SetCursor(LoadCursor(NULL,IDC_HAND));
		cursor_set=TRUE;
		switch(mouse_button){
		case CMD_PRIV_MSG:
		case WM_LBUTTONDBLCLK:
			initiate_privmsg(win->hwnd,n);
			break;
		case WM_CONTEXTMENU:
			GetCursorPos(&screen);
			TrackPopupMenu(list_menu,TPM_LEFTALIGN,screen.x,screen.y,0,win->hstatic,NULL);
			break;
		case CMD_CHANGE_NICK:
			if(show_user_input(win->hwnd,"enter new nick",n,sizeof(n))==IDOK)
				irc_cmd_nick(win->session,n);
			break;
		case CMD_WHOIS:
			if(win->session!=0)
				irc_cmd_whois(win->session,n);
			break;
		case CMD_SLAP:
			if(win->session!=0)
				irc_slap(win->session,win->channel,n);
			break;
		case CMD_OP:
			_snprintf(str,sizeof(str),"+o %s",n);
			if(win->session!=0)
				irc_cmd_channel_mode(win->session,win->channel,str);
			break;
		case CMD_DEOP:
			_snprintf(str,sizeof(str),"-o %s",n);
			if(win->session!=0)
				irc_cmd_channel_mode(win->session,win->channel,str);
			break;
		case CMD_DEVOICE:
			_snprintf(str,sizeof(str),"-v %s",n);
			if(win->session!=0)
				irc_cmd_channel_mode(win->session,win->channel,str);
			break;
		case CMD_VOICE:
			_snprintf(str,sizeof(str),"+v %s",n);
			if(win->session!=0)
				irc_cmd_channel_mode(win->session,win->channel,str);
			break;
		case CMD_KICK:
			if(win->session==0)
				break;
			irc_send_raw(win->session,"KICK %s %s kick-it",win->channel,n);
			SendMessage(win->hlist,LB_RESETCONTENT,0,0);
			irc_cmd_names(win->session,win->channel);
			break;
		case CMD_REFRESH_LIST:
			if(win->session!=0){
				SendMessage(win->hlist,LB_RESETCONTENT,0,0);
				irc_cmd_names(win->session,win->channel);
			}
			break;
		}
	}
	return cursor_set;
}
int handle_static_links(HWND hwnd,POINTL *p,int mouse_button)
{
	int pos,line,len,offset,cpy,cursor_set=FALSE;
	unsigned char str[512];
	POINT pt;
	pos=SendMessage(hwnd,EM_CHARFROMPOS,0,p);
	SendMessage(hwnd,EM_POSFROMCHAR,&pt,pos);
	line=SendMessage(hwnd,EM_EXLINEFROMCHAR,0,pos);
	len=SendMessage(hwnd,EM_LINELENGTH,pos,0);
	if(len>0 && p->y<(pt.y+20)){
		offset=SendMessage(hwnd,EM_LINEINDEX,line,0);
		offset=pos-offset;
		str[0]=sizeof(str)-1;
		str[1]=(sizeof(str)-1)>>8;
		str[sizeof(str)-1]=0;
		cpy=SendMessage(hwnd,EM_GETLINE,line,str);
		if(cpy>0 && cpy<sizeof(str)){
			unsigned char *start=0;
			int i,found=FALSE;
			mouse_target[0]=0;
			for(i=0;i<=offset;i++){
				if(str[offset-i]<=' '){
					if(i==0){
						found=FALSE;
						break;
					}
					if(found)
						break;
				}
				else{
					found=TRUE;
					start=str+offset-i;
				}
			}
			if(start!=0){
				unsigned char *match=0;
				int multi=TRUE,start_len,mt_len;
				start_len=strlen(start);
				for(i=0;i<start_len;i++){
					if(start[i]=='\r' || start[i]=='\n'){
						multi=FALSE;
						break;
					}
					else if(start[i]<=' '){
						multi=FALSE;
						start[i]=0;
						break;
					}
				}
				strncpy(mouse_target,start,sizeof(mouse_target));
				mouse_target[sizeof(mouse_target)-1]=0;
				mt_len=strlen(mouse_target);
				for(i=0;i<mt_len;i++){
					if(mouse_target[i]<=' '){
						mouse_target[i]=0;
						break;
					}
				}
				if(multi)
				for(i=1;i<20;i++){
					int j,end=FALSE,slen;
					str[0]=sizeof(str)-1;
					str[1]=(sizeof(str)-1)>>8;
					cpy=SendMessage(hwnd,EM_GETLINE,line+i,str);
					str[cpy]=0;
					slen=strlen(str);
					for(j=0;j<slen;j++){
						if(str[j]<=' '){
							str[j]=0;
							end=TRUE;
							break;
						}
					}
					_snprintf(mouse_target,sizeof(mouse_target)-1,"%s%s",mouse_target,str);
					if(end)
						break;
				}
				//printf("mouse_target=%s\n",mouse_target);
				match=_strnicmp(mouse_target,"http:",sizeof("http:")-1);
				if(match!=0)
					match=_strnicmp(mouse_target,"https:",sizeof("https:")-1);
				if(match!=0)
					match=_strnicmp(mouse_target,"www.",sizeof("www.")-1);
				if(match==0){
					SetCursor(LoadCursor(NULL,IDC_HAND));
					cursor_set=TRUE;
					if(mouse_button==WM_LBUTTONDBLCLK){
						if(!handle_lua_web_click(GetParent(hwnd),mouse_target))
							ShellExecute(NULL,"open",mouse_target,NULL,NULL,SW_SHOWNORMAL);
					}
					else if(mouse_button==WM_CONTEXTMENU){
						POINT screen={0};
						GetCursorPos(&screen);
						TrackPopupMenu(url_menu,TPM_LEFTALIGN,screen.x,screen.y,0,hwnd,NULL);
					}
				}
				else{
					IRC_WINDOW *win=find_window_by_hwnd(GetParent(hwnd));
					if(win!=0){
						cursor_set=handle_nick_links(win,mouse_target,mouse_button);
						if(!cursor_set){
							if(mouse_target[0]=='#' || mouse_target[1]=='#'){
								if(mouse_button==WM_LBUTTONDBLCLK){
									char *chan=mouse_target;
									if(mouse_target[1]=='#')
										chan=mouse_target+1;
									if(irc_is_connected(win->session))
										irc_cmd_join(win->session,chan,"");
								}
								SetCursor(LoadCursor(NULL,IDC_HAND));
								cursor_set=TRUE;								
							}
						}
					}
				}
			}
		}
	}
	if((!cursor_set) && mouse_button==WM_CONTEXTMENU){
		POINT screen={0};
		IRC_WINDOW *win=find_window_by_hwnd(GetParent(hwnd));
		if(win!=0){
			GetCursorPos(&screen);
			TrackPopupMenu(static_menu,TPM_LEFTALIGN,screen.x,screen.y,0,win->hstatic,NULL);
		}
	}
	return cursor_set;
}

