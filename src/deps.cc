/**
 * Copyright (c) Jason White
 *
 * MIT License
 *
 * Description:
 * Handles sending dependencies to parent build system.
 */
#include "deps.h"

#include <stdlib.h>

ImplicitDeps::ImplicitDeps() : _f_inputs(NULL), _f_outputs(NULL) {
    const char* var;
    int fd;

    var = getenv("BUTTON_INPUTS");
    if (var && (fd = atoi(var)))
        _f_inputs = fdopen(fd, "a");

    var = getenv("BUTTON_OUTPUTS");
    if (var && (fd = atoi(var)))
        _f_outputs = fdopen(fd, "a");
}

ImplicitDeps::~ImplicitDeps() {
    if (_f_inputs)  fclose(_f_inputs);
    if (_f_outputs) fclose(_f_outputs);
}

bool ImplicitDeps::hasParent() const {
    return _f_inputs != NULL || _f_outputs != NULL;
}

void ImplicitDeps::addInput(const Dependency& dep) {
    if (!_f_inputs) return;

    fwrite(&dep, sizeof(dep) + dep.length, 1, _f_inputs);
}

void ImplicitDeps::addOutput(const Dependency& dep) {
    if (!_f_outputs) return;

    fwrite(&dep, sizeof(dep) + dep.length, 1, _f_outputs);
}

void ImplicitDeps::addInput(const char* name, size_t length) {
    if (!_f_inputs) return;

    if (length > UINT32_MAX)
        length = UINT32_MAX;

    Dependency dep = {0};
    dep.length = (uint32_t)length;

    fwrite(&dep, sizeof(dep), 1, _f_inputs);
    fwrite(name, 1, dep.length, _f_inputs);
}

void ImplicitDeps::addOutput(const char* name, size_t length) {
    if (!_f_outputs) return;

    if (length > UINT32_MAX)
        length = UINT32_MAX;

    Dependency dep = {0};
    dep.length = (uint32_t)length;

    fwrite(&dep, sizeof(dep), 1, _f_outputs);
    fwrite(name, 1, dep.length, _f_outputs);
}
