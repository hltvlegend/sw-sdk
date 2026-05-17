#pragma once

#include "../lua/swlua.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#pragma pack(push, 1)

struct cbarg {
    int32_t type;
    uint8_t pad04[0x04];
    double matrix[16];
    double number;
    int64_t integer;
    int64_t id;
    const char* string;
    uint8_t pad_a8[0x08];
    bool boolean;
    uint8_t pad_b1[0x07];
};

struct cbargs {
    cbarg* data;
    uint32_t cap;
    uint32_t head;
    uint32_t count;

    cbarg* at(uint32_t index) const {
        if (!data || !cap || index >= count)
            return nullptr;

        return &data[(head + index) % cap];
    }
};

#pragma pack(pop)

static_assert(sizeof(cbarg) == 0xb8);
static_assert(offsetof(cbarg, matrix) == 0x08);
static_assert(offsetof(cbarg, number) == 0x88);
static_assert(offsetof(cbarg, integer) == 0x90);
static_assert(offsetof(cbarg, id) == 0x98);
static_assert(offsetof(cbarg, string) == 0xa0);
static_assert(offsetof(cbarg, boolean) == 0xb0);
static_assert(offsetof(cbargs, data) == 0x00);
static_assert(offsetof(cbargs, cap) == 0x08);
static_assert(offsetof(cbargs, head) == 0x0c);
static_assert(offsetof(cbargs, count) == 0x10);



struct cbctx {
    int64_t script{};
    sw_state* l{};
    const char* name{};
    cbargs* args{};
    lua lua;

    cbctx(int64_t script, sw_state* l, const char* name, cbargs* args, luaapi api)
        : script(script), l(l), name(name), args(args), lua(l, api) {}

    cbarg* at(uint32_t index) const {
        return args ? args->at(index) : nullptr;
    }

    uint32_t n() const {
        return args ? args->count : 0;
    }

    bool is(const char* other) const;
    const char* str(uint32_t index, const char* fallback = "") const;
    int64_t i(uint32_t index, int64_t fallback = 0) const;
    double num(uint32_t index, double fallback = 0.0) const;
    bool b(uint32_t index, bool fallback = false) const;
    const double* mat(uint32_t index) const;
};

class cb {
public:
    using fn = bool(*)(cbctx& ctx);

    void on(const char* name, fn f);
    void after(const char* name, fn f);
    bool fire(cbctx& ctx, bool post);

private:
    struct slot {
        std::string name;
        fn f{};
        bool post{};
    };

    std::vector<slot> slots_;
};
inline cb g_cb;

namespace lua_callback {
    char __fastcall hk(int64_t script, int64_t* name, int64_t args);
}
