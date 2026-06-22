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

enum class TimeFormat {
    SECONDS,
    SECONDS_MILLISECONDS,
    MINUTES_SECONDS,
    MINUTES_SECONDS_MILLISECONDS,
};


static inline std::string TimeToString(float seconds, TimeFormat format = TimeFormat::MINUTES_SECONDS) {
    seconds = std::max<float>(0.0f, seconds);

    const int totalSeconds = static_cast<int>(seconds);
    const int minutes = totalSeconds / 60;
    const int secs = totalSeconds % 60;
    const int milliseconds = static_cast<int>((seconds - totalSeconds) * 1000.0f);

    switch (format) {
    case TimeFormat::SECONDS:
        return std::format("{}", totalSeconds);

    case TimeFormat::SECONDS_MILLISECONDS:
        return std::format("{}.{:03}", totalSeconds, milliseconds);

    case TimeFormat::MINUTES_SECONDS:
        return std::format("{:02}:{:02}", minutes, secs);

    case TimeFormat::MINUTES_SECONDS_MILLISECONDS:
        return std::format("{:02}:{:02}.{:03}", minutes, secs, milliseconds);
    }
    return {};
}
