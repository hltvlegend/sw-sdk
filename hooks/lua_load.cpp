
#include "hooks.h"
#include "lua_load.h"
#include "../lua/swlua.hpp"
#include "lua_callback.h"
#include "../lua/lua_main.h"
#include "../print.h"

namespace lua_load {
    static sw_state* state = nullptr; // check whether the state is valid or not to save it

    char __fastcall hk(__int64 lua_script, char** code) {
        auto* L = (sw_state*)*(__int64*)(lua_script + 0x08);

        auto res = g_hooks.get("lua_load")->inl.fastcall<char>(lua_script, code);


        g_lua.bind(L, g_lua_api);

        luares out{};
        if (g_lua.server("getPlayers", {}, out, 1) && out.n > 0) {
            print("captured new valid state -> %p \n", L);
            main::init(L);
            state = L;
        }

        g_lua.bind(state, g_lua_api);

        return res;

    }

}
