#pragma once
#include "core/mod_context.h"
#include "core/config/con_var.h"
#include "core/timeline/timeline.h"
#include "gui/timeline/timeline_window.h"
#include "gui/menu_window.h"

class HookManager;
class Config;
class Input;
class ActionManager;

class GUI {
public:
    explicit GUI(ModContext& ctx)
        : config(ctx.config), input(ctx.input), actionMgr(ctx.actionMgr),
        timeline(ctx.freeCamera), timeline_window(timeline),
        menu_window(ctx, timeline, timeline_window) {
        instance = this;
    }

    void Initialize();
    bool RegisterHooks(HookManager& hookManager);
    void Render();

private:
    Timeline timeline;

    MenuWindow menu_window;
    TimelineWindow timeline_window;

    Config& config;
    Input& input;
    ActionManager& actionMgr;

    bool is_visible = true;
    bool is_cursor_visible = true;
    ImGuiStyle baseStyle{};

    static inline GUI* instance = nullptr;

    using SetCursorPos_t = BOOL(WINAPI*)(int, int);
    static inline SetCursorPos_t origSetCursorPos = nullptr;

    static BOOL WINAPI hkSetCursorPos(int X, int Y);
    void HandleCursorVisibility(bool draw_cursor_if_is_visible = true);
    void OnDpiChange();
    void InitializeStyle();
    void SubscribeEvents();
    void SetupDockspace();

    ConVar<bool> showMenuOnStartup{ "gui", "show_menu_on_startup", true };
    ConVar<bool> enableNotifications{ "gui", "enable_notifications", true };
    ConVar<bool> notifyFreecam{ "gui", "notify_freecam", false };
    ConVar<bool> notifySpeedhack{ "gui", "notify_speedhack", true };
    ConVar<bool> notifyFrameStepped{ "gui", "notify_frame_stepped", true };
    ConVar<bool> notifyCycleWeatherTime{ "gui", "notify_cycle_weather_time", true };
    ConVar<bool> notifyRecord{ "gui", "notify_record", true };
    ConVar<bool> notifyPlayRecord{ "gui", "notify_play_record", true };
    ConVar<bool> notifySaveState{ "gui", "notify_save_state", true };
    ConVar<bool> notifyInterpolate{ "gui", "notify_interpolate", true };
    ConVar<bool> notifyStateQueued{ "gui", "notify_state_queued", false };
};