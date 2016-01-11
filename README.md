[buildbadge]: https://travis-ci.org/jasonwhite/brilliant-build-lua.svg?branch=master
[buildstatus]: https://travis-ci.org/jasonwhite/brilliant-build-lua

# Lua Build Descriptions for Brilliant Build [![Build Status][buildbadge]][buildstatus]

A tool to generate build descriptions from Lua scripts that are suitable for
input to [Brilliant Build][].

[Brilliant Build]: https://github.com/jasonwhite/brilliant-build

## Example

```lua
local cc = require "rules.cc"

cc.binary {
    name = "foobar",
    srcs = {"foo.c", "bar.c"},
}
```

If this script is named `BUILD.lua`, we can generate the low-level build
description by running

    bblua BUILD.lua -o bb.json

See also [BUILD.lua](/BUILD.lua) for this repository for a real-world example.

## Building it

Simply run:

    make

This will create a self-contained executable named `bblua`.

## License

[MIT License](/LICENSE.md)
