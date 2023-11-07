#ifndef SLUA_3RD_LOADED
#include <lua.h>
#include <lauxlib.h>
#include <stdlib.h>
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


static lua_Number SqrDistPointSegment(lua_Number ax, lua_Number ay, lua_Number bx, lua_Number by, lua_Number px, lua_Number py)
{
	//Vector3 ab = b - a;
	//Vector3 ax = x - a;
	//Vector3 bx = x - b;
	lua_Number abx, aby, apx, apy, bpx, bpy;
	abx = bx - ax;
	aby = by - ay;

	apx = px - ax;
	apy = py - ay;

	bpx = px - bx;
	bpy = py - by;


	//float e = Vector3.Dot(ax, ab);
	//if (e <= 0.0f)
	//	return ax.sqrMagnitude;
	lua_Number e = apx * abx + apy * aby;
	//printf("e=%f\n", e);
	if (e <= 0.0f)
		return apx * apx + apy * apy; 

	//float f = ab.sqrMagnitude;
	//if (e >= f)
	//	return bx.sqrMagnitude;
	lua_Number f = abx * abx + aby * aby;
	//printf("f=%f\n", f);
	if(e>=f)
		return bpx * bpx + bpy * bpy;

	//return ax.sqrMagnitude - e * e / f;
	return (apx * apx + apy * apy) - e;
}

static int emath_sqrdist_point_2_segment(lua_State *L) {
    lua_Number ax = luaL_checknumber(L, 1);
    lua_Number ay = luaL_checknumber(L, 2);
    lua_Number bx = luaL_checknumber(L, 3);
    lua_Number by = luaL_checknumber(L, 4);
    lua_Number px = luaL_checknumber(L, 5);
    lua_Number py = luaL_checknumber(L, 6);
	lua_Number ret = SqrDistPointSegment(ax, ay, bx, by, px, py);
    lua_pushnumber(L, ret);
	return 1;
}

static int circles_intersect(lua_Number x1, lua_Number y1, lua_Number r1, lua_Number x2, lua_Number y2, lua_Number r2) {
    lua_Number sqrDistance = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
	lua_Number rDist = (r1 + r2);
    if (sqrDistance <= rDist * rDist) {
        return 1; // Circles intersect
    } else {
        return 0; // Circles do not intersect
    }
}
/* check circle intersect */
static int emath_intersect_cricle_circle(lua_State *L) {
    lua_Number x1 = luaL_checknumber(L, 1);
    lua_Number y1 = luaL_checknumber(L, 2);
    lua_Number r1 = luaL_checknumber(L, 3);
    lua_Number x2 = luaL_checknumber(L, 4);
    lua_Number y2 = luaL_checknumber(L, 5);
    lua_Number r2 = luaL_checknumber(L, 6);
    int result = circles_intersect(x1, y1, r1, x2, y2, r2);
    lua_pushboolean(L, result);
	return 1;
}

static lua_Number my_max(lua_Number a, lua_Number b){
	return a>b ? a: b;
}

static lua_Number my_min(lua_Number a, lua_Number b){
	return a<b ? a: b;
}
/*
obb
      +-------+
      |       |
      |       |
      |       |
      |   z   |
      |   ^   |
      |   |   |
      +---p---+ ->x
*/


/* aabb's center is (0,0) */
/* ref https://zhuanlan.zhihu.com/p/607728812?utm_id=0 */
static int check_intersect_aabb_circel(lua_Number halfW, lua_Number halfH, lua_Number cx, lua_Number cy, lua_Number cr ){
	//printf("%f, %f,%f, %f, %f\n", halfW, halfH, cx, cy, cr);
	lua_Number vx, vy, ux, uy;
	vx = abs(cx);
	vy = abs(cy);
	ux = my_max(0, vx - halfW);
	uy = my_max(0, vy - halfH);

	int result =  (ux * ux + uy * uy) < (cr * cr) ? 1 : 0;
	return result;
}

static void my_rotate2(lua_Number sinR,lua_Number cosR, lua_Number * cx, lua_Number * cy){
	lua_Number tmpcx = *cx;
	*cx = tmpcx * cosR + *cy * sinR;
	*cy = *cy * cosR - tmpcx * sinR;
}

static void my_rotate(lua_Number rot, lua_Number * cx, lua_Number * cy){
	lua_Number cosR = emath_cos_tablelookup(rot);
	lua_Number sinR = emath_cos_tablelookup(90 - rot);
	my_rotate2(sinR, cosR, cx, cy);
}

static int my_do_intersect_cricle_obb(
	lua_Number cx, lua_Number cy, lua_Number cr,
	lua_Number px, lua_Number py, lua_Number width, lua_Number height,
	lua_Number rot)
{
	lua_Number hx = width / 2;
	lua_Number hy = height / 2;

	/* prepare rotate around obb bottom center (px, py) */
	cx -= px;
	cy -= py;

	//printf("0 cx=%f cy=%f rot=%f cosR=%f sinR=%f\n", cx, cy, rot, cosR, sinR);
	/* rotate around (px, py) backrawd */
	my_rotate(-rot, &cx, &cy);

	//printf("1 cx=%f cy=%f\n", cx, cy);
	/* use obb center as coordiate center then obb is aabb*/
	cy -= hy;

	return check_intersect_aabb_circel(hx, hy, cx, cy, cr);
}

static int emath_intersect_cricle_obb(lua_State *L) {
    lua_Number cx = luaL_checknumber(L, 1);
    lua_Number cy = luaL_checknumber(L, 2);
    lua_Number cr = luaL_checknumber(L, 3);

	lua_Number px = luaL_checknumber(L, 4);
    lua_Number py = luaL_checknumber(L, 5);
    lua_Number width = luaL_checknumber(L, 6);
	lua_Number height = luaL_checknumber(L, 7);
	lua_Number rot = luaL_checknumber(L, 8);

	int result = my_do_intersect_cricle_obb(cx, cy, cr, px, py, width, height, rot);

	lua_pushboolean(L, result);
	return 1;
}


/*trapezoid
	     ^ z
  +-------|---er----+
   \      |       /
    \     |d     /
     \    |     /
      +---p-sr-+ ->x
*/
static int emath_intersect_cricle_trapezoid(lua_State *L) {
    lua_Number cx = luaL_checknumber(L, 1);
    lua_Number cy = luaL_checknumber(L, 2);
    lua_Number cr = luaL_checknumber(L, 3);

	lua_Number px = luaL_checknumber(L, 4);
    lua_Number py = luaL_checknumber(L, 5);
    lua_Number sr = luaL_checknumber(L, 6);
	lua_Number er = luaL_checknumber(L, 7);
	lua_Number d = luaL_checknumber(L, 8);
	lua_Number rot = luaL_checknumber(L, 9);

	if(d == 0 || (er == sr && sr == 0) || er < 0 || sr < 0)
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	/* prepare rotate around p(px, py) */
	cx -= px;
	cy -= py;
	
	/* rotate circle center backward rot degree */
	my_rotate(-rot, &cx, &cy);

	//printf("rotated cx=%f, cy=%f\n", cx, cy);
	/* use trapezoid center as coordiate center*/
	cy -= d / 2;

	/* flip circle to right */
	cx = abs(cx);

	int result = 0;
	/* step 1. check circle intersect with any point in outer aabb of trapezoid*/
	if(0 != check_intersect_aabb_circel(my_max(sr, er), d / 2, cx, cy, cr))
	{
		//printf("rotated1 cx=%f, cy=%f\n", cx, cy);
		if(sr == er)
		{
			result = 1;
		}
		else
		{
			/* step 2. check circle intersect with any point in inner aabb of trapezoid*/
			if(0 != check_intersect_aabb_circel(my_min(sr, er), d / 2, cx, cy, cr))
			{
				result = 1;
			}
			else
			{
				/* step 3. check circle intersect with any point in left corner triangle */
				/* make a obb contains the left coner triangle */
				if(sr > er)  /* flip trapezoid by X aix if need. make sure trapezoid's up edge is longer than bottom edge*/
				{
					/* flip trapezoid */
					lua_Number tmp = sr;
					sr = er;
					er = tmp;

					/* flip circle */
					cy = -cy;
				}
				lua_Number uLength = er - sr;
				lua_Number theta = emath_rad2deg(emath_atan_double(uLength / d));
				lua_Number sinTheta = emath_cos_tablelookup(90 - theta);
				lua_Number cosTheta = emath_cos_tablelookup(theta);

				lua_Number obbHeight = d / cosTheta;
				lua_Number obbWidth = d * sinTheta;

				lua_Number leftX = -1;
				lua_Number leftY = 0;

				/* circle center rotate with p*/
				my_rotate2(sinTheta, cosTheta, &leftX, &leftY);
				
				lua_Number obbPx = sr + leftX * obbWidth / 2;
				lua_Number obbPy = -d / 2 + leftY * obbWidth / 2;
				//printf("rotated1 (%f,%f) (%f, %f) (%f,%f) %f\n",cx, cy, obbPx, obbPy, obbWidth, obbHeight, theta);
				result = my_do_intersect_cricle_obb(cx, cy, cr, obbPx, obbPy, obbWidth, obbHeight, theta);
			}
		}
	}

	lua_pushboolean(L, result);
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
		{"sqrdist_point_2_segment",   	emath_sqrdist_point_2_segment},
		{"intersect_cricle_circle",   	emath_intersect_cricle_circle},
		{"intersect_cricle_obb",   		emath_intersect_cricle_obb},
		{"intersect_cricle_trapezoid",  emath_intersect_cricle_trapezoid},
		
		{NULL, NULL},
	};
	luaL_newlib(L, lfuncs);
	return 1;
}
