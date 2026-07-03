#pragma once
#include <vector>
#include <string>
#include <map>

#include "core/mod_context.h"
#include "core/config/con_var.h"
#include "core/timeline/timeline.h"
#include "gui/timeline/timeline_window.h"

class FreeCamera;
class Speedhack;
class HookManager;
class Config;
class Input;
class ActionManager;
class FrameStepper;
class CameraStateManager;
class PathRecorder;

class GUI {
    class InfoTab {
        FreeCamera& freeCamera;

    public:
        InfoTab(FreeCamera& freeCamera) : freeCamera(freeCamera) {}

        void Render();
    };

    class FeaturesTab {
        HookManager& hookManager;
        FreeCamera& freeCamera;
        Speedhack& speedhack;
        FrameStepper& frameStepper;
        CameraStateManager& cameraStateMgr;
        PathRecorder& pathRecorder;

    public:
        FeaturesTab(HookManager& hookManager, FreeCamera& freeCamera, Speedhack& speedhack);

        void RenderSpeedhack();
        void RenderFrameStepper();
        void RenderCycleWeatherTime();
        void RenderCameraStateManager();
        void RenderPathRecorder();
        void Render();
    };

    class SequencerTab {
        Timeline& timeline;
        TimelineWindow& timelineWindow;
        
    public:
        SequencerTab(Timeline& timeline, TimelineWindow& timelineWindow) 
            : timeline(timeline), timelineWindow(timelineWindow) {}

        template<typename T>
        void DrawCombo(const char* label, Track<T>& type);
        void Render();
    };
    
    class ConfigTab {
        Config& config;

        std::map<std::string, std::vector<IConVar*>> sortedConVars;
        bool areSorted = false;

    public:
        ConfigTab(Config& config) : config(config) {}

        void SortConVars();
        void Render();
    };

    class KeyBindsTab {
        Config& config;
    public:
        KeyBindsTab(Config& config) : config(config) {}

        void Render();
    };

    class LogTab {
    public:
        void Render();
    };

    InfoTab infoTab;
    FeaturesTab featuresTab;
    SequencerTab sequencerTab;
    ConfigTab configTab;
    KeyBindsTab keyBindsTab;
    LogTab logTab{};

    Config& config;
    Input& input;
    ActionManager& actionMgr;

    Timeline timeline;
    TimelineWindow timeline_window;

    bool is_visible = true;

    static inline GUI* instance = nullptr;

    using SetCursorPos_t = BOOL(WINAPI*)(int, int);
    static inline SetCursorPos_t origSetCursorPos = nullptr;

    static BOOL WINAPI hkSetCursorPos(int X, int Y);
    void HandleCursorVisibility();
    void OnDpiChange(); // todo
    void InitializeStyle();
    void SubscribeEvents();

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

public:
    explicit GUI(ModContext& ctx)
        : config(ctx.config), input(ctx.input), actionMgr(ctx.actionMgr),
        timeline(ctx.freeCamera), timeline_window(timeline),
        infoTab(ctx.freeCamera),
        featuresTab(ctx.hookManager, ctx.freeCamera, ctx.speedhack),
        sequencerTab(timeline, timeline_window),
        configTab(ctx.config),
        keyBindsTab(ctx.config) { instance = this; }

    ImGuiStyle baseStyle{};

    bool RegisterHooks(HookManager& hookManager);
    void Initialize();
    void Render();
};