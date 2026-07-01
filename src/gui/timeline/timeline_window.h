#pragma once
#include "gui/timeline/track_widget.h"
#include "gui/timeline/timeline_config.h"
#include "core/timeline/timeline.h"
#include "utils/time.h"

#include "imgui.h"

class TimelineWindow {
    Timeline& timeline;

    TimelineConfig config{};

    TrackWidget<float>      fovWidget{ "Fov",      timeline.GetFovTrack(), config };
    TrackWidget<float3>     posWidget{ "Position", timeline.GetPosTrack(), config };
    TrackWidget<Quaternion> rotWidget{ "Rotation", timeline.GetRotTrack(), config };

    bool was_clicked_in_timestamps_zone = false;
    bool is_visible = true;

public:
    explicit TimelineWindow(Timeline& timeline) : timeline(timeline) {}

    void SetVisibility(bool show) { is_visible = show; }
    bool IsVisible() const { return is_visible; }

    TimelineConfig& GetConfig() { return config; }

    void Render() {
        if (!is_visible) return;

        float time = timeline.GetTime();
        int time_pixels = config.TimeToPixels(time);
        float max_time = timeline.GetMaxTime();
        bool is_playing = timeline.IsPlaying();

        ImGui::SetNextWindowSizeConstraints(ImVec2(600, 160), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Timeline", &is_visible);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));

        ImGui::BeginChild("##sidebar", ImVec2(config.sidebar_width, config.track_height * 4), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();

            if (ImGui::Button(is_playing ? "Stop" : "Start", ImVec2(config.sidebar_width, config.track_height))) {
                is_playing ? timeline.StopPlay() : timeline.Play();
            }

            fovWidget.DrawSidebar(time, is_playing);
            posWidget.DrawSidebar(time, is_playing);
            rotWidget.DrawSidebar(time, is_playing);
        }
        ImGui::EndChild();

        ImGui::SameLine(0, 0);

        const int scroll_height = 14;
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));
        ImGui::BeginChild("##lanes", ImVec2(ImGui::GetContentRegionAvail().x, config.track_height * 4 + scroll_height), false, ImGuiWindowFlags_HorizontalScrollbar);
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float scroll_x = ImGui::GetScrollX();
            int track_width = config.TrackWidth(max_time);

            for (int i = 0; i < 4; ++i) {
                draw_list->AddRectFilled(
                    ImVec2(pos.x, pos.y + i * config.track_height),
                    ImVec2(ImGui::GetContentRegionAvail().x + track_width, pos.y + (i + 1) * config.track_height),
                    IM_COL32(255, 255, 255, (i % 2 == 1) * 10)
                );
            }

            // Drawing timestamps
            ImGui::BeginChild("##timestamps", ImVec2(track_width, config.track_height));
            const int seconds = ceil(config.PixelsToTime(track_width));
            const float minor_tick = config.pixels_per_second / 10.0f;
            for (float x = 0.0f; x < track_width; x += minor_tick) {
                draw_list->AddLine(
                    ImVec2(pos.x + x, pos.y),
                    ImVec2(pos.x + x, pos.y + config.track_height * 0.3f),
                    IM_COL32(155, 155, 155, 255));
            }

            for (int sec = 0; sec <= seconds; ++sec) {
                float x = sec * config.pixels_per_second;

                draw_list->AddLine(
                    ImVec2(pos.x + x, pos.y),
                    ImVec2(pos.x + x, pos.y + config.track_height * 0.5f),
                    IM_COL32(255, 255, 255, 255));

                draw_list->AddText(
                    ImVec2(pos.x + x + 10, pos.y + config.track_height * 0.3f + 2),
                    IM_COL32(255, 255, 255, 255),
                    TimeToString((float)sec).c_str());
            }
            ImGui::EndChild();

            fovWidget.DrawLane(max_time);
            posWidget.DrawLane(max_time);
            rotWidget.DrawLane(max_time);

            // Drawing playhead
            const int tringle_radius = 8;
            draw_list->AddLine(ImVec2(pos.x + time_pixels, pos.y), ImVec2(pos.x + time_pixels, pos.y + 1000), IM_COL32(255, 255, 255, 255), 1);
            draw_list->AddTriangleFilled(
                ImVec2(pos.x + time_pixels - tringle_radius, pos.y),
                ImVec2(pos.x + time_pixels + tringle_radius, pos.y),
                ImVec2(pos.x + time_pixels, pos.y + tringle_radius),
                IM_COL32(255, 255, 255, 255)
            );
            draw_list->AddText(ImVec2(pos.x + time_pixels + 12, pos.y + 20), IM_COL32(255, 255, 255, 255), TimeToString(time, TimeFormat::MINUTES_SECONDS_MILLISECONDS).c_str());

            bool is_dragging = false;
            bool s = was_clicked_in_timestamps_zone && ImGui::IsMouseDown(ImGuiMouseButton_Left);
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || s) {
                ImVec2 click_pos = ImGui::GetMousePos();

                int scroll_x = ImGui::GetScrollX();
                bool is_in_timestamps_zone = click_pos.x > pos.x + scroll_x && click_pos.y > pos.y && click_pos.y < pos.y + config.track_height;
                if (is_in_timestamps_zone || s) {
                    is_dragging = true;
                    int time_in_pixels = click_pos.x - pos.x;
                    timeline.SetTime(config.PixelsToTime(config.SnapPixelsToGrid(time_in_pixels)));
                    was_clicked_in_timestamps_zone = true;
                    timeline.StopPlay();
                }
            }
            else {
                was_clicked_in_timestamps_zone = false;
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::PopStyleVar();
        ImGui::End();
    }
};