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
#include "version.h"
#include <regex.h>
#include <stdio.h>
#include <string.h>

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

static char *__substring(const char *string, ssize_t from, ssize_t to)
{
    char *ret = malloc(to - from + 1);
    snprintf(ret, to - from, "%s", string+from);
    return ret;
}

int replace(lua_State *L)
{
    unsigned idx;
    const char *string, *substitution;
    bool is_string = false;
    regex_t *rx;
    struct LeRegex_t *leregex;
    regmatch_t *match;
    int retval = 1;

    string = luaL_checkstring(L, 1);
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

    substitution = luaL_checkstring(L, 3);
    if (substitution == NULL) {
        retval = 0;
        goto replace_out;
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

    char *sidx = malloc(16);
    char *string_copy = malloc(strlen(string)+1);
    memcpy(string_copy, string, strlen(string)+1);

    for (idx = 1; idx < blocks+1; idx++) {
        sprintf(sidx, "\\%lu", (unsigned long)idx);
        char *k = strstr(substitution, sidx);

        if (k == NULL) {
            free(sidx);
            retval = 0;
            break;
        }
        
        unsigned from = k - substitution;
        unsigned to = from + strlen(sidx);
        char *substring = __substring(string, match[idx].rm_so, match[idx].rm_eo);
        char *replaced = __string_replace(substitution, from, to, substring);
        char *string_iter = __string_replace(string_copy, match[0].rm_so, match[0].rm_eo, replaced);
        free(string_copy);
        string_copy = malloc(strlen(string_iter) + 1);
        memcpy(string_copy, string_iter, strlen(string_iter) + 1);
        free(string_iter);
        free(substring);
    }
    free(sidx);
    lua_pushstring(L, (const char *)string_copy);
    free(string_copy);
    free(match);

replace_out:
    if (is_string) {
        free(leregex->expression);
        free(leregex);
    }

    return retval;
}

int replace_all(lua_State *L){
    int retval = 1;
    // TODO
    return retval;
}

int replace_n(lua_State *L){
    int retval = 1;
    // TODO
    return retval;
}

static void lua_push_key_method(lua_State* L, const char *key, int (*function)(lua_State *))
{
    lua_pushcfunction(L, function);
    lua_setfield(L, -2, key);
}

struct RegexConstants_t {
    int reg_constant;
    const char *lua_name;
};

const struct RegexConstants_t leregex_constants[] = {
    {REG_EXTENDED, "leregex_extended"},
    {REG_ICASE, "leregex_icase"},
    {REG_NOSUB, "leregex_nosub"},
    {REG_NEWLINE, "leregex_newline"},
    {REG_NOTBOL, "leregex_notbol"},
    {REG_NOTEOL, "leregex_noteol"},
    {-1, NULL},
};

static void lua_export_regex_constants(lua_State *L)
{
    unsigned idx;
    for (idx = 0; leregex_constants[idx].lua_name != NULL; idx++){
        lua_pushinteger(L, leregex_constants[idx].reg_constant);
        lua_setfield(L, -2, leregex_constants[idx].lua_name);
    }
}


int luaopen_leregex(lua_State *L){
    lua_newtable(L);
    lua_push_key_method(L, "r", r);
    lua_push_key_method(L, "match", match);
    lua_push_key_method(L, "match_all", match_all);

    lua_export_regex_constants(L);

    lua_newtable(L);
    lua_pushinteger(L, LEREGEX_VERSION);
    lua_setfield(L, -2, "version_number");
    lua_pushstring(L, "" STR(LEREGEX_MAJOR) "." STR(LEREGEX_MINOR) "." STR(LEREGEX_RELEASE) "");
    lua_setfield(L, -2, "version_string");
    lua_setfield(L, -2, "version");

    return 1;
}
