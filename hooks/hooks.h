//
// Created by abcdefghkj on 15.05.2026.
//

#pragma once
#include "dep/safetyhook.hpp"


class fname {
public:
    std::string name;
    size_t id;
    fname(std::string name, size_t id) : name(name), id(id) {};

};


class hook {
public:
    SafetyHookInline inl;
    SafetyHookVmt vmt;
    SafetyHookVm vm;

    size_t id;

};


class hooks {
private:
    std::vector<fname> name_pool;
    std::vector<hook*> hook_pool;

public:

    void add(hook *obj, const std::string &name);
    hook* get(const std::string &name);

    void init();

};


inline hooks g_hooks;

