/**
 * Copyright (c) Jason White
 *
 * MIT License
 *
 * Description:
 * File path manipulation.
 */
#pragma once

#include <stddef.h> // For size_t
#include <string.h> // for strlen
#include <string>
#include <vector>

#include "path/base.h"

class PosixPath : public BasePath<PosixPath> {
public:
    using BasePath<PosixPath>::BasePath;

    static const char defaultSep = '/';
    static const bool caseSensitive = true;

    static int cmp(char a, char b);

    static inline bool isSep(char c) {
        return c == '/';
    }

    size_t rootLength() const;
};
