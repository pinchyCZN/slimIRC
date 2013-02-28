#include <windows.h>
#include <stdio.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

void lua_script_init(lua_State **L,HANDLE **lua_filenotify)
{
	extern char ini_file[];
	lua_State *lua;
	char path[MAX_PATH]={0};
	lua=*L;
	if(lua!=0){
		lua_close(lua);
		*L=0;
	}
	lua=luaL_newstate();
	luaL_openlibs(lua);
	if(luaL_loadfile(L,"script.lua")){
		lua_close(lua);
		return;
	}
	*L=lua;

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
		//ReadDirectoryChangesW()
		//*lua_filenotify=FindFirstChangeNotification(path,FALSE,
	}

}
void lua_script_unload(lua_State **L)
{
	lua_State *lua=*L;
	if(lua!=0){
		lua_close(lua);
		*L=0;
	}
}

int lua_process_event(void *session,
					  lua_State *L,
					  const char *event,
					  const char *origin,
					  const char ** params,
					  unsigned int count)
{
	if(L==0)
		return TRUE;
	if(stricmp(event,"PRIVMSG")==0){
		lua_getglobal(L,"privmsg_event");
	}
	return TRUE;
}
static int bail()
{
}

int luatest()
{
	int i,j;
   lua_State *L;

    L = luaL_newstate();                        /* Create Lua state variable */
    luaL_openlibs(L);                           /* Load Lua libraries */

    if (luaL_loadfile(L, "test.lua")) /* Load but don't run the Lua script */
	bail(L, "luaL_loadfile() failed");      /* Error out if file can't be read */

    /* ABOVE HERE IS HELLO WORLD CODE */

    if (lua_pcall(L, 0, 0, 0))                  /* PRIMING RUN. FORGET THIS AND YOU'RE TOAST */
	bail(L, "lua_pcall() failed");          /* Error out if Lua file has an error */


    lua_getglobal(L, "tellme");                 /* Tell what function to run */

    /* BELOW HERE IS THE HELLO WORLD CODE */
    printf("In C, calling Lua\n");

	if (lua_pcall(L, 0, 0, 0))                  /* Run the function */
		bail(L, "lua_pcall() failed");          /* Error out if Lua file has an error */

    printf("Back in C again\npress key to continue");
	getch();


	for(i=0;i<90000;i++){
		lua_getglobal(L, "square");                 /* Tell it to run callfuncscript.lua->square() */
		lua_pushnumber(L, 6);                       /* Submit 6 as the argument to square() */
		if (lua_pcall(L, 1, 1, 0))                  /* Run function, !!! NRETURN=1 !!! */
			bail(L, "lua_pcall() failed");

		j = lua_tonumber(L, -1);
		printf("Returned number=%d\n", j);
	}
	printf("press key\n");
	getch();
    lua_close(L);                               /* Clean up, free the Lua state var */

	printf("mem freed press key\n");
	getch();

    return 0;
}