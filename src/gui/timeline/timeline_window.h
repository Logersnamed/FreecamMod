#pragma once
#include "gui/timeline/track_widget.h"
#include "gui/timeline/timeline_config.h"
#include "core/timeline/timeline.h"

class TimelineWindow {
    Timeline& timeline;

    TimelineConfig config{};

    TrackWidget<float>      fovWidget{ "Fov",      timeline.GetFovTrack(), config };
    TrackWidget<float3>     posWidget{ "Position", timeline.GetPosTrack(), config };
    TrackWidget<Quaternion> rotWidget{ "Rotation", timeline.GetRotTrack(), config };

    bool was_clicked_in_timestamps_zone = false;
    bool is_visible = false;
	bool is_hovered = false;

public:
    explicit TimelineWindow(Timeline& timeline) : timeline(timeline) {}

    void SetVisibility(bool show) { is_visible = show; }
    bool IsVisible() const { return is_visible; }
	bool IsHovered() const { return is_hovered; }

    TimelineConfig& GetConfig() { return config; }

    void Render();
};