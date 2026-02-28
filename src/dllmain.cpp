#include <windows.h>
#include "freecam.h"

DWORD WINAPI MainThread(LPVOID lpParam) {
    Freecam freecam((HMODULE)lpParam, FindWindow(NULL, "ELDEN RING™"));
    freecam.Run();
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        CreateThread(0, 0, MainThread, module, 0, NULL);
    }
    return TRUE;
}