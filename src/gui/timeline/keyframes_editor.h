#pragma once
#include "gui/timeline/timeline_config.h"
#include "core/timeline/track.h"

struct TrackInputEvent {
    float drag_delta_x = 0;
    bool drag_released = false;
    bool mouse_clicked = false;
    bool shift_held = false;
    float2 click_pos{};
    float2 origin_pos{};
};

template<typename T>
struct KeyframesEditor {
    static void Update(Track<T>& track, const TrackInputEvent& events, const TimelineConfig& cfg) {
        ProcessDrag(track, events, cfg);
        ProcessSelect(track, events, cfg);
    }

    static void ProcessDrag(Track<T>& track, const TrackInputEvent& events, const TimelineConfig& cfg) {
        if (events.drag_released && events.drag_delta_x) {
            for (auto& k : track.GetKeyframes()) {
                if (!k.is_selected)  continue;

                k.time += cfg.PixelsToTime(events.drag_delta_x);
                k.time = std::max<float>(k.time, 0.0f);
                if (events.drag_delta_x) k.time = cfg.SnapTimeToGrid(k.time);   // snap only after drag
            }

            track.SortKeyframes();
        }
    }

    static void ProcessSelect(Track<T>& track, const TrackInputEvent& events, const TimelineConfig& cfg) {
        if (!events.mouse_clicked) return;

        const int radius = 5;
        auto& keyframes = track.GetKeyframes();
        for (auto& k : keyframes) {
            float2 keyframe_center_pos = float2(
                events.origin_pos.x + cfg.TimeToPixels(k.time),
                events.origin_pos.y + (cfg.track_height - radius) * 0.5f
            );

            bool is_keyframe_clicked = abs(keyframe_center_pos.x - events.click_pos.x) <= radius
                && abs(keyframe_center_pos.y - events.click_pos.y) <= radius;

            if (is_keyframe_clicked) k.is_selected = true;
            else {
                if (k.is_selected && !events.shift_held) {
                    k.is_selected = false;
                }
            }
        }
    }
};