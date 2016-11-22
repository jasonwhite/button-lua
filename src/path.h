/**
 * Copyright (c) Jason White
 *
 * MIT License
 *
 * Description:
 * Path manipulation module.
 */
#pragma once

#include <stddef.h> // For size_t
#include <string.h> // for strlen
#include <string>
#include <vector>

struct lua_State;

namespace path {

/**
 * Posix paths use forward slashes ('/') as directory separators. Absolute paths
 * begin with a directory separator.
 */
#define PATH_STYLE_POSIX 0

/**
 * Windows (not DOS!) paths use backslashes ('\') or forward slashes ('/') as
 * directory separators. Drive letters can be prepended to either absolute or
 * relative paths. Networked (UNC) paths begin with a double backslash ("\\").
 */
#define PATH_STYLE_WINDOWS 1

#ifndef PATH_STYLE
#   ifdef _WIN32
#       define PATH_STYLE PATH_STYLE_WINDOWS
#   else
#       define PATH_STYLE PATH_STYLE_POSIX
#   endif
#endif

#if PATH_STYLE == PATH_STYLE_WINDOWS

const char defaultSep = '\\';
const bool caseSensitive = false;

inline bool isSep(char c) {
    return c == '/' || c == '\\';
}

#else

const char defaultSep = '/';
const bool caseSensitive = true;

inline bool isSep(char c) {
    return c == '/';
}

#endif

/**
 * Compare two characters. The comparison is case insensitive for Windows style
 * paths.
 */
int cmp(char a, char b);

/**
 * Compares two paths for equality.
 */
int cmp(const char* a, const char* b, size_t len);
int cmp(const char* a, const char* b, size_t len1, size_t len2);

struct Split;

struct Path {

    Path() : path(NULL), length(0) {}
    Path(const std::string& s) : path(s.data()), length(s.length()) {}
    Path(const char* path) : path(path), length(strlen(path)) {}
    Path(const char* path, size_t length) : path(path), length(length) {}

    const char* path;
    size_t length;

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
    Path root() const;

    /**
     * Returns true if the path is just a root and nothing else.
     */
    bool isRoot() const;

    /**
     * Returns true if the given path is absolute.
     */
    bool isabs() const;

    /**
     * Gets the directory name of the path.
     */
    Path dirname() const;

    /**
     * Gets the name of just the file name in the path.
     */
    Path basename() const;

    /**
     * Returns a copy of the path as a string.
     */
    std::string copy() const;

    /**
     * Splits a path such that the head is the parent directory (empty if none) and
     * the tail is the basename of the file path.
     */
    Split split() const;

    /**
     * Splits a path into an extension.
     */
    Split splitExtension() const;

    /**
     * Returns a list of the path components.
     */
    std::vector<Path> components() const;
    void components(std::vector<Path>& v) const;
    int components(lua_State* L) const;

    /**
     * Normalizes the path such that "." and ".." are resolved. Superfluous
     * directory separators are also removed.
     */
    std::string norm() const;

    /**
     * Joins the normalized path to an existing buffer.
     */
    void norm(std::string& buf) const;

    /**
     * Returns true if the path is equal to ".".
     */
    bool isDot() const {
        return length == 1 && path[0] == '.';
    }

    /**
     * Returns true if the path is equal to "..".
     */
    bool isDotDot() const {
        return length == 2 && path[0] == '.' && path[1] == '.';
    }
};

/**
 * Helper struct for representing a split path.
 */
struct Split {
    Path head;
    Path tail;
};

/**
 * Joins two paths.
 */
std::string& join(std::string& buf, Path path);

}

/**
 * Pushes the path library onto the stack so that it can be registered.
 */
int luaopen_path(lua_State* L);
