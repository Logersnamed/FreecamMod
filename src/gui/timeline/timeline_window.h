#pragma once
#include "gui/timeline/track_widget.h"
#include "gui/timeline/timeline_config.h"
#include "core/timeline/timeline.h"

#include "ModUtils.h"

class Input;

class TimelineWindow {
    Timeline& timeline;
    Input& input;

    TimelineConfig config{};

    TrackWidget<float>      fovWidget{ "Fov",      timeline.GetFovTrack(), config };
    TrackWidget<float3>     posWidget{ "Position", timeline.GetPosTrack(), config };
    TrackWidget<Quaternion> rotWidget{ "Rotation", timeline.GetRotTrack(), config };

    bool was_clicked_in_timestamps_zone = false;
    bool is_visible = false;
	bool is_hovered = false;

	bool is_mouse_under_titlebar = false;

public:
    explicit TimelineWindow(Timeline& timeline, Input& input) : timeline(timeline), input(input) {
        if (!ModUtils::muWindow) {
            ModUtils::AttemptToGetWindowHandle();
        }
        RECT rect;
        GetWindowRect(ModUtils::muWindow, &rect);

        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

		config.sidebar_width *= width / 1920.0f;
        config.pixels_per_second *= width / 1920.0f;
        config.track_height *= height / 1080.0f;
    }

    void SetVisibility(bool show) { is_visible = show; }
    bool IsVisible() const { return is_visible; }
	bool IsHovered() const { return is_hovered; }

    TimelineConfig& GetConfig() { return config; }

    void Render();
};