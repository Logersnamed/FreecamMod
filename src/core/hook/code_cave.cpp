#include "core/hook/code_cave.h"
#include "utils/memory.h"

namespace CodeCaveAsm {
    namespace DaytimeUpdateAsm {
        extern "C" {
            void DaytimeUpdateFunc();
            uintptr_t returnAddress = 0;
            uintptr_t funcAddress = 0;
            bool freeze_time_day = 0;
            bool set_morning = 0;
            int cycle_speed = 1000000;
        }
    }
}

bool CodeCave::Hook(uintptr_t hkAddress) {
    if (!hkAddress) return false;
    hookAddress = hkAddress;

    SaveOriginalBytes(hookAddress);
    ModUtils::Hook(hookAddress, GetDestinationAddress(), caveSize - 14);
    return true;
}

void CodeCave::Unhook() {
    if (hookAddress) {
        ModUtils::ToggleMemoryProtection(false, hookAddress, caveSize);
        memcpy((void*)hookAddress, originalBytes, caveSize);
        ModUtils::ToggleMemoryProtection(true, hookAddress, caveSize);

        FlushInstructionCache(GetCurrentProcess(), (void*)hookAddress, caveSize);

        if (originalBytes) {
            delete[] originalBytes;
            originalBytes = nullptr;
        }

        hookAddress = 0;
    }
}

void DaytimeUpdateCave::SaveOriginalBytes(uintptr_t hkAddress) {
    using namespace CodeCaveAsm;

    caveSize = 16;

    originalBytes = new uint8_t[caveSize];
    ModUtils::ToggleMemoryProtection(false, hkAddress, caveSize);
    memcpy(originalBytes, (void*)hkAddress, caveSize);
    ModUtils::ToggleMemoryProtection(true, hkAddress, caveSize);

    uintptr_t callInstruction = hkAddress + 11;
    DaytimeUpdateAsm::funcAddress = Memory::GetCallTargetAddress(callInstruction);
    DaytimeUpdateAsm::returnAddress = hkAddress + caveSize;

    isDayTimeFrozen = &DaytimeUpdateAsm::freeze_time_day;
    isFastDayCycle = &DaytimeUpdateAsm::set_morning;
    cycleSpeed = &DaytimeUpdateAsm::cycle_speed;
}

uintptr_t DaytimeUpdateCave::GetDestinationAddress() {
    return (uintptr_t)&CodeCaveAsm::DaytimeUpdateAsm::DaytimeUpdateFunc;
}