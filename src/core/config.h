#pragma once
#include <windows.h>
#include <string>
#include <type_traits>
#include <filesystem>
#include <array>

#include "mini/ini.h"

#include "core/input/action_system.h"
#include "core/free_camera.h"
#include "utils/debug.h"

class Config {
	using enum ActionType;

public:
    struct Keybind {
        const char* name;
        Action defaultAction;
    };

    bool Initialize(HMODULE hModule);
    void Reload(ActionManager& actionMgr, FreeCamera& freeCamera);

    std::filesystem::path GetConfigDirPath() const { return modDirectoryPath; }

private:
    mINI::INIFile file = mINI::INIFile("");
    mINI::INIStructure ini;

    std::filesystem::path dllPath;
    std::filesystem::path modDirectoryPath;
    std::filesystem::path configFilePath;

    const std::string modDirectoryName = "Freecam";
    const std::string configFileName = "config.ini";

    std::array<Keybind, static_cast<size_t>(ActionType::Count)> keybinds = {
        Keybind{"toggle", Action{ Toggle, { VK_F1 }}},
        Keybind{"reload_config", Action{ ReloadConfig, { VK_F5 }}},
        Keybind{"reset_settings", Action{ ResetSettings, { 'R' }, { VK_CONTROL }}},
        Keybind{"toggle_freeze", Action{ ToggleFreeze, { 'P' }}},
        Keybind{"teleport_to_camera", Action{ TeleportToCamera, { VK_F3 }}},
        Keybind{"cycle_weather_time", Action{ CycleWeatherTime, { VK_F4 }}},
        Keybind{"exit_mod", Action{ ExitMod, { VK_DELETE }}},
        Keybind{"start/end_recording", Action{ StartEndRecording, { VK_F8 }}},
        Keybind{"strat/end_playing_recording", Action{ StartEndPlayingRecording, { VK_F9}}},
        Keybind{"step_frames", Action{ StepFrames, { VK_F2}}},
        Keybind{"move_forward", Action{ MoveForward, { 'W' }}},
        Keybind{"move_backward", Action{ MoveBackward, { 'S' }}},
        Keybind{"move_left", Action{ MoveLeft, { 'A' }}},
        Keybind{"move_right", Action{ MoveRight, { 'D' }}},
        Keybind{"move_up", Action{ MoveUp, { VK_SPACE }}},
        Keybind{"move_down", Action{ MoveDown, { VK_SHIFT }}},
        Keybind{"sprint", Action{ Sprint, { VK_LBUTTON }}},
        Keybind{"zoom_in", Action{ ZoomIn, { VK_OEM_PLUS }}},
        Keybind{"zoom_out", Action{ ZoomOut, { VK_OEM_MINUS }}},
        Keybind{"tilt_left", Action{ TiltLeft, { 'Q' }}},
        Keybind{"tilt_right", Action{ TiltRight, { 'E'}}},
        Keybind{"scroll_zoom_modifier", Action{ ScrollZoomModifier, { VK_CONTROL }}},
        Keybind{"scroll_camera_speed_modifier", Action{ ScrollCameraSpeedModifier, {}, { VK_CONTROL, 'V' }}},
        Keybind{"scroll_speedhack_modifier", Action{ ScrollSpeedhackModifier, { 'V' }}},
        Keybind{"toggle_speedhack", Action{ ToggleSpeedhack, { VK_F7 }}},
        Keybind{"reset_speedhack_speed", Action{ ResetSpeedhackSpeed, { VK_CONTROL, 'V', 'R' }}},
    };

    bool findDllPath(HMODULE hModule);

    template<typename T>
    T ReadValue(const std::string& section, const std::string& name, T defaultValue);

    Action ReadKeybind(const Keybind& keybind);

    int ParseKey(std::string key);
    std::string KeyToString(int key);

    struct KeyPair {
        const char* name;
        int vk;
    };

    static constexpr KeyPair keyMap[] = {
        {"LBUTTON",     VK_LBUTTON},
        {"RBUTTON",     VK_RBUTTON},
        {"MBUTTON",     VK_MBUTTON},
        {"XBUTTON1",    VK_XBUTTON1},
        {"XBUTTON2",    VK_XBUTTON2},

        {"BACKSPACE",   VK_BACK},
        {"TAB",         VK_TAB},
        {"ENTER",       VK_RETURN},
        {"SHIFT",       VK_SHIFT},
        {"CTRL",        VK_CONTROL},
        {"ALT",         VK_MENU},
        {"PAUSE",       VK_PAUSE},
        {"CAPSLOCK",    VK_CAPITAL},
        {"ESC",         VK_ESCAPE},
        {"SPACE",       VK_SPACE},

        {"PAGEUP",      VK_PRIOR},
        {"PAGEDOWN",    VK_NEXT},
        {"END",         VK_END},
        {"HOME",        VK_HOME},
        {"LEFT",        VK_LEFT},
        {"UP",          VK_UP},
        {"RIGHT",       VK_RIGHT},
        {"DOWN",        VK_DOWN},

        {"PRINTSCREEN", VK_SNAPSHOT},
        {"INSERT",      VK_INSERT},
        {"DELETE",      VK_DELETE},

        {"NUM0",        VK_NUMPAD0},
        {"NUM1",        VK_NUMPAD1},
        {"NUM2",        VK_NUMPAD2},
        {"NUM3",        VK_NUMPAD3},
        {"NUM4",        VK_NUMPAD4},
        {"NUM5",        VK_NUMPAD5},
        {"NUM6",        VK_NUMPAD6},
        {"NUM7",        VK_NUMPAD7},
        {"NUM8",        VK_NUMPAD8},
        {"NUM9",        VK_NUMPAD9},

        {"MULTIPLY",    VK_MULTIPLY},
        {"ADD",         VK_ADD},
        {"SUBTRACT",    VK_SUBTRACT},
        {"DECIMAL",     VK_DECIMAL},
        {"DIVIDE",      VK_DIVIDE},

        {"F1", VK_F1},{"F2", VK_F2},{"F3", VK_F3},{"F4", VK_F4},
        {"F5", VK_F5},{"F6", VK_F6},{"F7", VK_F7},{"F8", VK_F8},
        {"F9", VK_F9},{"F10", VK_F10},{"F11", VK_F11},{"F12", VK_F12},

        {"LSHIFT",      VK_LSHIFT},
        {"RSHIFT",      VK_RSHIFT},
        {"LCTRL",       VK_LCONTROL},
        {"RCTRL",       VK_RCONTROL},
        {"LALT",        VK_LMENU},
        {"RALT",        VK_RMENU},
    };
};