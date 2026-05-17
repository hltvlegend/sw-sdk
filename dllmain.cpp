#include <Windows.h>
#include "hooks/hooks.h"

DWORD WINAPI Main(LPVOID lpParam) {


    g_hooks.init();

    for (;;)
        Sleep(1000);

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);

        CreateThread(
            nullptr,
            0,
            Main,
            hinstDLL,
            0,
            nullptr
        );
    }

    return TRUE;
}