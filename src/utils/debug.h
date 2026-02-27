#pragma once
#include <windows.h>
#include <iostream>

namespace Debug {
    void CreateConsole() {
        AllocConsole();
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONIN$", "r", stdin);
        SetConsoleTitleA("Debug console");
    }
}