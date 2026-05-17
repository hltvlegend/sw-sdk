# C++ SDK for Stormworks: Build and Rescue Addon Lua

Experimental C++ SDK for running native code together with the Stormworks addon Lua environment.

The main idea is to keep Stormworks addon callbacks as the entry point, but handle the heavy or inconvenient logic in C++. The SDK gets access to the active addon Lua state, receives callback data, and lets C++ code call Stormworks API functions such as `server.*` and `matrix.*`.

This is useful when addon Lua is too limited for communication, persistence, debugging, or performance-sensitive logic.

## Core idea

Stormworks normally runs addon code through Lua callbacks:

```lua
function onTick(game_ticks)
end

function onCustomCommand(full_message, peer_id, is_admin, is_auth, command)
end
```

This SDK hooks into that flow and exposes the same callback events to C++:

```cpp
g_cb.on("onCustomCommand", onCustomCommand);
g_cb.on("onTick", onTick);
```

A C++ handler receives a `cbctx&`. From that context you can:

- read callback arguments;
- call Stormworks Lua API functions;
- block the original Lua callback if needed;
- run logic before or after the Lua callback.

## Why use this instead of only addon lua

Stormworks addon Lua is convenient, but it is still sandboxed Lua. Native C++ gives more control.

Main advantages:

- access to C++ libraries for HTTP, sockets, databases, files, JSON, logging, etc.;
- easier integration with external tools and local services;
- better performance for heavy calculations and large data processing;
- stronger typing and better project structure than large Lua scripts;
- easier debugging through native debuggers and `OutputDebugStringA`;
- direct access to Stormworks `server` and `matrix` API calls from C++;
- ability to keep Lua as a thin game-side layer while moving complex systems into native code.

Native code does not make every Stormworks API call free. Calls to `server.*` and `matrix.*` still go through the game's Lua API. The performance benefit comes from moving your own logic out of Lua.

## Basic usage

Register handlers in `main::init`:

```cpp
void init(sw_state* lua) {
    g_cb.on("onCustomCommand", onCustomCommand);
    g_cb.on("onTick", onTick);
}
```

Handle a callback:

```cpp
bool onCustomCommand(cbctx& ctx) {
    const char* full = ctx.str(0);
    int64_t peer = ctx.i(1);
    bool admin = ctx.b(2);
    bool auth = ctx.b(3);
    const char* cmd = ctx.str(4);

    if (strcmp(cmd, "?test") != 0)
        return true;

    luares out{};
    ctx.lua.server("announce", {
        luaarg::str("SDK"),
        luaarg::str("Hello from C++")
    }, out, 0);

    return false;
}
```

Return value:

- `true` lets the original Lua callback continue;
- `false` blocks the original Lua callback for this event.

You can also register post-callback handlers:

```cpp
g_cb.after("onTick", afterTick);
```

## Calling Stormworks API

Use `ctx.lua.server(...)` for `server.*` functions:

```cpp
luares out{};

ctx.lua.server("notify", {
    luaarg::num(-1),
    luaarg::str("Title"),
    luaarg::str("Message"),
    luaarg::num(8)
}, out, 1);
```

Use `ctx.lua.matrix(...)` for `matrix.*` functions:

```cpp
luares mat{};

ctx.lua.matrix("translation", {
    luaarg::num(100),
    luaarg::num(50),
    luaarg::num(200)
}, mat, 1);
```

You can pass returned Lua values back into another call:

```cpp
luares mat{};
luares out{};

ctx.lua.matrix("translation", {
    luaarg::num(100),
    luaarg::num(50),
    luaarg::num(200)
}, mat, 1);

ctx.lua.server("spawnExplosion", {
    luaarg::val(&mat.v[0]),
    luaarg::num(1)
}, out, 0);
```

## Important types

### `cbctx`

Callback context. It contains callback name, arguments, Lua state, and the `lua` wrapper.

Common helpers:

```cpp
ctx.is("onCustomCommand");
ctx.n();
ctx.str(index);
ctx.i(index);
ctx.num(index);
ctx.b(index);
ctx.mat(index);
```

Use these helpers instead of reading raw callback memory directly.

### `luaarg`

Argument wrapper used when passing values from C++ into Lua API calls.

Supported constructors:

```cpp
luaarg::nil();
luaarg::boolean(true);
luaarg::num(123.0);
luaarg::integer(123);
luaarg::str("text");
luaarg::val(&some_sw_tvalue);
luaarg::mat(matrix_ptr);
```

### `luares`

Container for values returned from a Lua API call.

```cpp
struct luares {
    sw_tvalue v[16];
    int n;
};
```

Example:

```cpp
luares out{};

if (ctx.lua.server("getPlayers", {}, out, 1)) {
    sw_tvalue* players = &out.v[0];
}
```

`n` is the number of expected return values requested by the call. The returned values are stored in `v`.

### `sw_tvalue`

Raw Stormworks/Lua value.

It can represent Lua values such as:

- nil;
- boolean;
- number;
- integer;
- string;
- table;
- function;
- matrix/userdata-like values used by the game.

Use `sw_tvalue` when you need to store or pass a Lua value without converting it to a C++ type.

For example, `matrix.translation(...)` returns a Lua value. You can pass it directly to `server.spawnExplosion(...)` with:

```cpp
luaarg::val(&mat.v[0])
```

For tables, use wrapper helpers instead of manually decoding memory:

```cpp
ctx.lua.get_i(row, "id", id);
ctx.lua.get_s(row, "name", name);
```

## Iterating Lua Tables

Some Stormworks functions return Lua tables, for example `server.getPlayers`.

```cpp
luares out{};

if (ctx.lua.server("getPlayers", {}, out, 1)) {
    ctx.lua.each(ctx, &out.v[0], [](cbctx& ctx, int64_t index, sw_tvalue* row) {
        int64_t id{};
        const char* name{};

        ctx.lua.get_i(row, "id", id);
        ctx.lua.get_s(row, "name", name);
    });
}
```

## Build

Requirements:

- Windows
- CMake
- MSVC-compatible compiler
- C++23

Build:

```powershell
cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release
```

The project builds a DLL:

```text
sw_sdk.dll
```

Loading or injecting the DLL into Stormworks is not handled by this repository.

## Limitations

This SDK relies on Stormworks internals. It can break after game updates because function signatures, Lua structures, or offsets may change.

The project is unofficial and not affiliated with Stormworks: Build and Rescue or its developers.
