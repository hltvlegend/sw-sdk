#include "lua_callback.h"

#include "hooks.h"
#include "lua_load.h"
#include "../print.h"
#include <cstring>

bool cbctx::is(const char* other) const {
    return name && other && std::strcmp(name, other) == 0;
}

const char* cbctx::str(uint32_t index, const char* fallback) const {
    cbarg* a = at(index);
    if (!a || a->type != 2 || !a->string)
        return fallback;

    return a->string;
}

int64_t cbctx::i(uint32_t index, int64_t fallback) const {
    cbarg* a = at(index);
    if (!a)
        return fallback;

    if (a->type == 1)
        return a->integer;

    if (a->type == 4)
        return a->id;

    if (a->type == 0)
        return (int64_t)a->number;

    return fallback;
}

double cbctx::num(uint32_t index, double fallback) const {
    cbarg* a = at(index);
    if (!a)
        return fallback;

    if (a->type == 0)
        return a->number;

    if (a->type == 1)
        return (double)a->integer;

    if (a->type == 4)
        return (double)a->id;

    return fallback;
}

bool cbctx::b(uint32_t index, bool fallback) const {
    cbarg* a = at(index);
    if (!a || a->type != 3)
        return fallback;

    return a->boolean;
}

const double* cbctx::mat(uint32_t index) const {
    cbarg* a = at(index);
    if (!a || a->type != 5)
        return nullptr;

    return a->matrix;
}

void cb::on(const char* name, fn f) {
    if (!name || !f)
        return;

    for (auto& s : slots_) {
        if (!s.post && s.name == name) {
            s.f = f;
            return;
        }
    }

    slots_.push_back({ std::string(name), f, false });
}

void cb::after(const char* name, fn f) {
    if (!name || !f)
        return;

    for (auto& s : slots_) {
        if (s.post && s.f == f && s.name == name)
            return;
    }

    slots_.push_back({ std::string(name), f, true });
}

bool cb::fire(cbctx& ctx, bool post) {
    bool pass = true;

    for (auto& s : slots_) {
        if (s.post != post)
            continue;

        if (!ctx.name)
            continue;

        if (s.name != ctx.name)
            continue;

        if (!s.f)
            continue;

        if (!s.f(ctx))
            pass = false;
    }

    return pass;
}

namespace lua_callback {
    static int64_t last_args = 0;

    char __fastcall hk(int64_t script, int64_t* name, int64_t args) {
        auto* L = (sw_state*)*(int64_t*)(script + 0x08);
        const char* cbname = name ? (const char*)*name : nullptr;
        auto* cargs = (cbargs*)args;

        if (args == last_args) {
            return g_hooks.get("lua_callback")->inl.fastcall<char>(script, name, args);
        }

        last_args = args;

        cbctx ctx(script, L, cbname, cargs, g_lua_api);

        bool pass = g_cb.fire(ctx, false);


        if (!pass)
            return 1;

        auto res = g_hooks.get("lua_callback")->inl.fastcall<char>(script, name, args);

        g_cb.fire(ctx, true);
        return res;
    }

}
