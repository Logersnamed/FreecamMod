#pragma once
#include <windows.h>
#include <string>
#include <filesystem>
#include <type_traits>

#include "mini/ini.h"

#include "core/input/action.h"
#include "core/free_camera.h"
#include "utils/debug.h"

class Config {
public:
    struct Keybind {
        const char* section;
        const char* name;
        Action::Type type;
        std::vector<int> defaultKeys;
        std::vector<int> defaultModifiers = {};
    };

private:
    std::string dllPath = "";
    std::string configFilePath = "";
    std::string configDirPath = "";
    const std::string configDirectoryName = "Freecam\\";
    const std::string configFileName = "config.ini";

    const std::vector<Keybind> keybinds = {
        {"keybinds", "toggle", Action::Type::Toggle, { VK_F1 }},
        {"keybinds", "reload_config", Action::Type::ReloadConfig, { VK_F5 }},
        {"keybinds", "exit_mod", Action::Type::ExitMod, { VK_DELETE }},
        {"keybinds", "move_forward", Action::Type::MoveForward, { 'W' }},
        {"keybinds", "move_backward", Action::Type::MoveBackward, { 'S' }},
        {"keybinds", "move_left", Action::Type::MoveLeft, { 'A' }},
        {"keybinds", "move_right", Action::Type::MoveRight, { 'D' }},
        {"keybinds", "move_up", Action::Type::MoveUp, { VK_SPACE }},
        {"keybinds", "move_down", Action::Type::MoveDown, { VK_SHIFT }},
        {"keybinds", "sprint", Action::Type::Sprint, { VK_LBUTTON }},
        {"keybinds", "zoom_in", Action::Type::ZoomIn, { VK_OEM_PLUS }},
        {"keybinds", "zoom_out", Action::Type::ZoomOut, { VK_OEM_MINUS }},
        {"keybinds", "scroll_zoom_modifier", Action::Type::ScrollZoomModifier, { VK_CONTROL }},
        {"keybinds", "scroll_camera_speed_modifier", Action::Type::ScroolCameraSpeedModifier, {}, { VK_CONTROL }},
    };

public:
    bool Initialize(HMODULE hModule);
    void Reload(ActionManager& actionManager, FreeCamera& freeCamera);
	bool CreateModDirectory();

    template<typename T>
    T ReadValue(const std::string& section, const std::string& name, T defaultValue, mINI::INIStructure& ini) {
        if (ini.has(section)) {
            auto& collection = ini[section];
            if (collection.has(name)) {
                if (std::is_same<T, int>::value) {
                    return std::stoi(collection[name]);
                }
                else if (std::is_same<T, float>::value) {
                    return std::stof(collection[name]);
                }
                else {
                    Logger::Warn("Unsupported config ReadValue type for [%s] %s, using default value", section.c_str(), name.c_str());
                }
            }
        }

        ini[section][name] = std::to_string(defaultValue);
        return defaultValue;
    }

    Action ReadKeybind(std::string section, std::string name, Action::Type actionType, const std::vector<int>& defaultkeyBind, mINI::INIStructure& ini) {
        const std::vector<int> empty;
        return ReadKeybind(section, name, actionType, defaultkeyBind, empty, ini);
    }

    Action ReadKeybind(
        std::string section,
        std::string name,
        Action::Type actionType,
        const std::vector<int>& defaultkeyBind,
        const std::vector<int>& defaultRestricted,
        mINI::INIStructure& ini
    );

    const std::vector<Keybind>& GetKeybinds() const { return keybinds; }
    const std::string& GetDllPath() const { return dllPath; }
	const std::string& GetConfigFilePath() const { return configFilePath; }
	const std::string& GetConfigDirPath() const { return configDirPath; }
	const std::string& GetConfigDirectoryName() const { return configDirectoryName; }
	const std::string& GetConfigFileName() const { return configFileName; }

private:
    bool findDllPath(HMODULE hModule);
    int ParseKey(std::string key);
    std::string KeyToString(int key);

    std::unordered_map<std::string, int> keyMap = {
        {"LBUTTON", VK_LBUTTON},
        {"RBUTTON", VK_RBUTTON},
        {"MBUTTON", VK_MBUTTON},
        {"XBUTTON1", VK_XBUTTON1},
        {"XBUTTON2", VK_XBUTTON2},

        {"BACKSPACE", VK_BACK},
        {"TAB", VK_TAB},
        {"ENTER", VK_RETURN},
        {"SHIFT", VK_SHIFT},
        {"CTRL", VK_CONTROL},
        {"ALT", VK_MENU},
        {"PAUSE", VK_PAUSE},
        {"CAPSLOCK", VK_CAPITAL},
        {"ESC", VK_ESCAPE},
        {"SPACE", VK_SPACE},

        {"PAGEUP", VK_PRIOR},
        {"PAGEDOWN", VK_NEXT},
        {"END", VK_END},
        {"HOME", VK_HOME},
        {"LEFT", VK_LEFT},
        {"UP", VK_UP},
        {"RIGHT", VK_RIGHT},
        {"DOWN", VK_DOWN},

        {"PRINTSCREEN", VK_SNAPSHOT},
        {"INSERT", VK_INSERT},
        {"DELETE", VK_DELETE},

        {"NUM0", VK_NUMPAD0},
        {"NUM1", VK_NUMPAD1},
        {"NUM2", VK_NUMPAD2},
        {"NUM3", VK_NUMPAD3},
        {"NUM4", VK_NUMPAD4},
        {"NUM5", VK_NUMPAD5},
        {"NUM6", VK_NUMPAD6},
        {"NUM7", VK_NUMPAD7},
        {"NUM8", VK_NUMPAD8},
        {"NUM9", VK_NUMPAD9},

        {"MULTIPLY", VK_MULTIPLY},
        {"ADD", VK_ADD},
        {"SUBTRACT", VK_SUBTRACT},
        {"DECIMAL", VK_DECIMAL},
        {"DIVIDE", VK_DIVIDE},

        {"F1", VK_F1},{"F2", VK_F2},{"F3", VK_F3},{"F4", VK_F4},
        {"F5", VK_F5},{"F6", VK_F6},{"F7", VK_F7},{"F8", VK_F8},
        {"F9", VK_F9},{"F10", VK_F10},{"F11", VK_F11},{"F12", VK_F12},

        {"LSHIFT", VK_LSHIFT},
        {"RSHIFT", VK_RSHIFT},
        {"LCTRL", VK_LCONTROL},
        {"RCTRL", VK_RCONTROL},
        {"LALT", VK_LMENU},
        {"RALT", VK_RMENU},
    };

    std::unordered_map<int, std::string> reverseKeyMap = {
        {VK_LBUTTON, "LBUTTON"},
        {VK_RBUTTON, "RBUTTON"},
        {VK_MBUTTON, "MBUTTON"},
        {VK_XBUTTON1, "XBUTTON1"},
        {VK_XBUTTON2, "XBUTTON2"},

        {VK_BACK, "BACKSPACE"},
        {VK_TAB, "TAB"},
        {VK_RETURN, "ENTER"},
        {VK_SHIFT, "SHIFT"},
        {VK_CONTROL, "CTRL"},
        {VK_MENU, "ALT"},
        {VK_PAUSE, "PAUSE"},
        {VK_CAPITAL, "CAPSLOCK"},
        {VK_ESCAPE, "ESC"},
        {VK_SPACE, "SPACE"},

        {VK_PRIOR, "PAGEUP"},
        {VK_NEXT, "PAGEDOWN"},
        {VK_END, "END"},
        {VK_HOME, "HOME"},
        {VK_LEFT, "LEFT"},
        {VK_UP, "UP"},
        {VK_RIGHT, "RIGHT"},
        {VK_DOWN, "DOWN"},

        {VK_SNAPSHOT, "PRINTSCREEN"},
        {VK_INSERT, "INSERT"},
        {VK_DELETE, "DELETE"},

        {VK_NUMPAD0, "NUM0"},
        {VK_NUMPAD1, "NUM1"},
        {VK_NUMPAD2, "NUM2"},
        {VK_NUMPAD3, "NUM3"},
        {VK_NUMPAD4, "NUM4"},
        {VK_NUMPAD5, "NUM5"},
        {VK_NUMPAD6, "NUM6"},
        {VK_NUMPAD7, "NUM7"},
        {VK_NUMPAD8, "NUM8"},
        {VK_NUMPAD9, "NUM9"},

        {VK_MULTIPLY, "MULTIPLY"},
        {VK_ADD, "ADD"},
        {VK_SUBTRACT, "SUBTRACT"},
        {VK_DECIMAL, "DECIMAL"},
        {VK_DIVIDE, "DIVIDE"},

        {VK_F1, "F1"}, {VK_F2, "F2"}, {VK_F3, "F3"}, {VK_F4, "F4"},
        {VK_F5, "F5"}, {VK_F6, "F6"}, {VK_F7, "F7"}, {VK_F8, "F8"},
        {VK_F9, "F9"}, {VK_F10, "F10"}, {VK_F11, "F11"}, {VK_F12, "F12"},

        {VK_LSHIFT, "LSHIFT"},
        {VK_RSHIFT, "RSHIFT"},
        {VK_LCONTROL, "LCTRL"},
        {VK_RCONTROL, "RCTRL"},
        {VK_LMENU, "LALT"},
        {VK_RMENU, "RALT"},
    };
};