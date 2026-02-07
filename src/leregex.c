/***************************************************************************
 *      Copyright (c) 2026 - Yassin Achengli <achengli@github.com>
 *
 *      This software is under BSD license.
 ***************************************************************************
 *      + File: leregex.c
 *      + Description: Extended regular expressions using regex.h for
 *                  lua.
 ***************************************************************************/
#include "leregex.h"

int r(lua_State *L)
{
    struct LeRegex_t *leregex;
    int reg_flags = REG_EXTENDED;

    const char *regular_expression = luaL_checkstring(L, 1);

#if LUA_VERSION_NUM > 501
    leregex = (struct LeRegex_t *)lua_newuserdatauv(L, sizeof(struct LeRegex_t) + strlen(regular_expression) + 1, 0);
#else
    leregex = (struct LeRegex_t *)lua_newuserdata(L, sizeof(struct LeRegex_t) + strlen(regular_expression) + 1);
#endif

    memcpy(leregex->expression, regular_expression, strlen(regular_expression)+1);

    if (regular_expression == NULL)
        return 0;

    if (lua_isinteger(L, 2))
        reg_flags = lua_tointeger(L, 2);

    if (regcomp(&leregex->rx, regular_expression, reg_flags) != 0)
        return 0;

    luaL_getmetatable(L, LEREGEX_TYPE);
    lua_setmetatable(L, -2);

    return 1;
}

int match(lua_State *L)
{
    regex_t *rx;
    struct LeRegex_t *leregex;
    regmatch_t match; 
    bool is_string = false;

    const char *string = luaL_checklstring(L, 1, NULL);

    if (string == NULL)
        return 0;

    if (lua_isstring(L, 2)) {
        const char *regex = luaL_checklstring(L, 2, NULL);
        rx = malloc(sizeof(regex_t));
        if (regcomp(rx, regex, REG_EXTENDED) != 0) {
            free(rx);
            return 0;
        }
        is_string = true;
    } else {
        leregex = (struct LeRegex_t *)lua_touserdata(L, 2);
        rx = &leregex->rx;
    }

    if (regexec(rx, string, 1, &match, 0) == 0) {
        size_t len = match.rm_eo - match.rm_so;
        char *substring = malloc(len + 1);

        if (substring == NULL) {
            if (is_string)
                free(rx);
            return 0;
        }

        memcpy(substring, string + match.rm_so,len);
        substring[len] = 0;

        lua_newtable(L);

        lua_pushinteger(L,match.rm_so + 1); // +1 (because lua indexing compatibility)
        lua_setfield(L, -2, "from");

        lua_pushinteger(L,match.rm_eo);
        lua_setfield(L, -2, "to");

        lua_pushstring(L, substring);
        lua_setfield(L, -2, "match");

        free(substring);
    } else {
        if (is_string)
            free(rx);
        return 0;
    }

    if (is_string)
        free(rx);

    return 1;
}

int match_all(lua_State *L)
{
    struct LeRegex_t *leregex;
    regex_t *rx;
    regmatch_t match; 
    unsigned idx;
    ssize_t last;
    bool is_string = false;

    const char *string = luaL_checklstring(L, 1, NULL);

    if (string == NULL)
        return 0;

    leregex = (struct LeRegex_t *)luaL_checkudata(L, 2, LEREGEX_TYPE);
    if (leregex != NULL) {
        rx = &leregex->rx;
    } else {
        const char *regex = luaL_checklstring(L, 2, NULL);
        if (regex == NULL)
            return 0;

        rx = malloc(sizeof(regex_t));
        if (regcomp(rx, regex, REG_EXTENDED) != 0) {
            free(rx);
            return 0;
        }
        is_string = true;
    } 

    lua_newtable(L);

    for (idx = 1, last = 0; regexec(rx, string+last, 1, &match, 0) == 0; idx++) {
        size_t len = match.rm_eo - match.rm_so;
        char *substring = malloc(len + 1);

        if (substring == NULL)
            break;

        memcpy(substring, string + last + match.rm_so, len);
        substring[len] = '\0';

        lua_pushinteger(L, idx);
        lua_newtable(L);

        lua_pushinteger(L, last + match.rm_so + 1); // +1 (because lua indexing compatibility)
        lua_setfield(L, -2, "from");

        lua_pushinteger(L, last + match.rm_eo);
        lua_setfield(L, -2, "to");

        lua_pushstring(L, substring);
        lua_setfield(L, -2, "match");

        lua_settable(L, -3);

        free(substring);

        if (strlen(string) <= last) break;
        last += match.rm_eo;
    }

    if (is_string) {
        free(rx);
    }

    return 1;
}

static int __match_bloc_count(const char *string)
{
    unsigned count = 0;
    unsigned idx;
    bool last = false;
    int stack = 0;

    for (idx = 0; idx < strlen(string); idx++){
        switch (string[idx]) {
            case '(':
                last = true;
                stack++;
                break;
            case ')':
                if (last) {
                    count++;
                }
                stack--;
                last = false;
                break;
            default:
                continue;
        }
    }
    
    return stack == 0 ? count : -1;
}

static char *__string_replace(const char *string, ssize_t from, ssize_t to, const char *substitution)
{
    const size_t len = strlen(string) - (to - from) + strlen(substitution);
    char *ret = malloc(len); 
    char *copy = malloc(strlen(string) + 1);
    memcpy(copy, string, strlen(string) + 1);

    copy[from] = '\0';

    free(copy);
    return ret;
}

int replace(lua_State *L)
{
    const char *string = luaL_checkstring(L, 1);
    bool is_string = false;
    regex_t *rx;
    struct LeRegex_t *leregex;
    regmatch_t *match;
    int retval = 1;

    if (string == NULL)
        return 0;

    leregex = (struct LeRegex_t *)luaL_checkudata(L, 2, LEREGEX_TYPE);
    if (leregex != NULL){
        rx = &leregex->rx;
    } else {
        leregex = malloc(sizeof(struct LeRegex_t));
        if (leregex == NULL)
            return 0;

        rx = &leregex->rx;

        const char *regex = luaL_checkstring(L, 2);
        leregex->expression = malloc(strlen(regex) + 1);
        memcpy(leregex->expression, regex, strlen(regex) + 1);

        if (regcomp(rx, regex, REG_EXTENDED) != 0) {
            free(rx);
            return 0;
        }

        is_string = true;
    }


    int blocks = __match_bloc_count(leregex->expression);

    if (blocks < 0) {
        if (is_string) {
            retval = 0;
            goto replace_out;
        }
    }


    match = malloc(sizeof(regmatch_t) * blocks + 1);
    if (regexec(&leregex->rx, string, blocks+1, match, 0) != 0){
        free(match);
        goto replace_out;
    }

    // TODO: Implement substitution logic


replace_out:
    if (is_string) {
        free(leregex->expression);
        free(leregex);
    }

    return retval;
}

static void lua_pushkeymethod(lua_State* L, const char *key, int (*function)(lua_State *))
{
    lua_pushcfunction(L, function);
    lua_setfield(L, -2, key);
}


int luaopen_leregex(lua_State *L){
    lua_newtable(L);
    lua_pushkeymethod(L, "r", r);
    lua_pushkeymethod(L, "match", match);
    lua_pushkeymethod(L, "match_all", match_all);

    lua_newtable(L);
    lua_pushinteger(L, 101);
    lua_setfield(L, -2, "version_number");
    lua_pushstring(L, "1.0.1");
    lua_setfield(L, -2, "version_string");
    lua_setfield(L, -2, "version");

    return 1;
}
