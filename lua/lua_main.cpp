//
// Created by abcdefghkj on 17.05.2026.
//
#include "lua_main.h"
#include "../hooks/lua_load.h"
#include "../hooks/lua_callback.h"
#include "../print.h"



namespace main {
    static bool registered = false;

    


    bool onCustomCommand(cbctx& ctx) {



        const char* full = ctx.str(0);
        int64_t peer = ctx.i(1);
        bool admin = ctx.b(2);
        bool auth = ctx.b(3);
        const char* cmd = ctx.str(4);


        if (strcmp(cmd, "?test1") == 0) {
            luares out{};

            ctx.lua.server("announce", { luaarg::str("api test"), luaarg::str("start")}, out, 0);
            ctx.lua.server("notify", {luaarg::num(-1), luaarg::str("notify"), luaarg::str("notify"), luaarg::num(8)}, out, 1);






            if (ctx.lua.server("getPlayers", {}, out, 1)) {
               ctx.lua.each(ctx, &out.v[0], [](cbctx& ctx, __int64 idx, sw_tvalue* row) -> void {
                   luares out{};
                   if (!lua::ist(row))
                       return;

                   int64_t id{};
                   const char* name{};

                   ctx.lua.get_i(row, "id", id);
                   ctx.lua.get_s(row, "name", name);

                   ctx.lua.server("announce", {luaarg::str("name"), luaarg::str(name)}, out, 0);

                   ctx.lua.server("addAdmin", {luaarg::num(id)}, out, 0);



                   ctx.lua.server("getTimeMillisec", {}, out, 1);

                   char buf[128];
                   sprintf_s(buf, 128, "%d", out.v[0].v);
                   ctx.lua.server("announce", {luaarg::str("time millisec"), luaarg::str(buf)}, out, 0);

               });
            }



            ctx.lua.server("addMapLabel", { luaarg::num(-1), luaarg::num(2), luaarg::num(4), luaarg::str("test"), luaarg::num(100), luaarg::num(150) }, out, 1);
            ctx.lua.server("setGameSetting", { luaarg::str("settings_menu"), luaarg::boolean(1) }, out, 1);



            luares mat{};
            ctx.lua.matrix("translation", {luaarg::num(100), luaarg::num(100), luaarg::num(100)}, mat, 1);


            ctx.lua.server("spawnExplosion", {luaarg::val(&mat.v[0]),luaarg::num(1)}, out, 1);


  

            return false;
        }



        return true;
    }

    bool onTick(cbctx& ctx) {
        luares out{};


        return true;
    }

    void init(sw_state* lua) {
        if (registered)
            return;

        registered = true;

        g_cb.on("onCustomCommand", onCustomCommand);
        g_cb.on("onTick", onTick);
    }
}

