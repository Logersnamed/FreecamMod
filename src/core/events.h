#pragma once
#include <vector>
#include <functional>
#include "utils/types.h"

#include "core/input/action_type.h"

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

    struct DPIChanged {};

    struct BlockCameraMouseMoveInput { bool isEnabled = false; };

    struct BlockActions {
        std::array<bool, static_cast<size_t>(ActionType::Count)> blocked{};

        static BlockActions All() {
            BlockActions b;
            b.blocked.fill(true);
            return b;
        }

        static BlockActions None() {
            return BlockActions{};
        }

        BlockActions& With(ActionType type) {
            blocked[static_cast<size_t>(type)] = true;
            return *this;
        }

        BlockActions& Except(ActionType type) {
            blocked[static_cast<size_t>(type)] = false;
            return *this;
        }
    };
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