
#include "hooks.h"
#include "lua_load.h"
#include "../lua/swlua.hpp"
#include "lua_callback.h"
#include "../lua/lua_main.h"
#include "../print.h"

namespace lua_load {

    char __fastcall hk(__int64 lua_script, char** code) {
        auto* L = (sw_state*)*(__int64*)(lua_script + 0x08);

        auto res = g_hooks.get("lua_load")->inl.fastcall<char>(lua_script, code);

        print("loaded new state -> %p\n", L);

        g_lua.state(L);
        main::init(L);



        return res;

    }

}
