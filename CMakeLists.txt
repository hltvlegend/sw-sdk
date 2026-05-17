cmake_minimum_required(VERSION 4.2)
project(sw_sdk)

set(CMAKE_CXX_STANDARD 23)

add_library(sw_sdk SHARED dllmain.cpp
        memory/mem.h
        hooks/hooks.cpp
        hooks/hooks.h
        hooks/dep/safetyhook.cpp
        hooks/dep/safetyhook.hpp
        hooks/dep/Zydis.c
        hooks/dep/Zydis.h
        hooks/lua_load.cpp
        hooks/lua_load.h
        hooks/lua_callback.cpp
        hooks/lua_callback.h
        lua/swlua.hpp
        lua/lua_main.cpp
        lua/lua_main.h
        print.h)
