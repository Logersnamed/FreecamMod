#pragma once
#include <vector>
#include <functional>
#include "utils/types.h"

namespace Event {
    struct ToggleFreecam { bool isEnabled = false; };
    struct ToggleSpeedhack { bool isEnabled = false; };
    struct FrameStepped { int framesStepped{}; };
    struct ToggleCycleWeatherTime { bool isEnabled = false; };
    struct Record { bool isEnabled = false; };
    struct PlayRecord { bool isEnabled = false; };
    struct SaveState { int slot{}; float3 pos{}; };
    struct Interpolate { bool isEnabled = false; FixedVec<uint8_t, 10> slots{}; };
    struct StateQueued { int slot; };
}

class EventBus {
public:
    template<typename Event>
    using callback_t = std::function<void(const Event&)>;

    template<typename Event>
    static void Subscribe(callback_t<Event> callback) {
        GetCallbackHandlers<Event>().push_back(std::move(callback));
    }

    template<typename Event>
    static void Emit(const Event& event) {
        auto& handlers = GetCallbackHandlers<Event>();

        for (auto& handler : handlers) {
            handler(event);
        }
    }

private:
    template<typename Event>
    static std::vector<callback_t<Event>>& GetCallbackHandlers() {
        static std::vector<callback_t<Event>> callbackHandlers;
        return callbackHandlers;
    }
};