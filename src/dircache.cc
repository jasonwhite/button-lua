/**
 * Copyright (c) Jason White
 *
 * MIT License
 */

#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>
#include <utility>

#include "dircache.h"
#include "glob.h"
#include "path.h"
#include "deps.h"

using path::Path;

bool operator<(const DirEntry& a, const DirEntry& b) {
    return std::tie(a.name, a.isDir) < std::tie(b.name, b.isDir);
}

namespace {

/**
 * Returns a list of the files in a directory.
 */
DirEntries dirEntries(const std::string& path) {

    DirEntries entries;

#ifdef _WIN32

    // TODO: Implement on Windows. We'll need to convert between UTF-8 and
    // UTF-16 when interacting with the Windows API.
#   error Not implemented yet.

#else // _WIN32

    DIR* dir = opendir(path.length() > 0 ? path.c_str() : ".");
    if (!dir) return entries; // TODO: Throw exception instead

    struct dirent* entry;
    struct stat statbuf;

    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, "." ) == 0 ||
            strcmp(entry->d_name, "..") == 0) continue;

        if (entry->d_type == DT_UNKNOWN) {
            // Directory entry type is unknown. The file system is not required
            // to provide this information. Thus, we need to figure it out by
            // using stat.
            if (fstatat(dirfd(dir), entry->d_name, &statbuf, 0) == 0) {
                switch (statbuf.st_mode & S_IFMT) {
                    case S_IFIFO:  entry->d_type = DT_FIFO; break;
                    case S_IFCHR:  entry->d_type = DT_CHR;  break;
                    case S_IFDIR:  entry->d_type = DT_DIR;  break;
                    case S_IFBLK:  entry->d_type = DT_BLK;  break;
                    case S_IFREG:  entry->d_type = DT_REG;  break;
                    case S_IFLNK:  entry->d_type = DT_LNK;  break;
                    case S_IFSOCK: entry->d_type = DT_SOCK; break;
                }
            }
        }

        entries.push_back(DirEntry { entry->d_name, entry->d_type == DT_DIR });
    }

#endif // !_WIN32

    // Sort the entries. The order in which directories are listed is not
    // guaranteed to be deterministic.
    std::sort(entries.begin(), entries.end());

    return entries;
}

/**
 * Returns true if the given string contains a glob pattern.
 */
bool isGlobPattern(Path p) {
    for (size_t i = 0; i < p.length; ++i) {
        switch (p.path[i]) {
            case '?':
            case '*':
            case '[':
                return true;
        }
    }

    return false;
}

/**
 * Returns true if the given path element is a recursive glob pattern.
 */
bool isRecursiveGlob(Path p) {
    return p.length == 2 && p.path[0] == '*' && p.path[1] == '*';
}

struct GlobClosure {

    // Directory listing cache.
    DirCache* dirCache;

    // Root directory we are globbing from.
    Path root;

    Path pattern;

    // Next callback
    GlobCallback next;
    void* nextData;
};

void globCallback(Path path, bool isDir, void* data) {
    if (isDir) {
        const GlobClosure* c = (const GlobClosure*)data;
        c->dirCache->glob(c->root, path, c->pattern, c->next, c->nextData);
    }
}

}

/**
 * Helper function for listing a directory with the given pattern. If the
 * pattern is empty,
 */
void DirCache::glob(Path root, Path path, Path pattern,
          GlobCallback callback, void* data) {

    std::string buf(path.path, path.length);

    if (pattern.length == 0) {
        path::join(buf, pattern);
        callback(Path(buf.data(), buf.size()), true, data);
        return;
    }

    for (auto&& entry: dirEntries(root, path)) {
        if (globMatch(entry.name, pattern)) {
            path::join(buf, entry.name);

            callback(Path(buf.data(), buf.size()), entry.isDir, data);

            buf.assign(path.path, path.length);
        }
    }
}

/**
 * Helper function to recursively yield directories for the given path.
 */
void DirCache::globRecursive(Path root, std::string& path,
        GlobCallback callback, void* data) {

    size_t len = path.size();

    // "**" matches 0 or more directories and thus includes this one.
    callback(Path(path.data(), path.size()), true, data);

    for (auto&& entry: dirEntries(root, path)) {
        path::join(path, entry.name);

        callback(Path(path.data(), path.size()), entry.isDir, data);

        if (entry.isDir)
            globRecursive(root, path, callback, data);

        path.resize(len);
    }
}

/**
 * Glob a directory.
 */
void DirCache::glob(Path root, Path path, GlobCallback callback, void* data) {

    path::Split s = path::split(path);

    if (isGlobPattern(s.head)) {
        // Directory name contains a glob pattern

        GlobClosure c;
        c.dirCache = this;
        c.root = root;
        c.pattern = s.tail;
        c.next = callback;
        c.nextData = data;

        glob(root, s.head, &globCallback, &c);
    }
    else if (isRecursiveGlob(s.tail)) {
        std::string buf(s.head.path, s.head.length);
        globRecursive(root, buf, callback, data);
    }
    else if (isGlobPattern(s.tail)) {
        // Only base name contains a glob pattern.
        glob(root, s.head, s.tail, callback, data);
    }
    else {
        // No glob pattern in this path.
        if (s.tail.length) {
            // TODO: Only yield this path if the file exists.
            callback(path, false, data);
        }
        else {
            // TODO: Only yield this path if the directory exists.
            callback(s.head, true, data);
        }
    }
}

DirCache::DirCache(ImplicitDeps* deps) : _deps(deps) {}

const DirEntries& DirCache::dirEntries(Path root, Path dir) {
    std::string buf(root.path, root.length);
    path::join(buf, dir);
    return dirEntries(buf);
}

const DirEntries& DirCache::dirEntries(const std::string& path) {

    // TODO: Normalize the input path.

    // Did we already do the work?
    const auto it = cache.find(path);
    if (it != cache.end())
        return it->second;

    if (_deps) _deps->addInput(path.data(), path.length());

    // List the directories, cache it, and return the cached list.
    return cache.insert(
            std::pair<std::string, DirEntries>(path, ::dirEntries(path))
            ).first->second;
}