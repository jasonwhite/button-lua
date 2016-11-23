/**
 * Copyright (c) Jason White
 *
 * MIT License
 *
 * Description:
 * File path manipulation.
 */
#include "posixpath.h"

#include "lua.hpp"

int PosixPath::cmp(char a, char b) {
    if (isSep(a) && isSep(b))
        return 0;

    //return (int)tolower(a) - (int)tolower(b);
    return (int)a - (int)b;
}

PosixPath PosixPath::root() const {
    if (length == 1 && isSep(path[0]))
        return PosixPath(path, 1);

    return PosixPath(nullptr, 0);
}

bool PosixPath::isabs() const {
    return length > 0 && isSep(path[0]);
}
