//
// Created by abcdefghkj on 17.05.2026.
//

#pragma once
#include "swlua.hpp"
#include "../hooks/lua_callback.h"

struct component_entry {
    std::string name;
    int64_t id;
    sw_tvalue v;
};

namespace main {
    void init(sw_state* lua);
    bool onCustomCommand(cbctx& ctx);
}