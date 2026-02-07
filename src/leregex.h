/***************************************************************************
 *      Copyright (c) 2026 - Yassin Achengli <achengli@github.com>
 *
 *      This software is under BSD license.
 ***************************************************************************
 *      + File: leregex.h
 *      + Description: Extended regular expressions using regex.h for
 *                  lua.
 ***************************************************************************/
#ifndef LEREGEX_H
#define LEREGEX_H

#ifdef _LUA_DEBUG_5_1_VERSON
#include <lua-5.1/lua.h>
#include <lua-5.1/lauxlib.h>
#include <lua-5.1/lualib.h>
#else
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#endif

#include <regex.h>
#include <stdlib.h>
#include <string.h>

#define LEREGEX_TYPE "leregex.regex"

struct LeRegex_t {
    regex_t rx;
    char *expression;
};

int r(lua_State *L);
int match(lua_State *L);
int match_all(lua_State *L);
// TODO
int replace(lua_State *L);
int replace_all(lua_State *L);
int replace_n(lua_State *L);
// END TODO

static const luaL_Reg leregex[] = {
    {"r", r},
    {"match", match},
    {"match_all", match_all},
    {NULL, NULL}
};

int luaopen_leregex(lua_State *L);

#endif
