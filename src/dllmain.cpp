#include <windows.h>
#include "freecam.h"
#include "utils/debug.h"

DWORD WINAPI MainThread(LPVOID lpParam) {
	Logger::Init();

    Freecam freecam((HMODULE)lpParam, FindWindow(NULL, "ELDEN RING™"));
    freecam.Run();

    Logger::Info("Shutting down..");
    Logger::Shutdown();

    Sleep(500);
    FreeLibraryAndExitThread(freecam.GetModule(), 0);
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        CreateThread(0, 0, MainThread, module, 0, NULL);
    }
    return TRUE;
}