#include "gui/gui.h"

#include <format>

#include "imgui.h"
#include "imgui_impl_win32.h"

#include "gui/overlay.h"
#include "core/events.h"
#include "core/config/config.h"
#include "core/free_camera.h"
#include "core/features/speedhack.h"
#include "core/game_data_manager.h"
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

void GUI::InfoTab::Render() {
    if (ImGui::BeginTabItem("Info")) {
        ImGui::BeginScrollableArea("##info_content");
        GameData::GameRend* gameRend = freeCamera.GetGameRend();

        if (!gameRend) {
            ImGui::Spacing();
            ImGui::TextDisabled("World isn't loaded.");
            ImGui::EndScrollableArea();
            ImGui::EndTabItem();
            return;
        }

        const bool isFreecamEnabled = freeCamera.IsEnabled();
        GameData::Camera* activeCamera = isFreecamEnabled ? gameRend->csDebugCam : gameRend->csPersCam1;

        if (!activeCamera) {
            ImGui::Spacing();
            ImGui::TextDisabled("No active camera.");
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

        ImGui::PopItemWidth();
        ImGui::EndScrollableArea();
        ImGui::EndTabItem();
    }
}

GUI::FeaturesTab::FeaturesTab(HookManager& hookManager, FreeCamera& freeCamera, Speedhack& speedhack)
    : hookManager(hookManager), freeCamera(freeCamera), speedhack(speedhack),
    frameStepper(freeCamera.GetFrameStepper()), cameraStateMgr(freeCamera.GetCameraStateManager()),
    pathRecorder(freeCamera.GetPathRecorder()) {
}

void GUI::FeaturesTab::RenderSpeedhack() {
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
        }
        ImGui::EndDisabled();

        float timeScale = speedhack.GetSpeedhackSpeed();
        if (ImGui::DragFloat("Speedhack speed", &timeScale, 0.001)) {
            speedhack.SetSpeed(timeScale);
        }

        float gameSpeed = speedhack.GetGameSpeed();
        ImGui::BeginDisabled();
        ImGui::DragFloat("Current game speed", &gameSpeed);
        ImGui::EndDisabled();

        ImGui::PopItemWidth();
        ImGui::Spacing();
    }
}

void GUI::FeaturesTab::RenderFrameStepper() {
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
        ImGui::EndDisabled();

        if (ImGui::InputInt("Step", &step)) {
            frameStepper.SetStepFromUI(step);
            IConVar::anyChangeByUi = true;
        }
        ImGui::PopItemWidth();
        ImGui::Spacing();
    }
}

void GUI::FeaturesTab::RenderCycleWeatherTime() {
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

void GUI::FeaturesTab::RenderCameraStateManager() {
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

void GUI::FeaturesTab::RenderPathRecorder() {
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

            int framesPlayed = pathRecorder.GetFramesPlayed();
            ImGui::BeginDisabled();
            ImGui::DragInt("Frames Played", &framesPlayed);
            ImGui::EndDisabled();

            //ImGui::SameLine();

            ImGui::BeginDisabled(framesRecorded < 1);
            if (ImGui::Button(pathRecorder.IsPlaying() ? "Stop Playing" : "Start Playing", ImVec2(Layout::BUTTON_WIDTH, 0.0f))) {
                pathRecorder.IsPlaying() ? pathRecorder.EndPlay() : pathRecorder.StartPlay();
            }
            ImGui::EndDisabled();

            ImGui::PopItemWidth();
            ImGui::Spacing();
        }
        ImGui::EndDisabled();
    }
}

void GUI::FeaturesTab::Render() {
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
void GUI::SequencerTab::DrawCombo(const char* label, Track<T>& track) {
    static const char* InterpolationTypeNames[] = { "Linear", "Catmull-Rom" };
    static const int count = IM_COUNTOF(InterpolationTypeNames);
    int current = (int)track.GetInterpolationType();
    ImGui::Combo(label, &current, InterpolationTypeNames, count);
    track.SetInterpolationType((InterpolationType)current);
}

void GUI::SequencerTab::Render() {
    if (ImGui::BeginTabItem("Sequencer")) {
        ImGui::BeginScrollableArea("##sequencer_content");

        bool isVisible = timelineWindow.IsVisible();
        if (ImGui::Checkbox("Show timeline", &isVisible)) {
            timelineWindow.SetVisibility(isVisible);
            if (isVisible) {
                static bool was_notified = false;
                if (!was_notified) NotificationPopUp::Notify("Info", "Click and hold the left mouse button in the game view to look around", 5);
                was_notified = true;
            }
        }

        float max_time = timeline.GetMaxTime();
        if (ImGui::DragFloat("Timeline lenght", &max_time, 1, 16.0f, 3600.0f, "%.f sec")) {
            timeline.SetMaxTime(max_time);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Interpolation type");

        DrawCombo("FOV##interp", timeline.GetFovTrack());
        DrawCombo("Position##interp", timeline.GetPosTrack());
        DrawCombo("Rotation##interp", timeline.GetRotTrack());

        ImGui::Spacing();
        ImGui::Separator();

        TimelineConfig& config = timelineWindow.GetConfig();

        ImGui::SeparatorText("Timeline");

        ImGui::DragInt("Pixels per Second", &config.pixels_per_second, 1, 10, 1000);
        ImGui::DragInt("Sidebar Width", &config.sidebar_width, 1, 50, 1000);
        ImGui::DragInt("Track Height", &config.track_height, 1, 10, 200);

        ImGui::Checkbox("Enable Snap", &config.snap_enabled);

        ImGui::BeginDisabled(!config.snap_enabled);
        ImGui::DragInt("Snap Pixels", &config.snap_pixels, 1, 1, 100);
        ImGui::EndDisabled();

        ImGui::EndScrollableArea();
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

void GUI::KeyBindsTab::Render() {
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

void GUI::LogTab::Render() {
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

void GUI::InitializeStyle() {
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
}

void GUI::SubscribeEvents() {
    EventBus::Subscribe<Event::DPIChanged>([this](const Event::DPIChanged& event) { OnDpiChange(); });

    EventBus::Subscribe<Event::ToggleFreecam>([this](const Event::ToggleFreecam& event) {
        if (notifyFreecam)
            NotificationPopUp::Notify("Info", event.isEnabled ? "Free camera enabled" : "Free camera disabled");
        });
    EventBus::Subscribe<Event::ToggleSpeedhack>([this](const Event::ToggleSpeedhack& event) {
        if (notifySpeedhack)
            NotificationPopUp::Notify("Info", event.isEnabled ? "Speedhack enabled" : "Speedhack disabled");
        });
    EventBus::Subscribe<Event::FrameStepped>([this](const Event::FrameStepped& event) {
        if (notifyFrameStepped)
            NotificationPopUp::Notify("Info", std::format("{} frames stepped", event.framesStepped));
        });
    EventBus::Subscribe<Event::ToggleCycleWeatherTime>([this](const Event::ToggleCycleWeatherTime& event) {
        if (notifyCycleWeatherTime)
            NotificationPopUp::Notify("Info", event.isEnabled ? "Started cycling weather time" : "Finished cycling weather time");
        });
    EventBus::Subscribe<Event::Record>([this](const Event::Record& event) {
        if (notifyRecord)
            NotificationPopUp::Notify("Info", event.isEnabled ? "Started recording" : "Finished recording");
        });
    EventBus::Subscribe<Event::PlayRecord>([this](const Event::PlayRecord& event) {
        if (notifyPlayRecord)
            NotificationPopUp::Notify("Info", event.isEnabled ? "Started playing recording" : "Finished playing recording");
        });
    EventBus::Subscribe<Event::SaveState>([this](const Event::SaveState& event) {
        if (notifySaveState)
            NotificationPopUp::Notify("Info", std::format("Saved state [{}], position = ({:.2f}, {:.2f}, {:.2f})", event.slot, event.pos.x, event.pos.y, event.pos.z));
        });
    EventBus::Subscribe<Event::Interpolate>([this](const Event::Interpolate& event) {
        if (!notifyInterpolate) return;
        std::string slotsStr = "[";
        for (size_t i = 0; i < event.slots.size(); ++i) {
            if (i) slotsStr += ", ";
            slotsStr += std::to_string(event.slots[i]);
        }
        slotsStr += "]";
        NotificationPopUp::Notify("Info", event.isEnabled ? std::format("Interpolation started between states = {}", slotsStr) : "Interpolation ended");
        });
    EventBus::Subscribe<Event::StateQueued>([this](const Event::StateQueued& event) {
        if (notifyStateQueued)
            NotificationPopUp::Notify("Info", std::format("State [{}] queued", event.slot));
        });
}

void GUI::Initialize() {
    is_visible = showMenuOnStartup;

    static std::string iniPath = (config.GetConfigDirPath() / "imgui.ini").string();
    ImGui::GetIO().IniFilename = iniPath.c_str();
    LOG_INFO("Path to imgui.ini: %s", iniPath.c_str());

    SubscribeEvents();
    InitializeStyle();
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

void GUI::OnDpiChange() {
    float newScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    LOG_INFO("DPI changed. New scale = %d", newScale);

    ImGuiStyle& style = ImGui::GetStyle();
    style = baseStyle;
    style.ScaleAllSizes(newScale);
    style.FontScaleDpi = newScale;
}

void GUI::Render() {
    static bool old_should_block = false;
    bool should_block_actions = false;
    if (is_visible && timeline_window.IsVisible()) {
        should_block_actions = timeline_window.IsHovered() || !input.IsPressed(VK_LBUTTON);
    }
    if (old_should_block != should_block_actions) {
        EventBus::Emit(Event::BlockCameraMouseMoveInput{ should_block_actions });
        auto block = Event::BlockActions::None();
        if (should_block_actions) {
            block
                .With(ActionType::MoveForward)
                .With(ActionType::MoveBackward)
                .With(ActionType::MoveLeft)
                .With(ActionType::MoveRight)
                .With(ActionType::MoveUp)
                .With(ActionType::MoveDown)
                .With(ActionType::ZoomIn)
                .With(ActionType::ZoomOut)
                .With(ActionType::TiltLeft)
                .With(ActionType::TiltRight);
        }
        EventBus::Emit(block);
        old_should_block = should_block_actions;
    }

    timeline.Update(ImGui::GetIO().DeltaTime);

    IConVar::anyChangeByUi = false;

    if (actionMgr.IsJustPressed(ActionType::ToggleMenu, input)) {
        is_visible = !is_visible;
        HandleCursorVisibility();
    }

    if (enableNotifications) {
        NotificationPopUp::Render();
    }

    bool isSequencerMode = true;
    if (isSequencerMode) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags host_flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("##Dockspace", nullptr, host_flags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID("GameDockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f),
            ImGuiDockNodeFlags_PassthruCentralNode
        );

        ImGui::End();
    }

    if (!is_visible)
        return;

    timeline_window.Render();

    bool oldVisibility = is_visible;
    ImGui::SetNextWindowSize(ImVec2(420, 360), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(340, 280), ImVec2(700, 900));
    ImGui::Begin("Freecam v2.0.0-beta", &is_visible, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (oldVisibility != is_visible) {
        HandleCursorVisibility();
    }

    if (ImGui::BeginTabBar("##tabs")) {
        infoTab.Render();
        featuresTab.Render();
        sequencerTab.Render();
        configTab.Render();
        keyBindsTab.Render();
        logTab.Render();

        if (IConVar::anyChangeByUi)
            config.Reload();

        ImGui::EndTabBar();
    }

    ImGui::End();
}