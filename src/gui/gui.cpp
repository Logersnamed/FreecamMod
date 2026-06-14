#include "gui/gui.h"

void GUI::InfoTab::Render() {
    if (ImGui::BeginTabItem("Info")) {
        GameData::GameRend* gameRend = freeCamera.GetGameRend();

        if (!gameRend) {
            ImGui::TextDisabled("World isn't loaded.");
            ImGui::EndTabItem();
            return;
        }

        if (gameRend) {
            bool isFreecamEnabled = freeCamera.IsEnabled();
            GameData::Camera* activeCamera = isFreecamEnabled ? gameRend->csDebugCam : gameRend->csPersCam1;

            if (activeCamera) {
                {
                    float centerOffset = (ImGui::GetFrameHeight() - ImGui::GetFontSize()) * 0.5f;
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + centerOffset);

                    ImGui::PushStyleColor(ImGuiCol_Text, isFreecamEnabled
                        ? ImVec4(0.4f, 0.9f, 0.4f, 1.0f)
                        : ImVec4(0.9f, 0.4f, 0.4f, 1.0f));
                    ImGui::Text(isFreecamEnabled ? "Freecam active" : "Freecam inactive");
                    ImGui::PopStyleColor();

                    ImGui::SameLine();
                    ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 140.0f + ImGui::GetCursorPosX());
                    if (ImGui::Button("Toggle Freecam", ImVec2(140.0f, 0.0f)))
                        if (gameRend) freeCamera.Toggle(gameRend);
                }

                ImGui::Separator();

                ImGui::SeparatorText("Camera");
                ImGui::BeginDisabled(!isFreecamEnabled);

                auto pos = activeCamera->matrix.c3.xyz();
                if (ImGui::DragFloat3("Position", &pos.x, 0.1f, 0.0f, 0.0f, "%.2f")) {
                    activeCamera->matrix.c3.x = pos.x;
                    activeCamera->matrix.c3.y = pos.y;
                    activeCamera->matrix.c3.z = pos.z;
                }
                ImGui::SliderFloat("FOV", &activeCamera->fov, freeCamera.GetMinFov(), freeCamera.GetMaxFov(), "%.2f rad");
                ImGui::DragFloat("Render Distance", &activeCamera->renderDistance, 10.0f, 0.0f, 0.0f, "%.0f m");

                float speed = freeCamera.GetSpeed();
                if (ImGui::DragFloat("Speed", &speed, 0.1f, 0.0f, 0.0f, "%.2f"))
                    freeCamera.SetSpeed(speed);

                ImGui::EndDisabled();

                ImGui::Spacing();
                ImGui::SeparatorText("Rotation (read-only)");
                constexpr float radToDeg = 57.29577951308232f;
                const auto rot = activeCamera->GetEuler();
                float pitch = rot.pitch * radToDeg;
                float yaw = rot.yaw * radToDeg;
                float roll = rot.roll * radToDeg;

                ImGui::BeginDisabled();
                ImGui::InputFloat("Pitch", &pitch, 0.0f, 0.0f, "%.2f°");
                ImGui::InputFloat("Yaw", &yaw, 0.0f, 0.0f, "%.2f°");
                ImGui::InputFloat("Roll", &roll, 0.0f, 0.0f, "%.2f°");
                ImGui::EndDisabled();
            }
        }

        ImGui::EndTabItem();
    }
}

void GUI::FeaturesTab::RenderSpeedhack() {
    if (ImGui::CollapsingHeader("FIX: Speedhack")) {
        bool isFreecamOnly = speedhack.IsFreecamOnly();
        bool isAvailable = !isFreecamOnly || (isFreecamOnly && freeCamera.IsEnabled());
        ImGui::BeginDisabled(!isAvailable);
        if (ImGui::Button(speedhack.IsEnabled() ? "Disable" : "Enable")) {
            speedhack.IsEnabled() ? speedhack.Disable() : speedhack.Enable();
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            if (!isAvailable) {
                ImGui::SetTooltip(
                    "Disabled by config: features_work_only_in_freecam.speedhack\n"
                    "Enable this option or turn on freecam mode to unlock."
                );
            }
        }
        ImGui::EndDisabled();

        float timeScale = speedhack.GetTimeScale();
        if (ImGui::DragFloat("Time Scale", &timeScale, 0.001)) {
            speedhack.SetTimeScale(timeScale);
        }

        ImGui::Spacing();
    }
}

void GUI::FeaturesTab::RenderFrameStepper() {
    if (ImGui::CollapsingHeader("Frame stepper")) {
        int framesToStep = frameStepper.GetFramesToStep();
        int step = frameStepper.GetStep();

        ImGui::BeginDisabled();
        ImGui::DragInt("Frames to step left", &framesToStep);
        ImGui::EndDisabled();

        const int MIN_FRAMES_TO_STOP_STEPPING = 15;
        ImGui::BeginDisabled(!freeCamera.IsEnabled());
        bool canStopStepping = framesToStep && step >= MIN_FRAMES_TO_STOP_STEPPING;
        if (ImGui::Button(canStopStepping ? "Stop stepping" : "Step frames")) {
            canStopStepping ? frameStepper.Reset() : frameStepper.StepFrames();
        }
        ImGui::EndDisabled();

        if (ImGui::InputInt("Step", &step)) {
            frameStepper.SetStepFromUI(step);
            IConVar::anyChangeByUi = true;
        }
        ImGui::Spacing();
    }
}

void GUI::FeaturesTab::RenderCycleWeatherTime() {
    if (ImGui::CollapsingHeader("Cycle Weather Time")) {
        auto& cave = hookManager.GetDaytimeUpdateCave();
        bool isFreecamOnly = cave.IsFreecamOnly();
        bool isAvailable = !isFreecamOnly || (isFreecamOnly && freeCamera.IsEnabled());
        ImGui::BeginDisabled(!isAvailable);
        if (ImGui::Button(cave.IsCycleWeatherTime() ? "Stop Cycling" : "Cycle")) {
            cave.ToggleCycleWeatherTime();
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            if (!isAvailable) {
                ImGui::SetTooltip(
                    "Disabled by config: features_work_only_in_freecam.cycle_weather_time\n"
                    "Enable this option or turn on freecam mode to unlock."
                );
            }
        }
        ImGui::EndDisabled();

        auto* cycleSpeed = cave.GetCycleSpeedPtr();
        if (cycleSpeed) {
            ImGui::DragInt("Cycle speed", cycleSpeed, 1000);
        }
        ImGui::Spacing();
    }

}

void GUI::FeaturesTab::RenderCameraStateManager() {
    if (ImGui::CollapsingHeader("Camera State Manager")) {
        ImGui::BeginDisabled(!freeCamera.IsEnabled());
        {
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                if (!freeCamera.IsEnabled()) {
                    ImGui::SetTooltip("Camera State Manager works only when free camera enabled.");
                }
            }

            float interpTime = cameraStateMgr.GetInterpolationTime();
            if (ImGui::DragFloat("Interpolation Time", &interpTime, 0.01, 0.0f)) {
                cameraStateMgr.SetInterpolationTimeFromUI(interpTime);
            }

            ImGui::Text("States order: ");
            Input::ReleasedNumkeys order = cameraStateMgr.GetSlotOrder();
            for (auto k : order) {
                ImGui::SameLine();
                ImGui::Text("%d", k);
            }

            float time = cameraStateMgr.GetTime();
            ImGui::BeginDisabled();
            ImGui::DragFloat("Time", &time);
            ImGui::EndDisabled();

            int interval = cameraStateMgr.GetInterval();
            ImGui::BeginDisabled();
            ImGui::DragInt("Interval", &interval);
            ImGui::EndDisabled();

            ImGui::Spacing();
        }
        ImGui::EndDisabled();
    }
}

void GUI::FeaturesTab::RenderPathRecorder() {
    if (ImGui::CollapsingHeader("Path recorder")) {
        ImGui::BeginDisabled(!freeCamera.IsEnabled());
        {
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                if (!freeCamera.IsEnabled()) {
                    ImGui::SetTooltip("Path Recorder works only when free camera enabled.");
                }
            }

            int framesRecorded = pathRecorder.GetFramesRecorded();
            ImGui::BeginDisabled();
            ImGui::DragInt("Frames Recorded", &framesRecorded);
            ImGui::EndDisabled();

            //ImGui::SameLine();

            if (ImGui::Button(pathRecorder.IsRecording() ? "Stop Recording" : "Start Recording")) {
                pathRecorder.IsRecording() ? pathRecorder.EndRecord() : pathRecorder.StartRecord();
            }

            int framesPlayed = pathRecorder.GetFramesPlayed();
            ImGui::BeginDisabled();
            ImGui::DragInt("Frames Played", &framesPlayed);
            ImGui::EndDisabled();

            //ImGui::SameLine();

            ImGui::BeginDisabled(framesRecorded < 1);
            if (ImGui::Button(pathRecorder.IsPlaying() ? "Stop Playing Recording" : "Start Playing Recording")) {
                pathRecorder.IsPlaying() ? pathRecorder.EndPlay() : pathRecorder.StartPlay();
            }
            ImGui::EndDisabled();

            ImGui::Spacing();
        }
        ImGui::EndDisabled();
    }
}

void GUI::FeaturesTab::Render() {
    if (ImGui::BeginTabItem("Features")) {
        RenderSpeedhack();
        RenderFrameStepper();
        RenderCycleWeatherTime();
        RenderCameraStateManager();
        RenderPathRecorder();

        ImGui::EndTabItem();
    }
}

void GUI::ConfigTab::SortConVars() {
    if (!areSorted) {
        for (auto* conVar : IConVar::allConVars) {
            sortedConVars[conVar->GetSection()].push_back(conVar);
        }
        areSorted = true;
    }
}

void GUI::ConfigTab::Render() {
    if (ImGui::BeginTabItem("Config")) {
        SortConVars();

        for (auto& [section, conVars] : sortedConVars) {
            ImGui::PushID(section.c_str());

            if (ImGui::CollapsingHeader(section.c_str())) {
                for (auto* conVar : conVars) {
                    ImGui::PushID(conVar->GetName());

                    conVar->Render();
                    if (conVar->WasChangedByUI()) IConVar::anyChangeByUi = true;

                    ImGui::PopID();
                }
            }

            ImGui::PopID();
        }

        ImGui::EndTabItem();
    }
}



void GUI::KeyBindsTab::Render() {
    if (ImGui::BeginTabItem("Keybinds")) {

        ImGui::EndTabItem();
    }
}


void GUI::LogTab::Render() {
    if (ImGui::BeginTabItem("Log")) {
        ImGui::BeginChild("Log");

        const auto& lines = Logger::GetLogLines();

        ImGuiListClipper clipper;
        clipper.Begin((int)lines.size());

        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                ImGui::TextUnformatted(lines[i].c_str());
            }
        }

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10.0f)
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::EndTabItem();
    }
}

BOOL WINAPI GUI::hkSetCursorPos(int X, int Y) {
    if (instance && !instance->is_visible) {
        return origSetCursorPos(X, Y);
    }
    return TRUE;
}

bool GUI::RegisterHooks(HookManager& hookManager) {
    return hookManager.Hook(
        GetProcAddress(GetModuleHandleW(L"user32"), "SetCursorPos"),
        &hkSetCursorPos,
        (void**)&origSetCursorPos
    );
}

void GUI::Initialize() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Layout
    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.IndentSpacing = 14.0f;
    style.ScrollbarSize = 11.0f;
    style.GrabMinSize = 10.0f;
    style.CellPadding = ImVec2(6, 4);

    // Rounding
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ChildRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;

    // Borders
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.TabBorderSize = 0.0f;
    style.SeparatorTextBorderSize = 1.0f;

    // Palette
    const ImVec4 bg1 = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
    const ImVec4 bg2 = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    const ImVec4 bg3 = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
    const ImVec4 bg4 = ImVec4(0.14f, 0.14f, 0.17f, 1.00f);
    const ImVec4 bg5 = ImVec4(0.17f, 0.17f, 0.20f, 1.00f);
    const ImVec4 bg6 = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
    const ImVec4 bg7 = ImVec4(0.18f, 0.18f, 0.21f, 1.00f);
    const ImVec4 bg8 = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
    const ImVec4 bg9 = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);

    const ImVec4 ui0 = ImVec4(0.22f, 0.22f, 0.25f, 1.00f);
    const ImVec4 ui1 = ImVec4(0.44f, 0.44f, 0.47f, 1.00f);
    const ImVec4 ui2 = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    const ImVec4 ui3 = ImVec4(0.40f, 0.40f, 0.45f, 1.00f);

    const ImVec4 text = ImVec4(0.88f, 0.88f, 0.90f, 1.00f);

    const ImVec4 acc0 = ImVec4(0.29f, 0.50f, 0.83f, 1.00f);
    const ImVec4 acc1 = ImVec4(0.38f, 0.62f, 0.90f, 1.00f);
    const ImVec4 acc2 = ImVec4(0.19f, 0.38f, 0.69f, 1.00f);

    auto with_alpha = [](ImVec4 v, float a) { v.w = a; return v; };

    ImVec4* colors = style.Colors;

    // Text
    colors[ImGuiCol_Text] = text;
    colors[ImGuiCol_TextDisabled] = ui1;

    // Backgrounds
    colors[ImGuiCol_WindowBg] = bg1;
    colors[ImGuiCol_ChildBg] = bg2;
    colors[ImGuiCol_PopupBg] = bg2;

    // Borders
    colors[ImGuiCol_Border] = ui0;
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frames
    colors[ImGuiCol_FrameBg] = bg3;
    colors[ImGuiCol_FrameBgHovered] = bg6;
    colors[ImGuiCol_FrameBgActive] = bg7;

    // Title
    colors[ImGuiCol_TitleBg] = bg1;
    colors[ImGuiCol_TitleBgActive] = bg2;
    colors[ImGuiCol_TitleBgCollapsed] = with_alpha(bg1, 0.75f);

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = bg1;
    colors[ImGuiCol_ScrollbarGrab] = ui0;
    colors[ImGuiCol_ScrollbarGrabHovered] = ui2;
    colors[ImGuiCol_ScrollbarGrabActive] = ui3;

    // Accent
    colors[ImGuiCol_CheckMark] = acc1;
    colors[ImGuiCol_SliderGrab] = acc0;
    colors[ImGuiCol_SliderGrabActive] = acc2;

    // Buttons
    colors[ImGuiCol_Button] = bg5;
    colors[ImGuiCol_ButtonHovered] = acc0;
    colors[ImGuiCol_ButtonActive] = acc2;

    // Headers
    colors[ImGuiCol_Header] = bg4;
    colors[ImGuiCol_HeaderHovered] = bg8;
    colors[ImGuiCol_HeaderActive] = bg9;

    // Separator
    colors[ImGuiCol_Separator] = ui0;
    colors[ImGuiCol_SeparatorHovered] = with_alpha(acc0, 0.78f);
    colors[ImGuiCol_SeparatorActive] = acc0;

    // Resize grip
    colors[ImGuiCol_ResizeGrip] = with_alpha(acc0, 0.15f);
    colors[ImGuiCol_ResizeGripHovered] = with_alpha(acc0, 0.50f);
    colors[ImGuiCol_ResizeGripActive] = with_alpha(acc0, 0.90f);

    // Tabs
    colors[ImGuiCol_Tab] = bg2;
    colors[ImGuiCol_TabHovered] = bg8;
    colors[ImGuiCol_TabSelected] = bg4;
    colors[ImGuiCol_TabSelectedOverline] = acc0;
    colors[ImGuiCol_TabDimmed] = ImVec4(0.07f, 0.07f, 0.08f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected] = bg3;
    colors[ImGuiCol_TabDimmedSelectedOverline] = with_alpha(acc0, 0.40f);

    // Misc
    colors[ImGuiCol_NavHighlight] = acc0;
    colors[ImGuiCol_NavWindowingHighlight] = with_alpha(acc0, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = with_alpha(bg1, 0.50f);
    colors[ImGuiCol_ModalWindowDimBg] = with_alpha(bg1, 0.60f);

    baseStyle = style;

    float dpiScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
    style.ScaleAllSizes(dpiScale);
    style.FontScaleDpi = dpiScale;

    HandleCursorVisibility();
}

void GUI::HandleCursorVisibility() {
    ImGuiIO& io = ImGui::GetIO();
    if (is_visible) {
        io.MouseDrawCursor = true;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    }
    else {
        io.MouseDrawCursor = false;
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
    }
}

void GUI::OnDpiUpdate() {
    // float newScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
    // ImGuiStyle& style = ImGui::GetStyle();
    // style = baseStyle;
    // dpiScale = newScale;
    // style.ScaleAllSizes(dpiScale);
    // style.FontScaleDpi = dpiScale;
}

void GUI::Render() {
    IConVar::anyChangeByUi = false;

    if (GetAsyncKeyState(VK_END) & 1) {
        is_visible = !is_visible;
        HandleCursorVisibility();
    }

    if (!is_visible)
        return;

    ImGui::Begin("Freecam v2.0.0-beta", &is_visible);

    if (ImGui::BeginTabBar("##tab_bar")) {
        infoTab.Render();
        featuresTab.Render();
        configTab.Render();
        keyBindsTab.Render();
        logTab.Render();

        if (IConVar::anyChangeByUi)
            config.Reload();

        ImGui::EndTabBar();
    }

    ImGui::End();
}