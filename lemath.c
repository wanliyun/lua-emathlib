#ifndef SLUA_3RD_LOADED
#include <lua.h>
#include <lauxlib.h>
#endif

#include "sincostan_lut.h"

/*fast;low accuracy*/
/* return degree */
static int emath_atand (lua_State *L) {
	lua_Number v = luaL_checknumber(L, 1);
	lua_Number rad = emath_atan_fast(v);
	lua_pushnumber(L, emath_rad2deg(rad));
	return 1;
}


/*high accuracy*/
/*return degree */
static int emath_atandx (lua_State *L) { 
	lua_Number v = luaL_checknumber(L, 1);
	lua_Number rad = emath_atan_double(v);
	lua_pushnumber(L, emath_rad2deg(rad));
	return 1;
}

/* input degree */
static int emath_sind (lua_State *L) {
	lua_Number deg = luaL_checknumber(L, 1);
	lua_Number ret = emath_cos_tablelookup(90 - deg);
	lua_pushnumber(L, ret);
	return 1;
}

/* input degree */
static int emath_cosd (lua_State *L) {
	lua_Number deg = luaL_checknumber(L, 1);
	lua_Number ret = emath_cos_tablelookup(deg);
	lua_pushnumber(L, ret);
	return 1;
}

/* input degree */
static int emath_tand (lua_State *L) {
	lua_Number deg = luaL_checknumber(L, 1);
	lua_Number sinV = emath_cos_tablelookup(90 - deg);
	lua_Number cosV = emath_cos_tablelookup(deg);
	if(cosV == 0)
	{
		/*keep same with math.tan(math.pi/2)*/
		if(sinV > 0)
			lua_pushnumber(L, (lua_Number)1.6331239353195e+16); /*TODO: */
		else
			lua_pushnumber(L, (lua_Number)-1.6331239353195e+16); /*TODO: */
	}
	else
	{
		lua_pushnumber(L, sinV / cosV);
	}
	return 1;
}

int luaopen_lemath(lua_State* L)
{
	luaL_Reg lfuncs[] = {
		/*
		{"acosd",  math_acos},
		{"asind",  math_asin},
		*/
		{"atand",  emath_atand},
		{"atandx", emath_atandx},
		{"cosd",   emath_cosd},
		{"sind",   emath_sind},
		{"tand",   emath_tand},
		{NULL, NULL},
	};
	luaL_newlib(L, lfuncs);
	return 1;
}
