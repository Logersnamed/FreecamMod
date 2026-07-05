#pragma once
#include "imgui.h"

class NotificationPopUp {
    static inline float timeLeft = 0.0f;
    static inline float duration = 0.0f;

    static inline float fadeTime = 0.0f;

    static inline std::string header{};
    static inline std::string message{};

    static inline const float PADDING = 20.0f;
public:
    static void Notify(std::string head, std::string msg, float time = 1.5f, float fade = 0.3f) {
        header = std::move(head);
        message = std::move(msg);

        duration = time + fade;
        timeLeft = time + fade;
        fadeTime = fade;
    }

    static void Render() {
        if (timeLeft <= 0) return;

        timeLeft -= ImGui::GetIO().DeltaTime;

        float alpha = 1.0f;
        if (timeLeft < fadeTime) {
            alpha = std::clamp(timeLeft / fadeTime, 0.0f, 1.0f);
        }

        ImGuiIO& io = ImGui::GetIO();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos;
        ImGui::SetNextWindowPos(ImVec2(work_pos.x + PADDING, work_pos.y + PADDING), ImGuiCond_Always);
        ImGui::SetNextWindowViewport(viewport->ID);
        window_flags |= ImGuiWindowFlags_NoMove;

        bool p_open = true;
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
        if (ImGui::Begin(header.c_str(), &p_open, window_flags)) {
            ImGui::Text(message.c_str());

            // Notification progress bar
/*            {
                ImDrawList* drawList = ImGui::GetWindowDrawList();

                ImVec2 pos = ImGui::GetWindowPos();
                ImVec2 size = ImGui::GetWindowSize();

                float t = std::clamp((timeLeft - fadeTime) / (duration - fadeTime), 0.0f, 1.0f);

                float barHeight = 3.0f;
                float y = pos.y + size.y - barHeight;

                float rounding = ImGui::GetStyle().WindowRounding;

                drawList->AddRectFilled(
                    ImVec2(pos.x, y),
                    ImVec2(pos.x + size.x * t, y + barHeight),
                    IM_COL32(83, 207, 81, 255),
                    rounding,
                    ImDrawFlags_RoundCornersBottom
                );
            }*/
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
};