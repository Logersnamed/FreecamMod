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
        : infoTab(ctx.freeCamera, ctx.cfg)
        , featuresTab(ctx.hookManager, ctx.freeCamera, ctx.speedhack, ctx.cfg)
        , sequencerTab(timeline, timeline_window, ctx.cfg)
        , configTab(ctx.cfg)
        , keyBindsTab(ctx.cfg) {
    }

    void Render();

    bool IsVisible() const { return is_visible; }
    void SetVisibility(bool show) { is_visible = show; }

private:
    bool is_visible = true;

    class InfoTab {
        FreeCamera& freeCamera;
		Config& cfg;

    public:
        InfoTab(FreeCamera& freeCamera, Config& cfg) : freeCamera(freeCamera), cfg(cfg) {}

        void Render();
    };

    class FeaturesTab {
        HookManager& hookManager;
        FreeCamera& freeCamera;
        Speedhack& speedhack;
        FrameStepper& frameStepper;
        CameraStateManager& cameraStateMgr;
        PathRecorder& pathRecorder;
		Config& cfg;

    public:
        FeaturesTab(HookManager& hookManager, FreeCamera& freeCamera, Speedhack& speedhack, Config& cfg);

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
		Config& cfg;

    public:
        SequencerTab(Timeline& timeline, TimelineWindow& timelineWindow, Config& cfg)
            : timeline(timeline), timelineWindow(timelineWindow), cfg(cfg) {}

        template<typename T>
        void DrawCombo(const char* label, Track<T>& type);
        void Render();
    };

    class ConfigTab {
        Config& cfg;

        std::map<std::string, std::vector<IConVar*>> sortedConVars;
        bool areSorted = false;

    public:
        ConfigTab(Config& cfg) : cfg(cfg) {}

        void SortConVars();
        void Render();
    };

    class KeyBindsTab {
        Config& cfg;
    public:
        KeyBindsTab(Config& cfg) : cfg(cfg) {}

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