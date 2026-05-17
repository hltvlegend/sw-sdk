#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <initializer_list>

struct cbctx;

#pragma pack(push, 1)

struct sw_tvalue {
    union {
        void* gc;
        double n;
        int64_t i;
        uint64_t raw;
        int32_t b;
    } v;
    uint64_t tt;
};

struct sw_node {
    sw_tvalue val;
    uint64_t key_value;
    uint32_t key_tt;
    int32_t next;
};

struct sw_table {
    void* next;
    uint8_t tt;
    uint8_t marked;
    uint8_t flags;
    uint8_t lsizenode;
    uint32_t alimit;
    sw_tvalue* array;
    sw_node* node;
    sw_node* lastfree;
    sw_table* metatable;
    void* gclist;
};

struct sw_gstate {
    uint8_t pad00[0x40];
    sw_table* registry;
};

struct sw_ci {
    sw_tvalue* func;
    sw_tvalue* top;
};

struct sw_state {
    uint8_t pad00[0x10];
    sw_tvalue* top;
    sw_gstate* g;
    sw_ci* ci;
    uint8_t pad28[0x08];
    sw_tvalue* stack_last;
    sw_tvalue* stack;
};

struct sw_call {
    sw_tvalue* func;
    int32_t nres;
    int32_t pad;
};

#pragma pack(pop)

static_assert(sizeof(sw_tvalue) == 0x10);
static_assert(sizeof(sw_node) == 0x20);
static_assert(offsetof(sw_table, alimit) == 0x0c);
static_assert(offsetof(sw_table, array) == 0x10);
static_assert(offsetof(sw_table, node) == 0x18);
static_assert(offsetof(sw_gstate, registry) == 0x40);
static_assert(offsetof(sw_state, top) == 0x10);
static_assert(offsetof(sw_state, g) == 0x18);

struct luaapi {
    using getfield_t = int64_t(__fastcall*)(sw_state* l, sw_tvalue* table, const char* key);
    using check_t = bool(__fastcall*)(sw_state* l, int count);
    using pcall_t = uint64_t(__fastcall*)(sw_state* l, void* tramp, sw_call* call);
    using pushstr_t = const char* (__fastcall*)(sw_state* l, const char* s);
    using pushmat_t = int64_t(__fastcall*)(sw_state* l, const double* m);

    getfield_t getfield{};
    check_t check{};
    pcall_t pcall{};
    pushstr_t pushstr{};
    pushmat_t pushmat{};
    void* tramp{};
};

enum class luakind {
    nil,
    boolean,
    number,
    integer,
    string,
    value,
    matrix,
};

struct luaarg {
    luakind kind{ luakind::nil };
    union {
        bool b;
        double n;
        int64_t i;
        const char* s;
        const sw_tvalue* tv;
        const double* m;
    } v{};

    static luaarg nil() {
        return {};
    }

    static luaarg boolean(bool x) {
        luaarg a{};
        a.kind = luakind::boolean;
        a.v.b = x;
        return a;
    }

    static luaarg num(double x) {
        luaarg a{};
        a.kind = luakind::number;
        a.v.n = x;
        return a;
    }

    static luaarg integer(int64_t x) {
        luaarg a{};
        a.kind = luakind::integer;
        a.v.i = x;
        return a;
    }

    static luaarg str(const char* x) {
        luaarg a{};
        a.kind = luakind::string;
        a.v.s = x;
        return a;
    }

    static luaarg val(const sw_tvalue* x) {
        luaarg a{};
        a.kind = luakind::value;
        a.v.tv = x;
        return a;
    }

    static luaarg mat(const double* x) {
        luaarg a{};
        a.kind = luakind::matrix;
        a.v.m = x;
        return a;
    }


};

struct luares {
    sw_tvalue v[16]{};
    int n{};
};

struct player {
    int64_t id{};
    const char* name{};
    bool admin{};
    bool auth{};
    int64_t steam{};
    int64_t obj{ -1 };
};

class lua {
public:
    lua(sw_state* state, luaapi api)
        : l_(state), api_(api) {
    }

    sw_state* state() const {
        return l_;
    }

    void state(sw_state* l) {
        l_ = l;
    }

    void bind(sw_state* l, luaapi api) {
        l_ = l;
        api_ = api;
    }


    sw_tvalue* top() const {
        return l_ ? l_->top : nullptr;
    }

    void top(sw_tvalue* t) const {
        if (l_)
            l_->top = t;
    }

    bool chk(int n) const {
        return api_.check ? api_.check(l_, n) : true;
    }

    sw_tvalue* reg(int64_t key) const {
        if (!l_ || !l_->g || !l_->g->registry)
            return nullptr;

        return tabint(l_->g->registry, key);
    }

    sw_tvalue* glob() const {
        return reg(2);
    }

    sw_tvalue* get(sw_tvalue* table, const char* key) const {
        if (!l_ || !table || !api_.getfield)
            return nullptr;

        sw_tvalue* old = l_->top;
        api_.getfield(l_, table, key);
        return old;
    }

    sw_tvalue* gets(sw_tvalue* table, const char* key) const {
        sw_tvalue* old = l_->top;
        sw_tvalue* r = get(table, key);
        l_->top = old;
        return r;
    }

    bool get_i(sw_tvalue* table, const char* key, int64_t& out) const {
        sw_tvalue* old = l_->top;
        sw_tvalue* r = get(table, key);
        bool ok = toi(r, out);
        l_->top = old;
        return ok;
    }

    bool get_b(sw_tvalue* table, const char* key, bool& out) const {
        sw_tvalue* old = l_->top;
        sw_tvalue* r = get(table, key);
        bool ok = isb(r);
        if (ok)
            out = r->v.b != 0;
        l_->top = old;
        return ok;
    }

    bool get_s(sw_tvalue* table, const char* key, const char*& out) const {
        sw_tvalue* old = l_->top;
        sw_tvalue* r = get(table, key);
        bool ok = iss(r);
        if (ok)
            out = str(r);
        l_->top = old;
        return ok;
    }

    bool call(sw_tvalue* func, int nres = 1) const {
        if (!l_ || !func || !api_.pcall || !api_.tramp)
            return false;

        sw_call c{};
        c.func = func;
        c.nres = nres;

        uint64_t* err = (uint64_t*)((uint8_t*)l_ + 0xb0);
        uint64_t old = *err;
        *err = 0;

        uint64_t status = api_.pcall(l_, api_.tramp, &c);

        *err = old;
        return status == 0;
    }

    bool push(const luaarg& a) const {
        if (!l_)
            return false;

        if (!chk(1))
            return false;

        sw_tvalue* t = l_->top;

        switch (a.kind) {
        case luakind::nil:
            t->v.raw = 0;
            t->tt = 0;
            l_->top = t + 1;
            return true;
        case luakind::boolean:
            t->v.b = a.v.b ? 1 : 0;
            t->tt = 1;
            l_->top = t + 1;
            return true;
        case luakind::number:
            t->v.n = a.v.n;
            t->tt = 3;
            l_->top = t + 1;
            return true;
        case luakind::integer:
            t->v.i = a.v.i;
            t->tt = 0x13;
            l_->top = t + 1;
            return true;
        case luakind::string:
            if (!api_.pushstr)
                return false;
            api_.pushstr(l_, a.v.s);
            return true;
        case luakind::value:
            if (!a.v.tv)
                return false;
            *t = *a.v.tv;
            l_->top = t + 1;
            return true;
        case luakind::matrix:
            if (!api_.pushmat || !a.v.m)
                return false;
            api_.pushmat(l_, a.v.m);
            return true;
        default:
            return false;
        }
    }

    bool callv(sw_tvalue* func, const luaarg* args, int argc, luares& out, int nres = 1) const {
        if (!l_ || !func || argc < 0 || nres < 0 || nres > 16)
            return false;

        sw_tvalue* base = func;

        if (!chk(argc + nres + 4)) {
            l_->top = base;
            return false;
        }

        for (int i = 0; i < argc; ++i) {
            if (!push(args[i])) {
                l_->top = base;
                return false;
            }
        }

        if (!call(func, nres)) {
            l_->top = base;
            return false;
        }

        out.n = nres;
        for (int i = 0; i < nres; ++i)
            out.v[i] = base[i];

        l_->top = base;
        return true;
    }

    bool fn(const char* lib, const char* name, sw_tvalue*& func) const {
        sw_tvalue* base = l_->top;
        sw_tvalue* g = glob();
        if (!ist(g))
            return false;

        get(g, lib);
        sw_tvalue* libv = base;
        if (!ist(libv)) {
            l_->top = base;
            return false;
        }

        get(libv, name);
        if (!isf(base + 1)) {
            l_->top = base;
            return false;
        }

        base[0] = base[1];
        l_->top = base + 1;
        func = base;
        return true;
    }

    bool gfn(const char* name, sw_tvalue*& func) const {
        sw_tvalue* base = l_->top;
        sw_tvalue* g = glob();
        if (!ist(g))
            return false;

        get(g, name);
        if (!isf(base)) {
            l_->top = base;
            return false;
        }

        func = base;
        return true;
    }

    bool callg(const char* name, const luaarg* args, int argc, luares& out, int nres = 1) const {
        sw_tvalue* base = l_->top;
        sw_tvalue* f = nullptr;

        if (!gfn(name, f)) {
            l_->top = base;
            return false;
        }

        return callv(f, args, argc, out, nres);
    }

    bool calllib(
        const char* lib,
        const char* name,
        std::initializer_list<luaarg> args,
        luares& out,
        int nres = 1
    ) const {
        sw_tvalue* base = l_->top;
        sw_tvalue* f = nullptr;

        if (!fn(lib, name, f)) {
            l_->top = base;
            return false;
        }

        return callv(f, args.begin(), (int)args.size(), out, nres);
    }

    bool server(
        const char* name,
        std::initializer_list<luaarg> args,
        luares& out,
        int nres = 1
    ) const {
        return calllib("server", name, args, out, nres);
    }

    bool matrix(
        const char* name,
        std::initializer_list<luaarg> args,
        luares& out,
        int nres = 1
    ) const {
        return calllib("matrix", name, args, out, nres);
    }

    template <class fn>
    bool each(cbctx& ctx, sw_tvalue* tv, fn cb) const {
        if (!ist(tv))
            return false;

        auto* t = (sw_table*)tv->v.gc;
        if (!t)
            return false;

        if (t->array) {
            for (uint32_t i = 0; i < t->alimit; ++i) {
                sw_tvalue* v = &t->array[i];
                if (!isn(v))
                    cb(ctx, (int64_t)i + 1, v);
            }
        }

        if (t->node && t->lsizenode < 31) {
            uint32_t nnode = 1u << t->lsizenode;

            for (uint32_t i = 0; i < nnode; ++i) {
                sw_node* n = &t->node[i];
                if (n->val.tt == 0)
                    continue;

                if (n->key_tt != 0x13)
                    continue;

                cb(ctx, (int64_t)n->key_value, &n->val);
            }
        }

        return true;
    }



    static bool isn(const sw_tvalue* v) {
        return !v || ((v->tt & 0x0f) == 0);
    }

    static bool isb(const sw_tvalue* v) {
        return v && ((v->tt & 0x0f) == 1);
    }

    static bool isnum(const sw_tvalue* v) {
        return v && ((v->tt & 0x0f) == 3);
    }

    static bool iss(const sw_tvalue* v) {
        return v && ((v->tt & 0x0f) == 4);
    }

    static bool ist(const sw_tvalue* v) {
        return v && ((v->tt & 0x0f) == 5);
    }

    static bool isf(const sw_tvalue* v) {
        return v && ((v->tt & 0x0f) == 6);
    }

    static bool isi(const sw_tvalue* v) {
        return v && ((v->tt & 0xff) == 0x13);
    }

    static const char* str(const sw_tvalue* v) {
        return iss(v) ? (const char*)((const uint8_t*)v->v.gc + 0x18) : nullptr;
    }

    static bool toi(const sw_tvalue* v, int64_t& out) {
        if (isi(v)) {
            out = v->v.i;
            return true;
        }

        if (isnum(v)) {
            out = (int64_t)v->v.n;
            return true;
        }

        return false;
    }

    static bool tod(const sw_tvalue* v, double& out) {
        if (isnum(v)) {
            out = v->v.n;
            return true;
        }

        if (isi(v)) {
            out = static_cast<double>(v->v.i);
            return true;
        }

        return false;
    }


private:


    static sw_tvalue* tabint(sw_table* table, int64_t key) {
        if (!table)
            return nullptr;

        if (key >= 1 && (uint64_t)key <= table->alimit)
            return &table->array[key - 1];

        if (!table->node)
            return nullptr;

        uint32_t mask = (1u << table->lsizenode) - 1u;
        sw_node* node = &table->node[key & mask];

        while (true) {
            if (node->key_tt == 0x13 && (int64_t)node->key_value == key)
                return &node->val;

            if (!node->next)
                return nullptr;

            node += node->next;
        }
    }

    sw_state* l_{};
    luaapi api_{};
};
