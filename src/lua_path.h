/**
 * Copyright (c) Jason White
 *
 * MIT License
 *
 * Description:
 * Path manipulation module.
 */
#pragma once

struct lua_State;

/**
 * Pushes the path library onto the stack so that it can be registered.
 */
int luaopen_path(lua_State* L);
