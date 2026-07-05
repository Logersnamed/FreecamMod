#pragma once
#include "imgui.h"

namespace ImHelpers {
    inline void Tooltip(const char* text, ImGuiHoveredFlags flags = ImGuiHoveredFlags_DelayNormal) {
		if (ImGui::IsItemHovered(flags)) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted(text);
			ImGui::EndTooltip();
		}
    }

    inline void TooltipWithShortcut(const char* text, const char* shortcut, ImGuiHoveredFlags flags = ImGuiHoveredFlags_DelayNormal) {
        if (ImGui::IsItemHovered(flags)) {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted(text);
            ImGui::SameLine();
            ImGui::TextDisabled("[%s]", shortcut);
            ImGui::EndTooltip();
        }
    }
}
