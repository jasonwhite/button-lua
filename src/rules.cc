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

int json_print(lua_State* L, FILE* f);
const char* json_escape_sequence(char c);
void json_print_string(const char* s, size_t len, FILE* f);
int json_print_table(lua_State* L, FILE* f);
int json_print(lua_State* L, FILE* f);

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
 * Prints the given string to the given file in JSON format.
 */
void json_print_string(const char* s, size_t len, FILE* f) {
    fputc('"', f); // Opening quote

    for (size_t i = 0; i < len; ++i) {
        if (const char* r = json_escape_sequence(s[i]))
            fputs(r, f);
        else
            fputc(s[i], f);
    }

    fputc('"', f); // Closing quote
}

/**
 * Prints the table at the top of the stack.
 *
 * The table is assumed to be a sequential array.
 */
int json_print_table(lua_State* L, FILE* f) {

    fputs("[", f);

    for (int i = 1; ; ++i) {
        int type = lua_rawgeti(L, -1, i);

        if (type == LUA_TNIL) {
            lua_pop(L, 1);
            break;
        }

        if (i > 1)
            fputs(", ", f);

        json_print(L, f);

        // Pop table element
        lua_pop(L, 1);
    }

    fputs("]", f);

    return 0;
}

/**
 * Prints the value at the top of the stack.
 */
int json_print(lua_State* L, FILE* f) {

    switch (lua_type(L, -1))
    {
    case LUA_TNIL:
        fputs("null", f);
        break;

    case LUA_TBOOLEAN:
        fputs(lua_toboolean(L, -1) ? "true" : "false", f);
        break;

    case LUA_TNUMBER:
        fprintf(f, "%f", lua_tonumber(L, -1));
        break;

    case LUA_TTABLE:
        return json_print_table(L, f);

    case LUA_TSTRING:
        size_t len;
        if (const char* s = lua_tolstring(L, -1, &len))
            json_print_string(s, len, f);
        break;

    default:
        return luaL_error(L, "cannot represent type %s as JSON",
                luaL_typename(L, -1));
    }

    return 0;
}

/**
 * Prints a single field of a JSON dictionary.
 */
int json_print_field(lua_State* L, const char* field, FILE* f) {

    fprintf(f, "\"%s\": ", field);

    json_print(L, f);

    return 0;
}

}

namespace buttonlua {

Rules::Rules(FILE* f) : _f(f), _n(0) {
    fputs("[", _f);
}

Rules::~Rules() {
    fputs("\n]\n", _f);
}

int Rules::add(lua_State* L) {

    luaL_checktype(L, 1, LUA_TTABLE);

    if (_n > 0)
        fputs(",", _f);

    fputs("\n    {\n        ", _f);

    // Inputs (required)
    lua_getfield(L, 1, "inputs");
    if (lua_type(L, -1) == LUA_TTABLE)
        json_print_field(L, "inputs", _f);
    else
        return luaL_error(L, "bad type for field '%s' (table expected, got %s)",
                "inputs", luaL_typename(L, -1));

    // Task (required)
    lua_getfield(L, 1, "task");
    if (lua_type(L, -1) == LUA_TTABLE) {
        fputs(",\n        ", _f);
        json_print_field(L, "task", _f);
    }
    else
        return luaL_error(L, "bad type for field '%s' (table expected, got %s)",
                "task", luaL_typename(L, -1));

    // Outputs (required)
    lua_getfield(L, 1, "outputs");
    if (lua_type(L, -1) == LUA_TTABLE) {
        fputs(",\n        ", _f);
        json_print_field(L, "outputs", _f);
    }
    else
        return luaL_error(L, "bad type for field '%s' (table expected, got %s)",
                "outputs", luaL_typename(L, -1));

    // Working directory (optional)
    lua_getfield(L, 1, "cwd");
    switch (lua_type(L, -1))
    {
    case LUA_TSTRING:
        fputs(",\n        ", _f);
        json_print_field(L, "cwd", _f);
        break;
    case LUA_TNIL:
        // Not specified
        break;
    default:
        return luaL_error(L, "bad type for field '%s' (string expected, got %s)",
                "cwd", luaL_typename(L, -1));
    }

    // Display (optional)
    lua_getfield(L, 1, "display");
    switch (lua_type(L, -1))
    {
    case LUA_TSTRING:
        fputs(",\n        ", _f);
        json_print_field(L, "display", _f);
        break;
    case LUA_TNIL:
        // Not specified
        break;
    default:
        return luaL_error(L, "bad type for field '%s' (string expected, got %s)",
                "display", luaL_typename(L, -1));
    }

    fputs("\n    }", _f);

    ++_n;
    return 0;
}

} // namespace buttonlua
