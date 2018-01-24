// Use Visual Studio 2017
// Next steps: implement OpenCV

// Will binding like LuaBridge be needed? Maybe not since almost everything will be handled in C++, just need Lua for emulator

// New method: http://www.troubleshooters.com/codecorn/lua/lua_lua_calls_c.htm


#include <windows.h>

extern "C" // must wrap it around extern "C" as it is c code, also must get rid of lua.c since it has main function
{
#include "src/lua.h"                               /* Always include this */
#include "src/lauxlib.h"                           /* Always include this */
#include "src/lualib.h"                            /* Always include this */
}

static int isquare(lua_State *L) {              /* Internal name of func */
	float rtrn = lua_tonumber(L, -1);      /* Get the single number arg */
	printf("Top of square(), nbr=%f\n", rtrn);
	lua_pushnumber(L, rtrn*rtrn);           /* Push the return */
	int result = MessageBox(NULL, "TEST", "TEST", MB_OK); // THIS WORKS
	return 1;                              /* One return value */
}
static int icube(lua_State *L) {                /* Internal name of func */
	float rtrn = lua_tonumber(L, -1);      /* Get the single number arg */
	printf("Top of cube(), number=%f\n", rtrn);
	lua_pushnumber(L, rtrn*rtrn*rtrn);      /* Push the return */
	return 1;                              /* One return value */
}


/* Register this file's functions with the
* luaopen_libraryname() function, where libraryname
* is the name of the compiled .so (or any library like .dll) output. In other words
* it's the filename (but not extension) after the -o
* in the cc command.
*
* So for instance, if your cc command has -o power.so then
* this function would be called luaopen_power().
*
* This function should contain lua_register() commands for
* each function you want available from Lua.
*
*/

// extern "C" needed to make sure function does not get mangled, 
// __declspec(dllexport) needed to expose function
extern "C" int __declspec(dllexport) luaopen_MarioCVPlugin(lua_State *L) {
	lua_register(
		L,               /* Lua state variable */
		"square",        /* func name as known in Lua */
		isquare          /* func name in this file */
	);
	lua_register(L, "cube", icube);
	return 0;
}
/*
// Old method that didn't work: http://lua-users.org/wiki/CreatingBinaryExtensionModules
#include <windows.h>

extern "C" // must wrap it around extern "C" as it is c code, also must get rid of lua.c since it has main function
{
#include "src/lauxlib.h"
}

//Pop-up a Windows message box with your choice of message and caption 
int lua_msgbox(lua_State* L)
{
	const char* message = luaL_checkstring(L, 1);
	const char* caption = luaL_optstring(L, 2, "");
	int result = MessageBox(NULL, message, caption, MB_OK);
	lua_pushnumber(L, result);
	return 1;
}

extern "C" int __declspec(dllexport) libinit(lua_State* L)
{
	lua_register(L, "msgbox", lua_msgbox);
	return 0;
}*/

// Extend to Mario Party?