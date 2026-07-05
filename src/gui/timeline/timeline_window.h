#pragma once
#include "gui/timeline/track_widget.h"
#include "gui/timeline/timeline_config.h"
#include "core/timeline/timeline.h"

#include "ModUtils.h"

class Input;
class ActionManager;
class Config;

class TimelineWindow {
    Timeline& timeline;
    Input& input;
	ActionManager& actionMgr;

    TimelineConfig cfg{};
    Config& config;

    TrackWidget<float>      fovWidget{ "Fov",      timeline.GetFovTrack(), cfg };
    TrackWidget<float3>     posWidget{ "Position", timeline.GetPosTrack(), cfg };
    TrackWidget<Quaternion> rotWidget{ "Rotation", timeline.GetRotTrack(), cfg };

    bool was_clicked_in_timestamps_zone = false;
    bool is_visible = false;
	bool is_hovered = false;

	bool is_mouse_under_titlebar = false;

public:
    explicit TimelineWindow(Timeline& timeline, Input& input, ActionManager& actionMgr, Config& config) : timeline(timeline), input(input), actionMgr(actionMgr), config(config) {
        if (!ModUtils::muWindow) {
            ModUtils::AttemptToGetWindowHandle();
        }

		if (!ModUtils::muWindow) {
			LOG_ERROR("Failed to get window handle for timeline window");
			return;
		}

        RECT rect;
        GetWindowRect(ModUtils::muWindow, &rect);

        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

		cfg.sidebar_width *= width / 1920.0f;
        cfg.pixels_per_second *= width / 1920.0f;
        cfg.track_height *= height / 1080.0f;
    }

    void SetVisibility(bool show) { is_visible = show; }
    bool IsVisible() const { return is_visible; }
	bool IsHovered() const { return is_hovered; }

    TimelineConfig& GetConfig() { return cfg; }

    void Render();
};