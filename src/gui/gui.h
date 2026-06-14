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
    public:
        void Render();
    };

    class LogTab {
    public:
        void Render();
    };

    InfoTab infoTab;
    FeaturesTab featuresTab;
    ConfigTab configTab;
    KeyBindsTab keyBindsTab{};
    LogTab logTab{};

    Config& config;

    bool is_visible = true;

    static inline GUI* instance = nullptr;

    using SetCursorPos_t = BOOL(WINAPI*)(int, int);
    static inline SetCursorPos_t origSetCursorPos = nullptr;

    static BOOL WINAPI hkSetCursorPos(int X, int Y);
    void HandleCursorVisibility();
    void OnDpiUpdate(); // todo

public:
    GUI(FreeCamera& freeCamera, Speedhack& speedhack, HookManager& hookManager, Config& config) :
        config(config),
        infoTab(freeCamera),
        featuresTab(hookManager, freeCamera, speedhack),
        configTab(config) { instance = this; }

    ImGuiStyle baseStyle{};

    bool RegisterHooks(HookManager& hookManager);
    void Initialize();
    void Render();
};