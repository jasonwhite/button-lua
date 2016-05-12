/**
 * Copyright (c) Jason White
 *
 * MIT License
 *
 * Description:
 * Handles writing out rules.
 */
#pragma once

#include <stdio.h>

struct lua_State;

namespace buttonlua {

class Rules
{
private:
    // File handle to write to.
    FILE* _f;

    // Number of rules.
    size_t _n;

public:
    Rules(FILE* f);
    ~Rules();

    /**
     * Outputs a rule to the file.
     */
    int add(lua_State *L);

private:
    int stringToJSON(lua_State* L, const char* field, size_t i);
    int listToJSON(lua_State* L, const char* field, size_t i);
};


}
