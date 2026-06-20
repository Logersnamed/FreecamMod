#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "core/config/con_var.h"
#include "core/config/config.h"
#include "core/free_camera.h"
#include "core/features/speedhack.h"

#include <unordered_map>

#include "core/game_data_manager.h"
#include "hook/hook_manager.h"

#include <MinHook.h>

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
        FeaturesTab(HookManager& hookManager, FreeCamera& freeCamera, Speedhack& speedhack)
            : hookManager(hookManager), freeCamera(freeCamera), speedhack(speedhack), 
            frameStepper(freeCamera.GetFrameStepper()), cameraStateMgr(freeCamera.GetCameraStateManager()),
            pathRecorder(freeCamera.GetPathRecorder()) {}

        void RenderSpeedhack();
        void RenderFrameStepper();
        void RenderCycleWeatherTime();
        void RenderCameraStateManager();
        void RenderPathRecorder();
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
    ConfigTab configTab;
    KeyBindsTab keyBindsTab;
    LogTab logTab{};

    Config& config;
    Input& input;
    ActionManager& actionMgr;

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
    GUI(FreeCamera& freeCamera, Speedhack& speedhack, HookManager& hookManager, Config& config, Input& input, ActionManager& actionMgr) :
        config(config), input(input), actionMgr(actionMgr),
        infoTab(freeCamera),
        featuresTab(hookManager, freeCamera, speedhack),
        configTab(config),
        keyBindsTab(config) { instance = this; }

    ImGuiStyle baseStyle{};

    bool RegisterHooks(HookManager& hookManager);
    void Initialize();
    void Render();
};