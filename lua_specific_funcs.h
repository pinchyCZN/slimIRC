int get_win_linecount(IRC_WINDOW *win)
{
	int result=0;
	if(win!=0){
		result=SendMessage(win->hstatic,EM_GETLINECOUNT,0,0);
	}
	return result;
}
int get_win_line(IRC_WINDOW *win,int line,char *str,int len)
{
	int result=0;
	if(win!=0){
		if(len>3){
			str[0]=len-1;
			str[1]=(len-1)>>8;
			result=SendMessage(win->hstatic,EM_GETLINE,line,str);
			if(result>0 && result<=len)
				str[result-1]=0;
			else
				str[0]=0;
		}
	}
	return result;
}
int get_session_from_window(IRC_WINDOW *win,void **session)
{
	if(win && session){
		*session=win->session;
		return TRUE;
	}
	return FALSE;
}