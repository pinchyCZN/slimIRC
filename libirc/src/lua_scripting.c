#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <libircclient.h>

int lua_script_enable=TRUE;
int lua_error_msg=0;

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
typedef struct{
	char *lua_name;
	int(*lua_func)(lua_State *L);
}LUA_C_FUNC_MAP;
LUA_C_FUNC_MAP lua_map[]={
	{"irc_cmd_msg",lua_irc_cmd_msg},
	{"irc_cmd_me",lua_irc_cmd_me},
	{"irc_send_raw",lua_irc_send_raw},
	{"post_message",lua_post_message},
	{"send_privmsg",lua_send_privmsg},
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
	extern char ini_file[];
	lua_State *lua;
	char path[MAX_PATH]={0};
	char fscript[MAX_PATH];

	if(ini_file[0]==0)
		GetCurrentDirectory(sizeof(path),path);
	else
		strncpy(path,ini_file,sizeof(path));
	path[sizeof(path)-1]=0;
	if(path[0]!=0){
		int i;
		for(i=strlen(path);i>0;i--){
			if(path[i]=='\\'){
				path[i+1]=0;
				break;
			}
		}
	}
	else
		return;

	_snprintf(fscript,sizeof(fscript),"%s%s",path,"script.lua");

	lua=*L;
	if(lua!=0){
		__int64 tt=0;
		get_last_write_time(fscript,&tt);
		if(tt!=(*ft)){
			*ft=tt;
			lua_close(lua);
			printf("lua close\n");
			lua=*L=0;
		}
	}
	if(lua==0 && lua_script_enable){
		lua=luaL_newstate();
		if(lua!=0){
			luaL_openlibs(lua);
			if(luaL_loadfile(lua,fscript)!=LUA_OK){
				printf("luaL_loadfile error:%s\n",lua_tostring(lua, -1));
				lua_close(lua);
				*L=0;
			}
			else{
				lua_register_c_functions(lua);
				if(lua_pcall(lua,0,0,0)!=LUA_OK){
					printf("lua_pcall error:%s\n",lua_tostring(lua, -1));
					lua_close(lua);
					*L=0;
				}
				else{
					*L=lua;
					get_last_write_time(fscript,ft);
					lua_error_msg=0;
				}
			}
		}
		else
			*L=0;
	}
	if(*lua_filenotify!=0){
		if(FindNextChangeNotification(*lua_filenotify)==0){
			FindCloseChangeNotification(*lua_filenotify);
			*lua_filenotify=0;
		}
	}
	if(*lua_filenotify==0){
		HANDLE fn;
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
enum{CHECK_IGNORE_FUNC,STANDARD_FUNC,NUMERIC_FUNC};
typedef struct{
	char *event;
	char *lua_func;
	int type;
}LUA_FUNC_MAP;
LUA_FUNC_MAP lua_funcs[]={
	{"CHECKIGNORE","check_ignore",CHECK_IGNORE_FUNC},
	{"PRIVMSG","privmsg_event",STANDARD_FUNC},
	{"CHANNEL","channel_event",STANDARD_FUNC},
	{"POST_CONNECT","post_connect_event",STANDARD_FUNC},
	{"JOIN","join_event",STANDARD_FUNC},
	{"NUMERIC","numeric_event",NUMERIC_FUNC},
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
	if(L==0)
		return TRUE;
	if(!lua_script_enable)
		return TRUE;
	index=lua_get_func_index(event);

	if(index<0)
		return TRUE;

	switch(lua_funcs[index].type){
	default:
		return TRUE;
	case CHECK_IGNORE_FUNC:
		lua_getglobal(L,lua_funcs[index].lua_func);
		lua_pushlightuserdata(L,session);
		lua_pushstring(L,origin);
		lua_pushstring(L,params[0]); //nick
		lua_pushstring(L,params[1]); //msg
		if(lua_pcall(L,4,1,0)!=LUA_OK)
			return FALSE;
		return lua_tointeger(L,-1);
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
			return TRUE;
		}
		return lua_tointeger(L,-1);
		break;
	case NUMERIC_FUNC:
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
			return TRUE;
		}
		return lua_tointeger(L,-1);
		break;
	}
	return TRUE;
}
