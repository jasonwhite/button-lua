/**
 * Copyright (c) Jason White
 *
 * MIT License
 *
 * Description:
 * Path manipulation module.
 */
#include "path.h"

#include "lua.hpp"

namespace path {

int cmp(char a, char b) {
    if (isSep(a) && isSep(b))
        return 0;

#if PATH_STYLE == PATH_STYLE_WINDOWS
    return (int)tolower(a) - (int)tolower(b);
#else
    return (int)a - (int)b;
#endif
}

int cmp(const char* a, const char* b, size_t len) {
	int result;
	for (size_t i = 0; i < len; ++i) {
        result = cmp(a[i], b[i]);
        if (result != 0)
            break;
	}

	return result;
}

int cmp(const char* a, const char* b, size_t len1, size_t len2) {
    if (len1 < len2)
        return -1;
    else if (len2 < len1)
        return 1;

    // Lengths are equal
    return cmp(a, b, len1);
}

bool Path::isabs() const {
    if(length > 0 && isSep(path[0]))
        return true;

#if PATH_STYLE == PATH_STYLE_WINDOWS
    if(length > 2 && path[1] == ':' && isSep(path[2]))
        return true;
#endif

    return false;
}

Path Path::root() const {
    if (length == 1 && path[0] == '/')
        return Path(path, 1);

    return Path(nullptr, 0);
}

bool Path::isRoot() const {
    return length > 0 && root().length == length;
}

Path Path::dirname() const {
    return split().head;
}

Path Path::basename() const {
    return split().tail;
}

std::string Path::copy() const {
    return std::string(path, length);
}

Split Path::split() const {

    // Search backwards for the last path separator
    size_t tail_start = length;

    while (tail_start > 0) {
        --tail_start;
        if (isSep(path[tail_start])) {
            ++tail_start;
            break;
        }
    }

    // Trim off the path separator(s)
    size_t head_end = tail_start;
    while (head_end > 0) {
        --head_end;

        if (!isSep(path[head_end])) {
            ++head_end;
            break;
        }
    }

    if (head_end == 0)
        head_end = tail_start;

    Split s;
    s.head.path   = path;
    s.head.length = head_end;
    s.tail.path   = path+tail_start;
    s.tail.length = length-tail_start;
    return s;
}

Split Path::splitExtension() const {
    size_t base = length;

    // Find the base name
    while (base > 0) {
        --base;
        if (isSep(path[base])) {
            ++base;
            break;
        }
    }

    // Skip past initial dots
    while (base < length && path[base] == '.')
        ++base;

    // Skip past non-dots
    while (base < length && path[base] != '.')
        ++base;

    Split s;
    s.head.path = path;
    s.head.length = base;
    s.tail.path = path+base;
    s.tail.length = length-base;
    return s;
}

std::vector<Path> Path::components() const {
    std::vector<Path> v;
    components(v);
    return v;
}

void Path::components(std::vector<Path>& v) const {
    Split s = split();

    if (s.head.isRoot() && !s.tail.length) {
        v.push_back(s.head);
        return;
    }

    if (s.head.length)
    {
        s.head.components(v);

        if (s.tail.length)
            v.push_back(s.tail);
    }
    else if (s.tail.length) {
        v.push_back(s.tail);
    }
}

int Path::components(lua_State* L) const {
    Split s = split();

    if (s.head.isRoot() && !s.tail.length) {
        lua_pushlstring(L, s.head.path, s.head.length);
        return 1;
    }

    if (s.head.length)
    {
        int n = s.head.components(L);
        if (s.tail.length) {
            lua_pushlstring(L, s.tail.path, s.tail.length);
            return n + 1;
        }

        return n;
    }
    else if (s.tail.length) {
        lua_pushlstring(L, s.tail.path, s.tail.length);
        return 1;
    }

    return 0;
}

std::string Path::norm() const {
    std::string buf;
    norm(buf);
    return buf;
}

void Path::norm(std::string& buf) const {

    // Stack of path components that we build up.
    std::vector<Path> stack;

    for (auto&& c: components()) {
        if (c.isDot()) {
            // Filter out "." path components
            continue;
        }
        else if (c.isDotDot() && !stack.empty() && !stack.back().isDotDot()) {
            if (!stack.back().isabs())
                stack.pop_back();
        }
        else {
            stack.push_back(c);
        }
    }

    if (stack.empty()) {
        join(buf, ".");
    }
    else {
        for (auto&& c: stack)
            join(buf, c);
    }
}

std::string& join(std::string& buf, Path path) {
    if (path.isabs()) {
        // Path is absolute, reset the buffer length
        buf.clear();
    }
    else {
        // Path is relative, add path separator if necessary.
        size_t len = buf.size();
        if (len > 0 && !isSep(buf[len-1]))
            buf.push_back(defaultSep);
    }

    return buf.append(path.path, path.length);
}

}

static int path_isabs(lua_State* L) {
    size_t len;
    const char* path = luaL_checklstring(L, 1, &len);
    lua_pushboolean(L, path::Path(path, len).isabs());
    return 1;
}

static int path_join(lua_State* L) {

    int argc = lua_gettop(L);

    luaL_Buffer b;
    luaL_buffinit(L, &b);

    for (int i = 1; i <= argc; ++i) {
        if (lua_isnil(L, i))
            continue;

        size_t len;
        const char* path = luaL_checklstring(L, i, &len);

        if (path::Path(path, len).isabs()) {
            // Path is absolute, reset the buffer length
            b.n = 0;
        }
        else {
            // Path is relative, add path separator if necessary.
            if (b.n > 0 && !path::isSep(b.b[b.n-1]))
                luaL_addchar(&b, path::defaultSep);
        }

        luaL_addlstring(&b, path, len);
    }

    luaL_pushresult(&b);
    return 1;
}

static int path_split(lua_State* L) {

    size_t len;
    const char* path = luaL_checklstring(L, 1, &len);

    path::Split s = path::Path(path, len).split();

    lua_pushlstring(L, s.head.path, s.head.length);
    lua_pushlstring(L, s.tail.path, s.tail.length);

    return 2;
}

static int path_basename(lua_State* L) {
    size_t len;
    const char* path = luaL_checklstring(L, 1, &len);

    path::Split s = path::Path(path, len).split();
    lua_pushlstring(L, s.tail.path, s.tail.length);
    return 1;
}

static int path_dirname(lua_State* L) {
    size_t len;
    const char* path = luaL_checklstring(L, 1, &len);

    path::Split s = path::Path(path, len).split();
    lua_pushlstring(L, s.head.path, s.head.length);
    return 1;
}

static int path_splitext(lua_State* L) {

    size_t len;
    const char* path = luaL_checklstring(L, 1, &len);

    path::Split s = path::Path(path, len).splitExtension();

    lua_pushlstring(L, s.head.path, s.head.length);
    lua_pushlstring(L, s.tail.path, s.tail.length);

    return 2;
}

static int path_getext(lua_State* L) {
    size_t len;
    const char* path = luaL_checklstring(L, 1, &len);

    path::Split s = path::Path(path, len).splitExtension();

    lua_pushlstring(L, s.tail.path, s.tail.length);
    return 1;
}

static int path_setext(lua_State* L) {
    size_t pathlen, extlen;
    const char* path = luaL_checklstring(L, 1, &pathlen);
    const char* ext = luaL_checklstring(L, 2, &extlen);

    path::Split s = path::Path(path, pathlen).splitExtension();

    lua_pushlstring(L, s.head.path, s.head.length);
    lua_pushlstring(L, ext, extlen);
    lua_concat(L, 2);

    return 1;
}

static int path_components(lua_State* L) {

    size_t len;
    if (const char* str = luaL_checklstring(L, 1, &len))
        return path::Path(str, len).components(L);

    return 0;
}

static int path_norm(lua_State* L) {

    size_t len;
    const char* path = luaL_checklstring(L, 1, &len);

    auto buf = path::Path(path, len).norm();

    lua_pushlstring(L, buf.data(), buf.length());

    return 1;
}

static const luaL_Reg pathlib[] = {
    {"isabs", path_isabs},
    {"join", path_join},
    {"split", path_split},
    {"basename", path_basename},
    {"dirname", path_dirname},
    {"splitext", path_splitext},
    {"getext", path_getext},
    {"setext", path_setext},
    {"components", path_components},
    {"norm", path_norm},
    {NULL, NULL}
};

int luaopen_path(lua_State* L) {
    luaL_newlib(L, pathlib);
    return 1;
}
