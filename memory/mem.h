//
// Created by abcdefghkj on 15.05.2026.
//

#pragma once
#include <cstddef>
#include <optional>
#include <Windows.h>
#include <Psapi.h>
#include <vector>
#include <string>
#include <cstdint>


namespace memory {
    template <class T>
    [[nodiscard]] T at(uintptr_t addr, std::ptrdiff_t off) {
        return *reinterpret_cast<const T*>(
            addr + off
        );
    }

    inline std::optional<uintptr_t> FindPattern(const char* pattern)
    {
        HMODULE hModule = GetModuleHandleA(NULL);
        if (!hModule) return std::nullopt;
        MODULEINFO mI{};
        GetModuleInformation(GetCurrentProcess(), hModule, &mI, sizeof(mI));
        auto* base = reinterpret_cast<std::uint8_t*>(mI.lpBaseOfDll);
        size_t size = mI.SizeOfImage;
        std::vector<std::pair<uint8_t, bool>> patternBytes;
        const char* current = pattern;
        while (*current)
        {
            while (*current == ' ') current++;
            if (!*current) break;
            if (*current == '?') { patternBytes.emplace_back(0x00, true); current++; if (*current == '?') current++; }
            else { patternBytes.emplace_back(static_cast<uint8_t>(strtoul(current, nullptr, 16)), false); }
            while (*current && *current != ' ') current++;
        }
        if (patternBytes.empty()) return std::nullopt;
        for (size_t i = 0; i < size - patternBytes.size(); i++)
        {
            bool found = true;
            for (size_t j = 0; j < patternBytes.size(); j++)
                if (!patternBytes[j].second && base[i + j] != patternBytes[j].first) { found = false; break; }
            if (found) return reinterpret_cast<uintptr_t>(&base[i]);
        }
        return std::nullopt;
    }
}