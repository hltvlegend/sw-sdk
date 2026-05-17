//
// Created by abcdefghkj on 15.05.2026.
//

#include "hooks.h"

#include "lua_load.h"
#include "lua_callback.h"
#include "../memory/mem.h"
#include "../lua/swlua.hpp"
#include "../print.h"

void hooks::add(hook* obj, const std::string& name) {
    obj->id = hook_pool.size() + 1;

    hook_pool.push_back(obj);
    name_pool.push_back({ name, obj->id });

    print("added new hook %s\n", name.c_str());
}


hook* hooks::get(const std::string& name) {
    size_t idx = 0;

    for (const auto& a : name_pool) {
        if (a.name == name) {
            idx = a.id;
            break;
        }
    }

    if (idx != 0) {
        for (auto* a : hook_pool) {
            if (a && a->id == idx)
                return a;
        }
    }

    print("failed to find %s entry\n", name.c_str());
    return nullptr;
}

/*
 * main hooking func
 * do not create 2 vmts of the same vmt instance
 *
 */


void hooks::init() {
/*
    auto server_tick = new hook;
    server_tick->inl = safetyhook::create_inline(memory::FindPattern("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 E1 48 81 EC 88 00 00 00 0F 29 74 24 70 4D").value(), server_tick::hk);
    add(server_tick, "server_tick");


    auto paint_traverse = new hook;
    paint_traverse->inl = safetyhook::create_inline(memory::FindPattern("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 50 F3 0F 10 8A").value(), paint_traverse::hk);
    add(paint_traverse, "paint_traverse");
*/



    g_lua_api.pcall = reinterpret_cast<luaapi::pcall_t>(memory::FindPattern("40 53 48 81 EC 70").value());
    g_lua_api.getfield = reinterpret_cast<luaapi::getfield_t>(memory::FindPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B FA 48 8B D9 49").value());
    g_lua_api.check = reinterpret_cast<luaapi::check_t>(memory::FindPattern("48 89 5C 24 08 89 54 24 10 57 48 83 EC 20 4C 8B 49 10 48").value());
    g_lua_api.pushstr = reinterpret_cast<luaapi::pushstr_t>(memory::FindPattern("48 89 5C 24 08 57 48 83 EC 20 48 8B FA 48 8B D9 48 85 D2").value());
    g_lua_api.pushmat = reinterpret_cast<luaapi::pushmat_t>(memory::FindPattern("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 83 EC 20 48 8B FA 48 8B D9").value());
    g_lua_api.tramp = reinterpret_cast<void*>(memory::FindPattern("40 53 48 83 EC 20 44 8B 42 08 48 8B D9 48 8B 12 66 FF 81 C4 00 00 00").value());

    print("lua api pcall=%p getfield=%p check=%p pushstr=%p pushmat=%p tramp=%p\n",
        (void*)g_lua_api.pcall,
        (void*)g_lua_api.getfield,
        (void*)g_lua_api.check,
        (void*)g_lua_api.pushstr,
        (void*)g_lua_api.pushmat,
        g_lua_api.tramp);


    auto lua_load = new hook;
    lua_load->inl = safetyhook::create_inline(memory::FindPattern("40 55 56 48 83 EC 58").value(), lua_load::hk);
    add(lua_load, "lua_load");

    auto lua_callback = new hook;
    lua_callback->inl = safetyhook::create_inline(memory::FindPattern("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 54 41 55 41 56 41 57 48 83 EC 30 4C").value(), lua_callback::hk);
    add(lua_callback, "lua_callback");



    printf("hook ok\n");
}
