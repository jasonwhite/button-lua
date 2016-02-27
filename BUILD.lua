--[[
Copyright (c) Jason White, 2016
License: MIT

Description:
Generates the build description.
]]

local cc = require "rules.cc"

import "contrib/BUILD.lua"

cc.binary {
    name = "bblua",
    deps = {"lua:static"},
    srcs = glob "src/*.cc",
    includes = {"contrib/lua/include"},
    compiler_opts = {"-g", "-Wall", "-Werror"},
    linker_opts = {"-dl"},
}
