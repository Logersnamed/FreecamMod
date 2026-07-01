#pragma once
#include <string>

inline bool HRCheck(HRESULT hr, const char* expr, const char* file, int line)
{
    if (FAILED(hr))
    {
        char buffer[512];
        sprintf_s(
            buffer,
            "HRESULT FAILED\nExpr: %s\nFile: %s\nLine: %d\nHR: 0x%08X\n",
            expr,
            file,
            line,
            static_cast<unsigned>(hr)
        );

        OutputDebugStringA(buffer);
        return false;
    }

    return true;
}

#define HR_CHECK(x) \
    if (!HRCheck((x), #x, __FILE__, __LINE__)) return false;