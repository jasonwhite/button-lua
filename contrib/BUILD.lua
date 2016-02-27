--[[
Copyright (c) Jason White, 2016
License: MIT

Description:
Generates the build description.
]]

local cc = require "rules.cc"

cc.library {
    name = "lua:static",
    static = true,
    srcs = glob {
        "lua-5.3.2/src/*.c",
        "!lua-5.3.2/src/lua.c",
        "!lua-5.3.2/src/luac.c",
        },
    compiler_opts = {"-std=gnu99", "-O2", "-Wall", "-Wextra", "-DLUA_COMPAT_5_2"},
    defines = {"LUA_USE_POSIX"},
}
