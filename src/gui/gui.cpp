#include "gui/gui.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_internal.h"

#include "core/events.h"
#include "core/config/config.h"
#include "gui/notification_popup.h"
#include "hook/hook_manager.h"
  
void GUI::Initialize() {
    is_visible = showMenuOnStartup;
    is_cursor_visible = showMenuOnStartup;

    static std::string iniPath = (config.GetConfigDirPath() / "imgui.ini").string();
    ImGui::GetIO().IniFilename = iniPath.c_str();
    LOG_INFO("Path to imgui.ini: %s", iniPath.c_str());

    SubscribeEvents();
    InitializeStyle();
    HandleCursorVisibility();
}

void GUI::Update() {
    static bool old_should_block = false;
    bool should_block_actions = false;
    if (is_visible && timeline_window.IsVisible()) {
        should_block_actions = timeline_window.IsHovered() || !input.IsPressed(VK_RBUTTON);
    }

    input.SetInputSource(should_block_actions ? InputSource::Timeline : InputSource::Default);

    if (old_should_block != should_block_actions) {
        HandleCursorVisibility(!timeline_window.IsVisible() || should_block_actions);
        old_should_block = should_block_actions;
    }

    timeline.Update(ImGui::GetIO().DeltaTime);

    if (actionMgr.IsJustPressed(ActionType::ToggleMenu, input)) {
        is_visible = !is_visible;

        if (is_visible) {
            menu_window.SetVisibility(true);
        }

        HandleCursorVisibility();
    }
}

void GUI::Render() {
    Update();

    IConVar::anyChangeByUi = false;

    if (enableNotifications) {
        NotificationPopUp::Render();
    }

    if (!is_visible) return;

    SetupDockspace();
    timeline_window.Render();
    menu_window.Render();

    bool new_visible = menu_window.IsVisible() || timeline_window.IsVisible();
    if (is_visible != new_visible) {
        is_visible = new_visible;
        HandleCursorVisibility();
    }

    if (IConVar::anyChangeByUi)
        config.Reload();
}

void GUI::HandleCursorVisibility(bool draw_cursor_if_is_visible) {
    ImGuiIO& io = ImGui::GetIO();
    is_cursor_visible = is_visible && draw_cursor_if_is_visible;
    if (is_cursor_visible) {
        io.MouseDrawCursor = true;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    }
    else {
        io.MouseDrawCursor = false;
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
    }
}

BOOL WINAPI GUI::hkSetCursorPos(int X, int Y) {
    if (instance && !instance->is_cursor_visible) {
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

void GUI::OnDpiChange() {
    float newScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    LOG_INFO("DPI changed. New scale = %d", newScale);

    ImGuiStyle& style = ImGui::GetStyle();
    style = baseStyle;
    style.ScaleAllSizes(newScale);
    style.FontScaleDpi = newScale;
}

void GUI::SetupDockspace() {
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

    ImGui::Begin("##dockspace", nullptr, host_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("dockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f),
        ImGuiDockNodeFlags_PassthruCentralNode
    );

    static bool init = false;
    if (!init) {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGuiID main = dockspace_id;
        ImGuiID bottom = ImGui::DockBuilderSplitNode(main, ImGuiDir_Down, 0.15f, nullptr, &main);

        ImGui::DockBuilderDockWindow("Timeline###timeline", bottom);
        ImGui::DockBuilderFinish(dockspace_id);
        init = true;
    }

    ImGui::End();
}

void GUI::SubscribeEvents() {
    EventBus::Subscribe<Event::DPIChanged>([this](const Event::DPIChanged& event) { OnDpiChange(); });

    EventBus::Subscribe<Event::ToggleFreecam>([this](const Event::ToggleFreecam& event) {
        if (notifyFreecam)
            NotificationPopUp::Notify("Info", event.isEnabled ? "Free camera enabled" : "Free camera disabled");
        }
    );
    EventBus::Subscribe<Event::ToggleSpeedhack>([this](const Event::ToggleSpeedhack& event) {
        if (notifySpeedhack)
            NotificationPopUp::Notify("Info", event.isEnabled ? "Speedhack enabled" : "Speedhack disabled");
        }
    );
    EventBus::Subscribe<Event::FrameStepped>([this](const Event::FrameStepped& event) {
        if (notifyFrameStepped)
            NotificationPopUp::Notify("Info", std::format("{} frames stepped", event.framesStepped));
        }
    );
    EventBus::Subscribe<Event::ToggleCycleWeatherTime>([this](const Event::ToggleCycleWeatherTime& event) {
        if (notifyCycleWeatherTime)
            NotificationPopUp::Notify("Info", event.isEnabled ? "Started cycling weather time" : "Finished cycling weather time");
        }
    );
    EventBus::Subscribe<Event::Record>([this](const Event::Record& event) {
        if (notifyRecord)
            NotificationPopUp::Notify("Info", event.isEnabled ? "Started recording" : "Finished recording");
        }
    );
    EventBus::Subscribe<Event::PlayRecord>([this](const Event::PlayRecord& event) {
        if (notifyPlayRecord)
            NotificationPopUp::Notify("Info", event.isEnabled ? "Started playing recording" : "Finished playing recording");
        }
    );
    EventBus::Subscribe<Event::SaveState>([this](const Event::SaveState& event) {
        if (notifySaveState)
            NotificationPopUp::Notify("Info", std::format("Saved state [{}], position = ({:.2f}, {:.2f}, {:.2f})", event.slot, event.pos.x, event.pos.y, event.pos.z));
        }
    );
    EventBus::Subscribe<Event::Interpolate>([this](const Event::Interpolate& event) {
        if (!notifyInterpolate) return;
        std::string slotsStr = "[";
        for (size_t i = 0; i < event.slots.size(); ++i) {
            if (i) slotsStr += ", ";
            slotsStr += std::to_string(event.slots[i]);
        }
        slotsStr += "]";
        NotificationPopUp::Notify("Info", event.isEnabled ? std::format("Interpolation started between states = {}", slotsStr) : "Interpolation ended");
        }
    );
    EventBus::Subscribe<Event::StateQueued>([this](const Event::StateQueued& event) {
        if (notifyStateQueued)
            NotificationPopUp::Notify("Info", std::format("State [{}] queued", event.slot));
        }
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