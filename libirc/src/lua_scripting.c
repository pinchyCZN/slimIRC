#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <libircclient.h>

int lua_script_enable=TRUE;

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
int lua_register_c_functions(lua_State *L)
{
	lua_register(L,"irc_cmd_msg",lua_irc_cmd_msg);
	lua_register(L,"irc_cmd_me",lua_irc_cmd_me);
	lua_register(L,"post_message",lua_post_message);
	lua_register(L,"send_privmsg",lua_send_privmsg);
	lua_register(L,"irc_send_raw",lua_irc_send_raw);
	return TRUE;
}



static int get_last_write_time(char *fname,FILETIME *ft)
{
	int result=FALSE;
	HANDLE hf;
	hf=CreateFile(fname,0,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hf!=INVALID_HANDLE_VALUE){
		result=GetFileTime(hf,NULL,NULL,ft);
		if(result!=0)
			result=TRUE;
		CloseHandle(hf);
	}
	return result;
}
void lua_script_init(lua_State **L,HANDLE **lua_filenotify)
{
	extern char ini_file[];
	lua_State *lua;
	char path[MAX_PATH]={0};
	char fscript[MAX_PATH];
	static FILETIME ft={0,0};

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
		FILETIME tt={0,0};
		get_last_write_time(fscript,&tt);
		if((tt.dwHighDateTime!=ft.dwHighDateTime)||(tt.dwLowDateTime!=ft.dwLowDateTime)){
			ft=tt;
			lua_close(lua);
			lua=*L=0;
		}
	}
	if(lua==0 && lua_script_enable){
		lua=luaL_newstate();
		if(lua!=0){
			luaL_openlibs(lua);
			if(luaL_loadfile(lua,fscript)!=LUA_OK){
				printf("error loading script %i\n",rand());
				lua_close(lua);
				*L=0;
			}
			else{
				lua_register_c_functions(lua);
				if(lua_pcall(lua,0,0,0)!=LUA_OK){
					lua_close(lua);
					*L=0;
				}
				else{
					*L=lua;
					get_last_write_time(fscript,&ft);
					printf("script loaded ok\n");
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
int lua_handle_event(lua_State *L,
					  void *session,
					  const char *event,
					  const char *origin,
					  const char ** params,
					  unsigned int count)
{
	if(L==0)
		return TRUE;
	if(!lua_script_enable)
		return TRUE;
	if(stricmp(event,"CHECKIGNORE")==0){
		lua_getglobal(L,"check_ignore");
		lua_pushlightuserdata(L,session);
		lua_pushstring(L,origin);
		lua_pushstring(L,params[0]); //nick
		lua_pushstring(L,params[1]); //msg
		if(lua_pcall(L,4,1,0)!=LUA_OK)
			return FALSE;
		return lua_tointeger(L,-1);
	}
	else if(stricmp(event,"PRIVMSG")==0){
		lua_getglobal(L,"privmsg_event");
		lua_pushlightuserdata(L,session);
		lua_pushstring(L,origin);
		lua_pushstring(L,params[0]); //nick
		lua_pushstring(L,params[1]); //msg
		if(lua_pcall(L,4,1,0)!=LUA_OK)
			return TRUE;
		return lua_tointeger(L,-1);
	}
	else if(stricmp(event,"CHANNEL")==0){
		lua_getglobal(L,"channel_event");
		lua_pushlightuserdata(L,session);
		lua_pushstring(L,origin);
		lua_pushstring(L,params[0]); //nick
		lua_pushstring(L,params[1]); //msg
		if(lua_pcall(L,4,1,0)!=LUA_OK)
			return TRUE;
		return lua_toboolean(L,-1);
	}
	else if(stricmp(event,"POST_CONNECT")==0){
		lua_getglobal(L,"post_connect_event");
		lua_pushlightuserdata(L,session);
		lua_pushstring(L,origin);
		lua_pushstring(L,params[0]);
		lua_pushstring(L,params[1]);
		if(lua_pcall(L,4,1,0)!=LUA_OK)
			return TRUE;
		return lua_toboolean(L,-1);
	}
	return TRUE;
}
