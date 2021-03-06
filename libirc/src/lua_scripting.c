#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <libircclient.h>

int lua_script_enable=FALSE;
int lua_error_msg=0;
#define LUA_SCRIPT_NAME "script.lua"
static int lua_irc_cmd_msg(lua_State *L)
{
	void *session;
	const char *nch,*msg;
	int result=LIBIRC_ERR_INVAL;
	if(lua_gettop(L)==3){
		session=lua_touserdata(L,1);
		if(session!=NULL){
			nch=lua_tostring(L,2);
			msg=lua_tostring(L,3);
			if((nch!=NULL) && (msg!=NULL)){
				result=irc_cmd_msg(session,nch,msg);
			}
		}
	}
	lua_pushinteger(L,result);
	return 1;
}
static int lua_irc_cmd_me(lua_State *L)
{
	void *session;
	const char *nch,*msg;
	int result=LIBIRC_ERR_INVAL;
	if(lua_gettop(L)==3){
		session=lua_touserdata(L,1);
		if(session!=NULL){
			nch=lua_tostring(L,2);
			msg=lua_tostring(L,3);
			if((nch!=NULL) && (msg!=NULL)){
				result=irc_cmd_me(session,nch,msg);
			}
		}
	}
	lua_pushinteger(L,result);
	return 1;
}
static int lua_irc_send_raw(lua_State *L)
{
	void *session;
	const char *str;
	int result=LIBIRC_ERR_INVAL;
	if(lua_gettop(L)==2){
		session=lua_touserdata(L,1);
		str=lua_tostring(L,2);
		if(session && str)
			result=irc_send_raw(session,str);
	}
	lua_pushinteger(L,result);
	return 1;
}
static int lua_post_message(lua_State *L)
{
	void *session;
	const char *nch,*msg;
	int result=LIBIRC_ERR_INVAL;
	if(lua_gettop(L)==3){
		session=lua_touserdata(L,1);
		if(session!=NULL){
			nch=lua_tostring(L,2);
			msg=lua_tostring(L,3);
			if((nch!=NULL) && (msg!=NULL)){
				void *hwnd;
				hwnd=get_hwnd_by_session(session,nch);
				if(hwnd!=0){
					result=post_message(hwnd,msg);
				}
			}
		}
	}
	lua_pushinteger(L,result);
	return 1;
}
static int lua_send_privmsg(lua_State *L)
{
	void *session;
	const char *origin,*mynick,*msg;
	int type;
	int result=LIBIRC_ERR_INVAL;
	if(lua_gettop(L)==5){
		//(void *session,char *origin,char *mynick,char *msg,int type)
		session=lua_touserdata(L,1);
		origin=lua_tostring(L,2);
		mynick=lua_tostring(L,3);
		msg=lua_tostring(L,4);
		type=lua_tounsigned(L,5);
		if(session && origin && mynick && msg)
			result=privmsg_event(session,origin,mynick,msg,type);
	}
	lua_pushinteger(L,result);
	return 1;
}
static int lua_find_channel_window(lua_State *L)
{
	void *result=0;
	if(lua_gettop(L)==2){
		const void *session;
		const char *nch;
		//(void *session,char *nch)
		session=lua_touserdata(L,1);
		nch=lua_tostring(L,2);
		if(session && nch){
			result=find_channel_window(session,nch);
			if(0==result)
				result=find_privmsg_window(session,nch);
		}
	}
	lua_pushlightuserdata(L,result);
	return 1;
}
static int lua_find_network_session(lua_State *L)
{
	const char *network;
	void *result=0;
	if(lua_gettop(L)==1){
		//(char *network)
		network=lua_tostring(L,1);
		if(network){
			void *win=0;
			win=find_window_by_network(network);
			if(win)
				get_session_from_window(win,&result);
		}
	}
	lua_pushlightuserdata(L,result);
	return 1;
}
static int lua_get_session_network(lua_State *L)
{
	char network[80]={0};
	if(lua_gettop(L)==1){
		const void *session;
		session=lua_touserdata(L,1);
		if(session){
			get_session_info(session,network,sizeof(network));
		}
	}
	lua_pushstring(L,network);
	return 1;
}
static int lua_add_line_mdi_nolog(lua_State *L)
{
	const void *win;
	const char *str;
	int result=LIBIRC_ERR_INVAL;
	if(lua_gettop(L)==2){
		//(void *win,char *str)
		win=lua_touserdata(L,1);
		str=lua_tostring(L,2);
		if(win && str)
			result=add_line_mdi_nolog(win,str);
	}
	lua_pushinteger(L,result);
	return 1;
}
static int lua_add_line_mdi(lua_State *L)
{
	const void *win;
	const char *str;
	int result=LIBIRC_ERR_INVAL;
	if(lua_gettop(L)==2){
		//(void *win,char *str)
		win=lua_touserdata(L,1);
		str=lua_tostring(L,2);
		if(win && str)
			result=add_line_mdi(win,str);
	}
	lua_pushinteger(L,result);
	return 1;
}
static int lua_get_win_linecount(lua_State *L)
{
	const void *win;
	int result=LIBIRC_ERR_INVAL;
	if(lua_gettop(L)==1){
		//(void *win)
		win=lua_touserdata(L,1);
		if(win)
			result=get_win_linecount(win);
	}
	lua_pushinteger(L,result);
	return 1;
}
static int lua_get_win_line(lua_State *L)
{
	const void *win;
	int line;
	static char str[1024];
	str[0]=0;
	if(lua_gettop(L)==2){
		//(void *win,int line)
		win=lua_touserdata(L,1);
		line=lua_tointeger(L,2);
		if(win)
			get_win_line(win,line,str,sizeof(str));
	}
	lua_pushstring(L,str);
	return 1;
}
static int lua_irc_cmd_ctcp_reply(lua_State *L)
{
	void *session;
	const char *nick,*reply;
	int result=LIBIRC_ERR_INVAL;
	if(lua_gettop(L)==3){
		//(void *session,char *nick,char *reply)
		session=lua_touserdata(L,1);
		nick=lua_tostring(L,2);
		reply=lua_tostring(L,3);
		if(session && nick && reply)
			result=irc_cmd_ctcp_reply(session,nick,reply);
	}
	lua_pushinteger(L,result);
	return 1;
}
static int lua_shell_execute(lua_State *L)
{
	int result=0;
	int count=lua_gettop(L);
	if(count>=1){
		const char *file;
		const char *params=0,*dir=0;
		int showcmd=SW_SHOWNORMAL;
		//(file[,params,dir,showcmd])
		file=lua_tostring(L,1);
		if(count>=2)
			params=lua_tostring(L,2);
		if(count>=3)
			dir=lua_tostring(L,3);
		if(count>=4)
			showcmd=lua_tointeger(L,4);
		if(file){
			if(32<ShellExecute(NULL,"open",file,params,dir,showcmd))
				result=1;
		}
	}
	lua_pushinteger(L,result);
	return 1;
}
static int lua_beep(lua_State *L)
{
	int result=0;
	int count=lua_gettop(L);
	if(count>=2){
		unsigned int freq,duration;
		const unsigned int MAX_TIME=10000;
		freq=lua_tounsigned(L,1);
		duration=lua_tounsigned(L,2);
		if(duration>=MAX_TIME)
			duration=MAX_TIME;
		Beep(freq,duration);
		result=1;
	}
	lua_pushinteger(L,result);
	return 1;
}
static int lua_flash_window(lua_State *L)
{
	extern HWND ghmainframe;
	int result=0;
	int count=lua_gettop(L);
	if(count>=1){
		unsigned int i,count;
		count=lua_tounsigned(L,1);
		if(count>5)
			count=5;
		for(i=0;i<count;i++){
			FlashWindow(ghmainframe,1);
			Sleep(500);
			FlashWindow(ghmainframe,0);
			Sleep(500);
		}
		result=1;
	}
	lua_pushinteger(L,result);
	return 1;
}
static int lua_bring_window_top(lua_State *L)
{
	extern HWND ghmainframe;
	int result=1;
	SetForegroundWindow(ghmainframe);
	lua_pushinteger(L,result);
	return 1;
}
static int lua_highlight_button(lua_State *L)
{
	int result=LIBIRC_ERR_INVAL;
	if(lua_gettop(L)==1){
		const void *win;
		//(void *win)
		win=lua_touserdata(L,1);
		if(win)
			result=highlight_button_text(win);
	}
	lua_pushinteger(L,result);
	return 1;
}
typedef struct{
	char *lua_name;
	int(*lua_func)(lua_State *L);
	char *descrip;
}LUA_C_FUNC_MAP;
LUA_C_FUNC_MAP lua_map[]={
	{"irc_cmd_msg",lua_irc_cmd_msg,"(session,nch,msg)"},
	{"irc_cmd_me",lua_irc_cmd_me,"(session,nch,msg)"},
	{"irc_send_raw",lua_irc_send_raw,"(session,str)"},
	{"irc_cmd_ctcp_reply",lua_irc_cmd_ctcp_reply,"(session,nick,reply)"},
	{"post_message",lua_post_message,"(session,nch,msg)"},
	{"send_privmsg",lua_send_privmsg,"(session,origin,mynick,msg,type)"},
	{"find_channel_window",lua_find_channel_window,"(session,channel)"},
	{"find_network_session",lua_find_network_session,"(network)"},
	{"get_session_network",lua_get_session_network,"(session)"},
	{"add_line_mdi",lua_add_line_mdi,"(win,str)"},
	{"add_line_mdi_nolog",lua_add_line_mdi_nolog,"(win,str)"},
	{"get_win_linecount",lua_get_win_linecount,"(win)"},
	{"get_win_line",lua_get_win_line,"(win,line)"},
	{"shell_execute",lua_shell_execute,"(file[,params,dir,showcmd])"},
	{"beep",lua_beep,"(freq,duration)"},
	{"flash_window",lua_flash_window,"(count)"},
	{"bring_win_top",lua_bring_window_top,""},
	{"highlight_button",lua_highlight_button,"(win)"},
	0
};
int lua_register_c_functions(lua_State *L)
{
	int i;
	for(i=0;i<sizeof(lua_map)/sizeof(LUA_C_FUNC_MAP);i++){
		if(lua_map[i].lua_name==0)
			break;
		lua_register(L,lua_map[i].lua_name,lua_map[i].lua_func);
	}
	return TRUE;
}



static int get_last_write_time(char *fname,__int64 *ft)
{
	int result=FALSE;
	HANDLE hf;
	hf=CreateFile(fname,0,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hf!=INVALID_HANDLE_VALUE){
		result=GetFileTime(hf,NULL,NULL,(FILETIME*)ft);
		if(result!=0)
			result=TRUE;
		CloseHandle(hf);
	}
	return result;
}
void lua_script_init(lua_State **L,HANDLE **lua_filenotify,__int64 *ft)
{
	char fscript[MAX_PATH]={0};
	int script_changed=FALSE;

	get_lua_script_fname(fscript,sizeof(fscript));

	if(lua_script_enable){
		__int64 tt=0;
		get_last_write_time(fscript,&tt);
		if(tt!=(*ft)){
			script_changed=TRUE;
			hide_tooltip();
		}
	}

	if(lua_script_enable){
		if((*L)==0 || script_changed){
			lua_State *lua;
			lua=luaL_newstate();
			if(lua!=0){
				luaL_openlibs(lua);
				if(luaL_loadfile(lua,fscript)!=LUA_OK){
					printf("luaL_loadfile error:%s\n",lua_tostring(lua, -1));
					show_tooltip(lua_tostring(lua, -1),0,0);
					lua_close(lua);
				}
				else{
					lua_register_c_functions(lua);
					if(lua_pcall(lua,0,0,0)!=LUA_OK){
						printf("lua_pcall error:%s\n",lua_tostring(lua, -1));
						show_tooltip(lua_tostring(lua, -1),0,0);
						lua_close(lua);
					}
					else{
						if((*L)!=0)
							lua_close(*L);
						*L=lua;
						get_last_write_time(fscript,ft);
						lua_error_msg=0;
						hide_tooltip();
					}
				}
			}
		}
	}
	if(*lua_filenotify!=0){
		if(FindNextChangeNotification(*lua_filenotify)==0){
			FindCloseChangeNotification(*lua_filenotify);
			*lua_filenotify=0;
		}
	}
	if(*lua_filenotify==0){
		HANDLE fn;
		char path[MAX_PATH]={0};
		get_ini_path(path,sizeof(path));
		fn=FindFirstChangeNotification(path,FALSE,FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_LAST_WRITE);
		if(fn!=INVALID_HANDLE_VALUE)
			*lua_filenotify=fn;
	}
}
void lua_script_unload(lua_State **L,HANDLE **lua_filenotify)
{
	lua_State *lua=*L;
	if(lua!=0){
		lua_close(lua);
		*L=0;
	}
	if(*lua_filenotify!=0){
		FindCloseChangeNotification(*lua_filenotify);
		*lua_filenotify=0;
	}
}
enum{CHECK_IGNORE_FUNC,STANDARD_FUNC,USER_FUNC};
typedef struct{
	char *event;
	char *lua_func;
	int type;
}LUA_FUNC_MAP;
LUA_FUNC_MAP lua_funcs[]={
	{"CHECKIGNORE","check_ignore",CHECK_IGNORE_FUNC},
	{"CTCP","ctcp_event",CHECK_IGNORE_FUNC},
	{"PRIVMSG","privmsg_event",STANDARD_FUNC},
	{"CHANNEL","channel_event",STANDARD_FUNC},
	{"POST_CONNECT","post_connect_event",STANDARD_FUNC},
	{"JOIN","join_event",STANDARD_FUNC},
	{"NUMERIC","numeric_event",STANDARD_FUNC},
	{"USER_CALLED","user_called_event",USER_FUNC},
	0
};
int lua_get_func_index(const char *event)
{
	int i;
	for(i=0;i<sizeof(lua_funcs)/sizeof(LUA_FUNC_MAP);i++){
		if(lua_funcs[i].event==0)
			break;
		if(stricmp(event,lua_funcs[i].event)==0)
			return i;
	}
	return -1;
}
int lua_handle_event(lua_State *L,
					  void *session,
					  const char *event,
					  const char *origin,
					  const char ** params,
					  unsigned int count)
{
	int index;
	int result=0;

	if(!lua_script_enable){
		lua_error_msg=0;
		return result;
	}
	if(L==0)
		return result;

	index=lua_get_func_index(event);
	if(index<0)
		return result;

	switch(lua_funcs[index].type){
	case CHECK_IGNORE_FUNC:
		lua_getglobal(L,lua_funcs[index].lua_func);
		lua_pushlightuserdata(L,session);
		lua_pushstring(L,origin);
		lua_pushstring(L,params[0]); //nick
		lua_pushstring(L,params[1]); //msg
		if(lua_pcall(L,4,1,0)==LUA_OK)
			result=lua_tointeger(L,-1);
		break;
	case STANDARD_FUNC:
		lua_getglobal(L,lua_funcs[index].lua_func);
		lua_pushlightuserdata(L,session);
		lua_pushstring(L,origin);
		lua_pushstring(L,params[0]); //nick
		lua_pushstring(L,params[1]); //msg
		if(lua_pcall(L,4,1,0)!=LUA_OK){
			if(lua_error_msg<5){
				printf("lua error:%s\n event=%s\n",lua_tostring(L, -1),event);
				lua_error_msg++;
			}
		}
		else
			result=lua_tointeger(L,-1);
		break;
	case USER_FUNC:
		lua_getglobal(L,lua_funcs[index].lua_func);
		lua_pushlightuserdata(L,session);
		lua_pushstring(L,origin);
		{
			unsigned int i;
			for(i=0;i<count;i++){
				if(i<2)
					lua_pushstring(L,params[i]); //nick,msg
				else
					lua_pushlightuserdata(L,(void*)params[i]); //window
			}
		}
		if(lua_pcall(L,2+count,1,0)!=LUA_OK){
			if(lua_error_msg<5){
				printf("lua error:%s\n event=%s\n",lua_tostring(L, -1),event);
				lua_error_msg++;
			}
		}
		else
			result=lua_tointeger(L,-1);
		break;
	}
	return result;
}

int lua_help(int(*mdi_window)(void *,char *),void *win)
{
	int i;
	if(mdi_window==0 || win==0)
		return 0;
	lua_error_msg=0;
	mdi_window(win,"\r\nlua func args:(session,origin,nch,msg)");
	for(i=0;i<sizeof(lua_funcs)/sizeof(LUA_FUNC_MAP);i++){
		char str[80];
		if(lua_funcs[i].event==0)
			break;
		_snprintf(str,sizeof(str)-1,"event:%s lua_func:%s",
			lua_funcs[i].event,lua_funcs[i].lua_func);
		str[sizeof(str)-1]=0;
		mdi_window(win,str);
	}
	mdi_window(win,"\r\n avail C functions for lua:");
	for(i=0;i<sizeof(lua_map)/sizeof(LUA_C_FUNC_MAP);i++){
		char str[80];
		if(lua_map[i].lua_name==0)
			break;
		_snprintf(str,sizeof(str)-1,"%s %s",lua_map[i].lua_name,lua_map[i].descrip);
		str[sizeof(str)-1]=0;
		mdi_window(win,str);
	}
	return 0;
}
int get_lua_script_fname(char *fname,int len)
{
	if(fname!=0 && len>0){
		fname[0]=0;
		get_ini_path(fname,len);
		_snprintf(fname,len,"%s%s",fname,LUA_SCRIPT_NAME);
	}
	return 0;
}
int lua_create_default_file(int(*mdi_window)(void *,char *),void *win)
{
	FILE *f;
	char fname[MAX_PATH]={0};
	get_lua_script_fname(fname,sizeof(fname));
	if(does_file_exist(fname)){
		if(mdi_window && win){
			mdi_window(win,"file allready exists:");
			mdi_window(win,fname);
		}
		return FALSE;
	}
	f=fopen(fname,"wb");
	if(f!=0){
		int i;
		fprintf(f,"-- external C functions available:\n");
		for(i=0;i<sizeof(lua_map)/sizeof(LUA_C_FUNC_MAP);i++){
			if(lua_map[i].lua_name==0)
				break;
			fprintf(f,"-- %s %s\n",lua_map[i].lua_name,lua_map[i].descrip);
		}
		fprintf(f,"\n\n-- lua functions that get called after certain events\n"
				" -- session=irc session pointer\n"
				" -- origin=full nick (billy!~test@example.com)\n"
				" -- nch=nick or channel\n"
				" -- msg=message body\n");
		for(i=0;i<sizeof(lua_funcs)/sizeof(LUA_FUNC_MAP);i++){
			if(lua_funcs[i].event==0)
				break;
			fprintf(f,"\n-- function %s(session,origin,nch,msg%s)\n",lua_funcs[i].lua_func,
				lua_funcs[i].type==USER_FUNC?",win":"");
			fprintf(f,"-- end\n");
		}
		fclose(f);
		if(mdi_window && win){
			mdi_window(win,"created file:");
			mdi_window(win,fname);
		}
	}
	return FALSE;
}