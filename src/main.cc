/**
 * Copyright (c) Jason White
 *
 * MIT License
 *
 * Description:
 * Program entry point.
 */
#include <iostream>

#include "button-lua.h"

int main(int argc, char **argv) {
    lua_State *L = luaL_newstate();
    if (!L) return 1;

    int ret;

    ret = buttonlua::init(L);

    if (ret == 0)
        ret = buttonlua::execute(L, argc, argv);

    lua_close(L);

    return ret;
}
