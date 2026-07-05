#pragma once
#include "gui/timeline/track_editor.h"
#include "gui/timeline/keyframes_editor.h"
#include "gui/timeline/timeline_config.h"
#include "gui/helpers.h"
#include "core/timeline/track.h"

template<typename T>
class TrackWidget {
    Track<T>& track;
    const TimelineConfig& cfg;

    std::string name{};

    float delta_x = 0;

public:
    TrackWidget(std::string name, Track<T>& track, const TimelineConfig& cfg)
        : name(std::move(name)), track(track), cfg(cfg) {}

    void DrawKeyframes(std::vector<Keyframe<T>>& keyframes, const TrackInputEvent& events) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        const int radius = 5;
        for (auto& k : keyframes) {
            float drag_delta = (k.is_selected && !events.drag_released) ? events.drag_delta_x : 0;

            float delta = std::max<float>(0.0f, cfg.TimeToPixels(k.time) + drag_delta);
            if (drag_delta) delta = cfg.SnapPixelsToGrid(delta);    // snap only after drag
            draw_list->AddCircle(
                ImVec2(pos.x + delta, pos.y + cfg.track_height * 0.5f),
                radius,
                k.is_selected ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 255, 255, 255)
                , 4, 2
            );
        }
    }

    TrackInputEvent GetEvents(ImVec2 origin) {
        bool released = false;
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            delta_x = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
        }
        else if (delta_x) {
            released = true;
        }
        ImVec2 mp = ImGui::GetMousePos();

        TrackInputEvent events{
            delta_x,
            released,
            ImGui::IsMouseClicked(ImGuiMouseButton_Left),
            ImGui::IsKeyDown(ImGuiKey_LeftShift),
            {mp.x, mp.y},
            {origin.x, origin.y}
        };

        return events;
    }

    void DrawSidebar(float time, bool is_playing) {
        ImGui::PushID(this);
        ImGui::BeginChild("##sidebar_header", ImVec2(cfg.sidebar_width, cfg.track_height));
        if (ImGui::Button("+")) {
            track.AddKeyframe(time);
        }
		ImHelpers::Tooltip("Add keyframe");

        ImGui::SameLine();

        T data = track.GetData();
        if (TrackEditor<T>::DrawValue(name, data)) {
            track.SetData(data);
            track.AddKeyframe(time);
            track.WriteCameraValue(data);
        }
		ImGui::EndChild();
        ImGui::PopID();
    }

    void DrawLane(float max_time, ImVec4 bg_color) {
        ImGui::PushID(this);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg_color);
        ImGui::BeginChild("##lane", ImVec2(cfg.TrackWidth(max_time), cfg.track_height));
        ImVec2 origin = ImGui::GetCursorScreenPos();
        TrackInputEvent events = GetEvents(origin);

        KeyframesEditor<T>::Update(track, events, cfg);
        DrawKeyframes(track.GetKeyframes(), events);

        if (events.drag_released)
            delta_x = 0;

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopID();
    }
};