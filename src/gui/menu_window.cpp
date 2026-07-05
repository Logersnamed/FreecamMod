#include "gui/menu_window.h"

#include <format>
#include "shellapi.h"

#include "imgui.h"

#include "core/config/config.h"
#include "core/free_camera.h"
#include "core/features/speedhack.h"
#include "core/game_data_manager.h"
#include "gui/notification_popup.h"
#include "gui/helpers.h"
#include "hook/hook_manager.h"

namespace Layout {
    constexpr float ITEM_WIDTH = -150.0f;
    constexpr float BUTTON_WIDTH = 150.0f;
    constexpr float SMALL_BUTTON_WIDTH = 110.0f;

    inline void RightAlignNext(float buttonWidth) {
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - buttonWidth);
    }
}

namespace ImGui {
    static inline void BeginScrollableArea(const char* str_id) {
        ImGui::SetCursorPosX(0);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(ImGui::GetStyle().WindowPadding.x, 0));
        ImGui::BeginChild(str_id, ImVec2(ImGui::GetWindowWidth(), 0), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground);
        ImGui::PopStyleVar();
    }

    static inline void EndScrollableArea() {
        ImGui::EndChild();
    }
}

void MenuWindow::Render() {
    if (!is_visible) return;

    ImGui::SetNextWindowSize(ImVec2(420, 360), ImGuiCond_FirstUseEver);
    ImGui::Begin("Freecam v2.0.0", &is_visible, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (ImGui::BeginTabBar("##tabs")) {
        infoTab.Render();
        featuresTab.Render();
        sequencerTab.Render();
        configTab.Render();
        keyBindsTab.Render();
        logTab.Render();

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void MenuWindow::InfoTab::Render() {
    if (ImGui::BeginTabItem("Info")) {
        ImGui::BeginScrollableArea("##info_content");
        GameData::GameRend* gameRend = freeCamera.GetGameRend();

        const bool isFreecamEnabled = freeCamera.IsEnabled();
        GameData::Camera* activeCamera{};
        if (gameRend) {
            activeCamera = isFreecamEnabled ? gameRend->csDebugCam : gameRend->csPersCam1;
        }

        if (!gameRend || !activeCamera) {
            ImGui::Spacing();
            ImGui::TextDisabled("No active camera.");

            ImGui::Spacing();
            ImGui::SeparatorText("About");

            ImGui::BulletText("Freecam v2.0.0");
            ImGui::BulletText("Build date: %s", __DATE__);

            ImGui::BulletText("Wiki & docs:");
            ImGui::SameLine(0, 0);
            ImGui::TextLinkOpenURL("https://github.com/Logersnamed/FreecamMod/wiki ");

            ImGui::BulletText("Report a bug/feature: ");
            ImGui::SameLine(0, 0);
            ImGui::TextLinkOpenURL("https://github.com/Logersnamed/FreecamMod/issues");

            ImGui::Spacing();
            ImGui::SeparatorText("Quick Start");

            ImGui::BulletText("Toggle UI: [%s]", config.GetKeybindString(ActionType::ToggleMenu).c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            ImHelpers::Tooltip("Rebindable in config.ini");

            ImGui::BulletText("Toggle freecam: [%s]", config.GetKeybindString(ActionType::Toggle).c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            ImHelpers::Tooltip("Rebindable in config.ini");

            ImGui::BulletText("Move: [%s %s %s %s]", config.GetKeybindString(ActionType::MoveForward).c_str(), config.GetKeybindString(ActionType::MoveLeft).c_str(), config.GetKeybindString(ActionType::MoveBackward).c_str(), config.GetKeybindString(ActionType::MoveRight).c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            ImHelpers::Tooltip("Rebindable in config.ini");

            ImGui::BulletText("Move up / down: [%s] / [%s]", config.GetKeybindString(ActionType::MoveUp).c_str(), config.GetKeybindString(ActionType::MoveDown).c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            ImHelpers::Tooltip("Rebindable in config.ini");

            ImGui::BulletText("Tilt: [%s] / [%s]", config.GetKeybindString(ActionType::TiltLeft).c_str(), config.GetKeybindString(ActionType::TiltRight).c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            ImHelpers::Tooltip("Rebindable in config.ini");

            ImGui::Spacing();
            ImGui::SeparatorText("Tabs");

            ImGui::BulletText("Features");
            ImGui::SameLine();
            ImGui::BeginDisabled();
            ImGui::TextWrapped("- controls freecam features: speedhack, change weather/daytime, etc.");
            ImGui::EndDisabled();

            ImGui::BulletText("Timeline");
            ImGui::SameLine();
            ImGui::BeginDisabled();
            ImGui::TextWrapped("- set keyframes for the camera and play back smooth camera paths.");
            ImGui::EndDisabled();

            ImGui::BulletText("Config");
            ImGui::SameLine();
            ImGui::BeginDisabled();
            ImGui::TextWrapped("- adjust mod settings. Changes are saved across restarts.");
            ImGui::EndDisabled();

#ifdef _WIN32
            ImGui::Spacing();
            if (ImGui::Button("Open Config Folder", ImVec2(Layout::BUTTON_WIDTH, 0.0f))) {
                ShellExecuteW(NULL, L"open", config.GetConfigDirPath().c_str(), NULL, NULL, SW_SHOWNORMAL);
            }
#endif

            ImGui::EndScrollableArea();
            ImGui::EndTabItem();
            return;
        }

        {
            float centerOffset = (ImGui::GetFrameHeight() - ImGui::GetFontSize()) * 0.5f;
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + centerOffset);

            ImGui::PushStyleColor(ImGuiCol_Text, isFreecamEnabled
                ? ImVec4(0.4f, 0.9f, 0.4f, 1.0f)
                : ImVec4(0.9f, 0.4f, 0.4f, 1.0f));
            ImGui::Text(isFreecamEnabled ? "Freecam active" : "Freecam inactive");
            ImGui::PopStyleColor();

            Layout::RightAlignNext(Layout::BUTTON_WIDTH);
            if (ImGui::Button("Toggle Freecam", ImVec2(Layout::BUTTON_WIDTH, 0.0f)))
                if (gameRend) freeCamera.Toggle(gameRend);
        }

        ImGui::SeparatorText("Camera");
        ImGui::PushItemWidth(Layout::ITEM_WIDTH);
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
        if (ImGui::DragFloat("Speed", &speed, 0.1f, 0.0f, 0.0f, "%.2f")) {
            freeCamera.SetSpeed(speed);
        }
		ImHelpers::TooltipWithShortcut("Adjust camera fly speed.", std::format("{} + Scroll", config.GetKeybindString(ActionType::ScrollCameraSpeedModifier).c_str()).c_str());
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

        ImGui::PopItemWidth();
        ImGui::EndScrollableArea();
        ImGui::EndTabItem();
    }
}

MenuWindow::FeaturesTab::FeaturesTab(HookManager& hookManager, FreeCamera& freeCamera, Speedhack& speedhack, Config& config)
    : hookManager(hookManager), freeCamera(freeCamera), speedhack(speedhack), config(config),
    frameStepper(freeCamera.GetFrameStepper()), cameraStateMgr(freeCamera.GetCameraStateManager()),
    pathRecorder(freeCamera.GetPathRecorder()) {
}

void MenuWindow::FeaturesTab::RenderSpeedhack() {
    if (ImGui::CollapsingHeader("Speedhack")) {
        ImGui::PushItemWidth(Layout::ITEM_WIDTH);
        bool isFreecamOnly = speedhack.IsFreecamOnly();
        bool isAvailable = !isFreecamOnly || (isFreecamOnly && freeCamera.IsEnabled());
        ImGui::BeginDisabled(!isAvailable);
        if (ImGui::Button(speedhack.IsEnabled() ? "Disable" : "Enable", ImVec2(Layout::SMALL_BUTTON_WIDTH, 0.0f))) {
            speedhack.IsEnabled() ? speedhack.Disable() : speedhack.Enable();
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            if (!isAvailable) {
                ImGui::SetTooltip(
                    "Disabled by config: features_work_only_in_freecam.speedhack\n"
                    "Enable this option or turn on freecam mode to unlock."
                );
            }
            else {
				ImHelpers::TooltipWithShortcut("Enable/disable speedhack.", std::format("{}", config.GetKeybindString(ActionType::ToggleSpeedhack).c_str()).c_str());
            }
        }
        ImGui::EndDisabled();

        float timeScale = speedhack.GetSpeedhackSpeed();
        if (ImGui::DragFloat("Speedhack speed", &timeScale, 0.001)) {
            speedhack.SetSpeed(timeScale);
        }
		ImHelpers::TooltipWithShortcut("Adjust speedhack speed.", std::format("{} + Scroll", config.GetKeybindString(ActionType::ScrollSpeedhackModifier).c_str()).c_str());

        float gameSpeed = speedhack.GetGameSpeed();
        ImGui::BeginDisabled();
        ImGui::DragFloat("Current game speed", &gameSpeed);
        ImGui::EndDisabled();

        ImGui::PopItemWidth();
        ImGui::Spacing();
    }
}

void MenuWindow::FeaturesTab::RenderFrameStepper() {
    if (ImGui::CollapsingHeader("Frame stepper")) {
        ImGui::PushItemWidth(Layout::ITEM_WIDTH);
        int framesToStep = frameStepper.GetFramesToStep();
        int step = frameStepper.GetStep();

        ImGui::BeginDisabled();
        ImGui::DragInt("Frames to step left", &framesToStep);
        ImGui::EndDisabled();

        const int MIN_FRAMES_TO_STOP_STEPPING = 15;
        ImGui::BeginDisabled(!freeCamera.IsEnabled());
        bool canStopStepping = framesToStep && step >= MIN_FRAMES_TO_STOP_STEPPING;
        if (ImGui::Button(canStopStepping ? "Stop stepping" : "Step frames", ImVec2(Layout::SMALL_BUTTON_WIDTH, 0.0f))) {
            canStopStepping ? frameStepper.Reset() : frameStepper.StepFrames();
        }
		ImHelpers::TooltipWithShortcut("Step frames forward.", std::format("{}", config.GetKeybindString(ActionType::StepFrames).c_str()).c_str());
        ImGui::EndDisabled();

        if (ImGui::InputInt("Step", &step)) {
            frameStepper.SetStepFromUI(step);
            IConVar::anyChangeByUi = true;
        }
        ImGui::PopItemWidth();
        ImGui::Spacing();
    }
}

void MenuWindow::FeaturesTab::RenderCycleWeatherTime() {
    if (ImGui::CollapsingHeader("Cycle Weather Time")) {
        ImGui::PushItemWidth(Layout::ITEM_WIDTH);
        auto& cave = hookManager.GetDaytimeUpdateCave();
        bool isFreecamOnly = cave.IsFreecamOnly();
        bool isAvailable = !isFreecamOnly || (isFreecamOnly && freeCamera.IsEnabled());
        ImGui::BeginDisabled(!isAvailable);
        if (ImGui::Button(cave.IsCycleWeatherTime() ? "Stop Cycling" : "Cycle", ImVec2(Layout::SMALL_BUTTON_WIDTH, 0.0f))) {
            cave.ToggleCycleWeatherTime();
            NotificationPopUp::Notify("Info", cave.IsCycleWeatherTime() ? "Started cycling" : "Stoped cycling");
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            if (!isAvailable) {
                ImGui::SetTooltip(
                    "Disabled by config: features_work_only_in_freecam.cycle_weather_time\n"
                    "Enable this option or turn on freecam mode to unlock."
                );
            }
            else {
				ImHelpers::TooltipWithShortcut("Start/stop cycling weather and time.", std::format("{}", config.GetKeybindString(ActionType::CycleWeatherTime).c_str()).c_str());
            }
        }
        ImGui::EndDisabled();

        auto* cycleSpeed = cave.GetCycleSpeedPtr();
        if (cycleSpeed) {
            ImGui::DragInt("Cycle speed", cycleSpeed, 1000);
        }
        ImGui::PopItemWidth();
        ImGui::Spacing();
    }

}

void MenuWindow::FeaturesTab::RenderCameraStateManager() {
    if (ImGui::CollapsingHeader("Camera State Manager")) {
        ImGui::PushItemWidth(Layout::ITEM_WIDTH);
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

            ImGui::PopItemWidth();
            ImGui::Spacing();
        }
        ImGui::EndDisabled();
    }
}

void MenuWindow::FeaturesTab::RenderPathRecorder() {
    if (ImGui::CollapsingHeader("Path recorder")) {
        ImGui::PushItemWidth(Layout::ITEM_WIDTH);
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

            if (ImGui::Button(pathRecorder.IsRecording() ? "Stop Recording" : "Start Recording", ImVec2(Layout::BUTTON_WIDTH, 0.0f))) {
                pathRecorder.IsRecording() ? pathRecorder.EndRecord() : pathRecorder.StartRecord();
            }
			ImHelpers::TooltipWithShortcut("Start/stop recording camera path.", std::format("{}", config.GetKeybindString(ActionType::StartEndRecording).c_str()).c_str());

            int framesPlayed = pathRecorder.GetFramesPlayed();
            ImGui::BeginDisabled();
            ImGui::DragInt("Frames Played", &framesPlayed);
            ImGui::EndDisabled();

            //ImGui::SameLine();

            ImGui::BeginDisabled(framesRecorded < 1);
            if (ImGui::Button(pathRecorder.IsPlaying() ? "Stop Playing" : "Start Playing", ImVec2(Layout::BUTTON_WIDTH, 0.0f))) {
                pathRecorder.IsPlaying() ? pathRecorder.EndPlay() : pathRecorder.StartPlay();
            }
			ImHelpers::TooltipWithShortcut("Start/stop playing recorded camera path.", std::format("{}", config.GetKeybindString(ActionType::StartEndPlayingRecording).c_str()).c_str());
            ImGui::EndDisabled();

            ImGui::PopItemWidth();
            ImGui::Spacing();
        }
        ImGui::EndDisabled();
    }
}

void MenuWindow::FeaturesTab::Render() {
    if (ImGui::BeginTabItem("Features")) {
        ImGui::BeginScrollableArea("##features_content");

        RenderSpeedhack();
        RenderCycleWeatherTime();
        RenderFrameStepper();
        RenderCameraStateManager();
        RenderPathRecorder();

        ImGui::EndScrollableArea();
        ImGui::EndTabItem();
    }
}

template<typename T>
void MenuWindow::SequencerTab::DrawCombo(const char* label, Track<T>& track) {
    static const char* InterpolationTypeNames[] = { "Linear", "Catmull-Rom" };
    static const int count = IM_COUNTOF(InterpolationTypeNames);
    int current = (int)track.GetInterpolationType();
    ImGui::Combo(label, &current, InterpolationTypeNames, count);
    track.SetInterpolationType((InterpolationType)current);
}

void MenuWindow::SequencerTab::Render() {
    if (ImGui::BeginTabItem("Sequencer")) {
        ImGui::BeginScrollableArea("##sequencer_content");

        bool isVisible = timelineWindow.IsVisible();
        if (ImGui::Checkbox("Show timeline", &isVisible)) {
            timelineWindow.SetVisibility(isVisible);
        }

        float max_time = timeline.GetMaxTime();
        if (ImGui::DragFloat("Timeline lenght", &max_time, 1, 16.0f, 3600.0f, "%.f sec")) {
            timeline.SetMaxTime(max_time);
        }

        ImGui::Spacing();
        ImGui::SeparatorText("Interpolation type");

        DrawCombo("FOV##interp", timeline.GetFovTrack());
        DrawCombo("Position##interp", timeline.GetPosTrack());
        DrawCombo("Rotation##interp", timeline.GetRotTrack());

        TimelineConfig& config = timelineWindow.GetConfig();

        ImGui::Spacing();
        ImGui::SeparatorText("Timeline");

        ImGui::DragInt("Pixels per Second", &config.pixels_per_second, 1, 10, 1000);
        ImGui::DragInt("Sidebar Width", &config.sidebar_width, 1, 50, 1000);
        ImGui::DragInt("Track Height", &config.track_height, 1, 10, 200);

        ImGui::Checkbox("Enable Snap", &config.snap_enabled);
		ImHelpers::TooltipWithShortcut("Snap playhead/keyframes to grid when dragging", "Alt");

        ImGui::BeginDisabled(!config.snap_enabled);
        ImGui::DragInt("Snap Pixels", &config.snap_pixels, 1, 1, 100);
        ImGui::EndDisabled();

        ImGui::Spacing();
        ImGui::SeparatorText("Controls");
        ImGui::Text("[Space]"); ImGui::SameLine(); ImGui::TextDisabled("- Play/Pause");
		ImGui::Text("[O]"); ImGui::SameLine(); ImGui::TextDisabled("- Add all keyframes");
		ImGui::Text("[X]"); ImGui::SameLine(); ImGui::TextDisabled("- Delete selected keyframes");
		ImGui::Text("[Ctrl + A]"); ImGui::SameLine(); ImGui::TextDisabled("- Select all keyframes");
		ImGui::Text("[Ctrl + Scroll]"); ImGui::SameLine(); ImGui::TextDisabled("- Zoom timeline");
        ImGui::Text("[Shift + Scroll]"); ImGui::SameLine(); ImGui::TextDisabled("- Scroll timeline");
        ImGui::TextDisabled("Hold"); ImGui::SameLine(); ImGui::Text("[RMB]"); ImGui::SameLine(); ImGui::TextDisabled("in game area to look around");
        ImGui::TextDisabled("Hold"); ImGui::SameLine(); ImGui::Text("[Shift]"); ImGui::SameLine(); ImGui::TextDisabled("and click to select multiple keyframes");
		ImGui::TextDisabled("Hold"); ImGui::SameLine(); ImGui::Text("[Alt]"); ImGui::SameLine(); ImGui::TextDisabled("to disable playhead/keyframes snap to grid");

        ImGui::EndScrollableArea();
        ImGui::EndTabItem();
    }
}

void MenuWindow::ConfigTab::SortConVars() {
    if (!areSorted) {
        for (auto* conVar : IConVar::allConVars) {
            sortedConVars[conVar->GetSection()].push_back(conVar);
        }
        areSorted = true;
    }
}

void MenuWindow::ConfigTab::Render() {
    if (ImGui::BeginTabItem("Config")) {
        ImGui::BeginScrollableArea("##config_content");

        SortConVars();

        for (auto& [section, conVars] : sortedConVars) {
            ImGui::PushID(section.c_str());

            if (ImGui::CollapsingHeader(section.c_str())) {
                ImGui::PushItemWidth(Layout::ITEM_WIDTH);
                for (auto* conVar : conVars) {
                    ImGui::PushID(conVar->GetName());

                    bool isDefault = conVar->IsValueDefault();

                    // temp solution
                    if (std::string(conVar->GetName()) == "min_fov") {
                        auto* floatVar = dynamic_cast<ConVar<float>*>(conVar);
                        if (floatVar && *floatVar < 0.001f) {
                            isDefault = true;
                        }
                    }

                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, !isDefault);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                    if (ImGui::Button("*", ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()))) {
                        conVar->ResetFromUI();
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();

                    if (!isDefault && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                        ImGui::SetTooltip("Reset to default");
                    }

                    ImGui::SameLine();

                    conVar->Render();
                    if (conVar->WasChangedByUI()) IConVar::anyChangeByUi = true;

                    ImGui::PopID();
                }
                ImGui::PopItemWidth();
            }

            ImGui::PopID();
        }

        ImGui::EndScrollableArea();
        ImGui::EndTabItem();
    }
}

void MenuWindow::KeyBindsTab::Render() {
    if (ImGui::BeginTabItem("Keybinds")) {
        ImGui::BeginScrollableArea("##keybinds_content");
        ImGui::PushItemWidth(Layout::ITEM_WIDTH);

        auto& keybinds = config.GetKeybinds();

        static const char* capturingKeybind = nullptr;

        for (auto& keybind : keybinds) {
            std::string name = keybind.name;
            std::string keybindStr{};
            // fix: every frame getting string from file
            if (!config.GetKeybindString(keybind, &keybindStr)) continue;

            ImGui::PushID(keybind.name);

            ImGui::BeginDisabled();
            {
                bool isCapturing = (capturingKeybind == keybind.name);

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

                std::string capturedKeybindsStr = "";
                if (isCapturing) {
                    for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) {
                        if (!ImGui::IsKeyDown(key)) continue;
                        //if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyDown(key)) continue; 
                        std::string keyName = ImGui::GetKeyName(key);
                        capturedKeybindsStr += keyName + " ";
                    }
                }

                std::string label = isCapturing ? capturedKeybindsStr : keybindStr;
                if (ImGui::Button(label.c_str(), ImVec2(Layout::BUTTON_WIDTH, 0.0f))) {
                    capturingKeybind = keybind.name;
                    ImGui::SetNextFrameWantCaptureKeyboard(true);
                }

                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
            }
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip("Changing keybind from gui is currently not implemented");
            }

            ImGui::SameLine();

            ImGui::TextUnformatted(name.c_str());

            ImGui::PopID();
        }

        ImGui::PopItemWidth();
        ImGui::EndScrollableArea();
        ImGui::EndTabItem();
    }
}

void MenuWindow::LogTab::Render() {
    if (ImGui::BeginTabItem("Log")) {
        ImGui::BeginChild("##log_scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

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