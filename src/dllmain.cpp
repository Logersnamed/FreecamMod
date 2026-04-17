#include <windows.h>

#include "freecam.h"

DWORD WINAPI MainThread(LPVOID lpParam) {
    Sleep(500);
    
    {
        Freecam freecam((HMODULE)lpParam);
        freecam.Run();
        freecam.Dispose();
    }

    Sleep(500);
    FreeLibraryAndExitThread((HMODULE)lpParam, 0);
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        CreateThread(0, 0, MainThread, module, 0, NULL);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        if (Freecam::instance) Freecam::instance->Dispose();
    }
    return TRUE;
}