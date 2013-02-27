#include <windows.h>
#include <stdio.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

void lua_script_init(lua_State **L)
{
	lua_State *lua;
	char path[MAX_PATH];
	lua=*L;
	if(lua!=0)
		lua_close(lua);
	lua=luaL_newstate();
	luaL_openlibs(lua);
	*L=lua;
}
void lua_script_unload(lua_State **L)
{
	lua_State *lua=*L;
	if(lua!=0)
		lua_close(lua);
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