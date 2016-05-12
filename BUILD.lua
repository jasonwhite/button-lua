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
        "src/lua/src/*.c",
        "!src/lua/src/lua.c",
        "!src/lua/src/luac.c",
        },
    compiler_opts = {"-std=gnu99", "-O2", "-Wall", "-Wextra", "-DLUA_COMPAT_5_2"},
    defines = {"LUA_USE_POSIX"},
}

cc.binary {
    name = "button-lua",
    deps = {"lua:static"},
    srcs = glob "src/*.cc",
    includes = {"contrib/lua/include"},
    compiler_opts = {"-g", "-Wall", "-Werror"},
    linker_opts = {"-dl"},
}
