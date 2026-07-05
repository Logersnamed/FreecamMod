#pragma once
#include <vector>
#include <string>
#include <map>

#include "core/mod_context.h"
#include "core/config/con_var.h"
#include "core/timeline/timeline.h"
#include "gui/timeline/timeline_window.h"
#include "gui/menu_window.h"

class FreeCamera;
class Speedhack;
class HookManager;
class Config;
class Input;
class ActionManager;
class FrameStepper;
class CameraStateManager;
class PathRecorder;

class MenuWindow {
public:
    explicit MenuWindow(ModContext& ctx, Timeline& timeline, TimelineWindow& timeline_window)
        : infoTab(ctx.freeCamera)
        , featuresTab(ctx.hookManager, ctx.freeCamera, ctx.speedhack)
        , sequencerTab(timeline, timeline_window)
        , configTab(ctx.config)
        , keyBindsTab(ctx.config) {
    }

    void Render();

    bool IsVisible() const { return is_visible; }
    void SetVisibility(bool show) { is_visible = show; }

private:
    bool is_visible = true;

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
};