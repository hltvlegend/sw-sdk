//
// Created by abcdefghkj on 17.05.2026.
//

#pragma once
#include "swlua.hpp"
#include "../hooks/lua_callback.h"

namespace main {
    void init(sw_state* lua);
    bool onCustomCommand(cbctx& ctx);
}
