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

    /**
     * Returns true if the given character is a path separator.
     */
    static inline bool isSep(char c) {
        return c == '/';
    }

    /**
     * Returns the root portion of the path.
     *
     * On Posix, simply returns "/" if the path starts with it.
     *
     * On Windows, returns a matching "X:\", "\\server\share", or
     * "\\?\UNC\server\share".
     *
     * If the path is not rooted, returns a path of length 0.
     */
    PosixPath root() const;

    /**
     * Returns true if the given path is absolute.
     */
    bool isabs() const;
};
