#include "gui/timeline/timeline_window.h"
#include "gui/helpers.h"
#include "core/input/input.h"

#include "utils/time.h"

#include "imgui.h"

void TimelineWindow::Render() {
    if (!is_visible) return;

    float time = timeline.GetTime();
    int time_pixels = config.TimeToPixels(time);
    float max_time = timeline.GetMaxTime();
    bool is_playing = timeline.IsPlaying();

	// Handle input
	bool isTimelineInput = input.GetInputSource() == InputSource::Timeline;
    if (isTimelineInput) {
        if (input.IsHotkeyPressed({ VK_CONTROL, 'A' })) {
			timeline.SelectAllKeyframes();
		}

        if (input.IsPressed(VK_CONTROL)) {
            float delta = input.GetScrollDelta();
			config.pixels_per_second += (int)(delta * 5.0f);
        }
    }

    std::string title = "Timeline " + TimeToString(time, TimeFormat::MINUTES_SECONDS_MILLISECONDS);
    ImGui::Begin((title + "###timeline").c_str(), &is_visible, is_mouse_under_titlebar ? ImGuiWindowFlags_NoMove : 0);

	is_mouse_under_titlebar = ImGui::GetCursorScreenPos().y < ImGui::GetMousePos().y;
    is_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

    int spacing = 4;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, 0));
    ImGui::BeginChild("##sidebar", ImVec2(config.sidebar_width, config.track_height * 4), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
        ImGui::BeginChild("##sidebar_header", ImVec2(config.sidebar_width, config.track_height));
        if (ImGui::Button("+##add_all") || input.IsJustPressed('O')) {
            timeline.AddAllKeyframes(time);
        }
		ImHelpers::TooltipWithShortcut("Add all keyframes", "O");

        ImGui::SameLine();

        if (ImGui::Button(is_playing ? "Pause" : "Play", ImVec2(ImGui::GetContentRegionAvail().x - spacing, 0)) || (input.IsJustPressed(VK_SPACE) && isTimelineInput)) {
            is_playing ? timeline.StopPlay() : timeline.Play();
        }
        ImHelpers::TooltipWithShortcut("Play/Pause", "Space");
        ImGui::EndChild();

        fovWidget.DrawSidebar(time, is_playing);
        posWidget.DrawSidebar(time, is_playing);
        rotWidget.DrawSidebar(time, is_playing);
    }
    ImGui::EndChild();

    ImGui::SameLine(0, 0);
    ImGui::Dummy(ImVec2(spacing, 0));
    ImGui::SameLine(0, 0);

    const int scroll_height = 14;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));
    ImGui::BeginChild("##lanes", ImVec2(ImGui::GetContentRegionAvail().x, config.track_height * 4 + scroll_height), false, ImGuiWindowFlags_HorizontalScrollbar);
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float scroll_x = ImGui::GetScrollX();
        int track_width = config.TrackWidth(max_time);

        // Drawing timestamps
        ImGui::BeginChild("##timestamps", ImVec2(track_width, config.track_height));
        const int seconds = ceil(config.PixelsToTime(track_width));
        const float minor_tick = config.pixels_per_second / 10.0f;
        for (float x = 0.0f; x < track_width; x += minor_tick) {
            draw_list->AddLine(
                ImVec2(pos.x + x, pos.y),
                ImVec2(pos.x + x, pos.y + config.track_height * 0.3f),
                IM_COL32(155, 155, 155, 255)
            );
        }

        for (int sec = 0; sec <= seconds; ++sec) {
            float x = sec * config.pixels_per_second;

            draw_list->AddLine(
                ImVec2(pos.x + x, pos.y),
                ImVec2(pos.x + x, pos.y + config.track_height * 0.5f),
                IM_COL32(255, 255, 255, 255)
            );

            draw_list->AddText(
                ImVec2(pos.x + x + 10, pos.y + config.track_height * 0.3f + 2),
                IM_COL32(255, 255, 255, 255),
                TimeToString((float)sec).c_str()
            );
        }
        ImGui::EndChild();

        const ImVec4 bg1 = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
        const ImVec4 bg2 = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);

        fovWidget.DrawLane(max_time, bg2);
        posWidget.DrawLane(max_time, bg1);
        rotWidget.DrawLane(max_time, bg2);

        // Drawing playhead
        const int tringle_radius = 8;
        draw_list->AddLine(ImVec2(pos.x + time_pixels, pos.y), ImVec2(pos.x + time_pixels, pos.y + 1000), IM_COL32(255, 255, 255, 255), 1);
        draw_list->AddTriangleFilled(
            ImVec2(pos.x + time_pixels - tringle_radius, pos.y),
            ImVec2(pos.x + time_pixels + tringle_radius, pos.y),
            ImVec2(pos.x + time_pixels, pos.y + tringle_radius),
            IM_COL32(255, 255, 255, 255)
        );

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