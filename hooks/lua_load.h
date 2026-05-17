//
// Created by abcdefghkj on 16.05.2026.
//

#pragma once
#include "../lua/swlua.hpp"
#include <cstdint>

inline luaapi g_lua_api;
inline lua g_lua(nullptr, g_lua_api);

namespace lua_load {
    char __fastcall hk(__int64 a1, char** a2);
}
