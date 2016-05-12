/**
 * Copyright (c) Jason White
 *
 * MIT License
 *
 * Description:
 * Handles writing out rules.
 */
#include "lua.hpp"

#include <stdio.h>
#include "rules.h"

namespace {

const char  escape_chars[] = {'\"',   '\t',  '\r',  '\n',  '\b',  '\\',  '\0'};
const char* replacements[] = {"\\\"", "\\t", "\\r", "\\n", "\\b", "\\\\", NULL};

/**
 * For the given character, returns the equivalent JSON escape sequence. If the
 * given character is not a character to be escaped, returns NULL.
 */
const char* json_escape_sequence(char c) {
    for (size_t i = 0; escape_chars[i] != '\0'; ++i) {
        if (escape_chars[i] == c)
            return replacements[i];
    }

    return NULL;
}

/**
 * Escapes the given string that will be output to JSON. The resulting string is
 * left at the top of the stack.
 */
int json_escaped_string(lua_State *L, const char* s, size_t len) {

    size_t newlen = len;

    // Calculate the new size of the escaped string
    for (size_t i = 0; i < len; ++i) {
        if (json_escape_sequence(s[i]))
            ++newlen;
    }

    luaL_Buffer b;
    char* buf = luaL_buffinitsize(L, &b, newlen);
    size_t j = 0;

    for (size_t i = 0; i < len; ++i) {
        if (const char* r = json_escape_sequence(s[i])) {
            buf[j++] = r[0];
            buf[j++] = r[1];
            continue;
        }

        buf[j++] = s[i];
    }

    luaL_pushresultsize(&b, newlen);

    return 1;
}

}

namespace buttonlua {

Rules::Rules(FILE* f) : _f(f), _n(0) {
    fputs("[", _f);
}

Rules::~Rules() {
    fputs("\n]\n", _f);
}

int Rules::add(lua_State *L) {

    luaL_checktype(L, 1, LUA_TTABLE);

    if (_n > 0)
        fputs(",", _f);

    fputs("\n    {", _f);

    lua_getfield(L, 1, "inputs");
    listToJSON(L, "inputs", 0);

    lua_getfield(L, 1, "task");
    listToJSON(L, "task", 1);

    lua_getfield(L, 1, "outputs");
    listToJSON(L, "outputs", 2);

    // Optional working directory
    if (lua_getfield(L, 1, "cwd") != LUA_TNIL)
    {
        stringToJSON(L, "cwd", 3);
    }

    // Optional task display
    if (lua_getfield(L, 1, "display") != LUA_TNIL)
    {
        stringToJSON(L, "display", 4);
    }

    fputs("\n    }", _f);

    ++_n;

    return 0;
}

int Rules::stringToJSON(lua_State *L, const char* field, size_t i) {

    if (i > 0)
        fputs(",", _f);

    fprintf(_f, "\n        \"%s\": ", field);

    size_t len;
    const char* s;

    s = lua_tolstring(L, -1, &len);
    if (!s)
        return luaL_error(L, "bad type for field '%s' (string expected, got %s)",
                field, luaL_typename(L, -1));

    json_escaped_string(L, s, len);
    s = lua_tolstring(L, -1, &len);

    fputs("\"", _f);
    fwrite(s, 1, len, _f);
    fputs("\"", _f);

    lua_pop(L, 2); // Pop escaped string and table field

    return 0;
}

int Rules::listToJSON(lua_State *L, const char* field, size_t i) {

    // Which element we're on.
    size_t element = 0;

    if (i > 0)
        fputs(",", _f);

    fprintf(_f, "\n        \"%s\": ", field);

    size_t len;
    const char* s;

    if (lua_type(L, -1) != LUA_TTABLE)
        return luaL_error(L, "bad type for field '%s' (table expected, got %s)",
                field, luaL_typename(L, -1));

    fputs("[", _f);

    for (int i = 1; ; ++i) {

        if (lua_rawgeti(L, -1, i) == LUA_TNIL) {
            lua_pop(L, 1);
            break;
        }

        s = lua_tolstring(L, -1, &len);
        if (!s)
            return luaL_error(L,
                    "bad type in table for field '%s' (string expected, got %s)",
                    field, luaL_typename(L, -1));

        if (element > 0)
            fputs(", ", _f);

        json_escaped_string(L, s, len);
        const char* escaped = lua_tolstring(L, -1, &len);

        fputs("\"", _f);
        fwrite(escaped, 1, len, _f);
        fputs("\"", _f);

        lua_pop(L, 1); // Pop escaped string

        ++element;

        lua_pop(L, 1); // Pop table element
    }

    fputs("]", _f);

    lua_pop(L, 1); // Pop table field

    return 0;
}

} // namespace buttonlua
