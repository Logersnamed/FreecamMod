#pragma once
#include "imgui.h"

struct TimelineConfig {
    int pixels_per_second = 100;
    int sidebar_width = 300;
    int track_height = 25;
    int snap_pixels = 5;
	int track_visible_width = 1000;
    bool snap_enabled = true;

    int TimeToPixels(float time)  const { return (int)(time * (float)pixels_per_second); }
    float PixelsToTime(int pixels)  const { return pixels / (float)pixels_per_second; }

    int SnapPixelsToGrid(int pixels) const {
        if (!snap_enabled || ImGui::IsKeyDown(ImGuiKey_LeftAlt)) return pixels;
        return ((pixels + snap_pixels / 2) / snap_pixels) * snap_pixels;
    }

    float SnapTimeToGrid(float time) const {
        if (!snap_enabled || ImGui::IsKeyDown(ImGuiKey_LeftAlt)) return time;
        return PixelsToTime(SnapPixelsToGrid(TimeToPixels(time)));
    }

    int TrackWidth(float max_time) const {
        return (int)(max_time * (float)pixels_per_second);
    }

	void SetPixelsPerSecond(int pps, float max_time) {
		pixels_per_second = std::max(pps, GetMinPixelsPerSecond(max_time));
	}

    int GetMinPixelsPerSecond(float max_time) const {
        if (max_time <= 0.0f) return pixels_per_second;
        return (int)((float)track_visible_width / max_time);
    }
};