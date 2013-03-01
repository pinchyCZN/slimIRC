
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
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
	if(lua==0){
		lua=luaL_newstate();
		if(lua!=0){
			luaL_openlibs(lua);
			if(luaL_loadfile(lua,fscript)!=LUA_OK){
				lua_close(lua);
				*L=0;
			}
			else{
				if(lua_pcall(lua,0,0,0)!=LUA_OK){
					lua_close(lua);
					*L=0;
				}
				else{
					*L=lua;
					get_last_write_time(fscript,&ft);
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

int lua_process_event(irc_session_t *session,
					  const char *event,
					  const char *origin,
					  const char ** params,
					  unsigned int count)
{
	lua_State *L=session->lua_context;
	if(L==0)
		return TRUE;
	if(stricmp(event,"PRIVMSG")==0){
		char str[1024+20]={0};
		char *s=0;
		lua_getglobal(L,"privmsg_event");
		lua_pushstring(L,origin);
		lua_pushstring(L,params[0]); //nick
		lua_pushstring(L,params[1]); //msg
		if(lua_pcall(L,3,3,0)!=LUA_OK)
			return TRUE;
		if(lua_toboolean(L,-2)){
			s=lua_tostring(L,-1);
			if(s!=0){
				strncpy(str,s,sizeof(str));
				str[sizeof(str)-1]=0;
				printf("new string:%s\n",str);
			}
		}
		if(lua_isboolean(L,-3)){
			return lua_toboolean(L,-3);
		}
	}
	else if(stricmp(event,"CHANNEL")==0){
		char str[1024+20]={0};
		char *s=0;
		lua_getglobal(L,"channel_event");
		lua_pushstring(L,origin);
		lua_pushstring(L,params[0]); //nick
		lua_pushstring(L,params[1]); //msg
		if(lua_pcall(L,3,3,0)!=LUA_OK)
			return TRUE;
		if(lua_toboolean(L,-2)){
			s=lua_tostring(L,-1);
			if(s!=0){
				strncpy(str,s,sizeof(str));
				str[sizeof(str)-1]=0;
				printf("new string:%s\n",str);
			}
		}
		if(lua_isboolean(L,-3)){
			return lua_toboolean(L,-3);
		}
	}
	return TRUE;
}
