#pragma once
#include <chrono>

class Time {
    using clock = std::chrono::steady_clock;

public:
    static float DeltaTime() {
        static auto last = clock::now();

        auto now = clock::now();
        std::chrono::duration<float> delta = now - last;
        last = now;

        return delta.count();
    }
};