#include "core/config.h"

#include <filesystem>

#include "utils/math.h"

bool Config::Initialize(HMODULE hModule) {
    if (!findDllPath(hModule)) return false;

    configDirPath = dllPath + configDirectoryName;
    Logger::InitFile(configDirPath);
    Logger::Info("Initializing Config...");
    Logger::Info("DLL Path: %s", dllPath.c_str());

    if (configDirPath.empty()) {
        Logger::Error("Failed to determine config directory path");
		return false;
    }
    CreateModDirectory();

    configFilePath = configDirPath + configFileName;
    Logger::Info("Config path: %s", configFilePath.c_str());

    file = mINI::INIFile(configFilePath);

    return true;
}

void Config::Reload(ActionManager &actionManager, FreeCamera &freeCamera) {
    Logger::Info("Loading config...");
    CreateModDirectory();

    ini.clear();
    bool fileExists = file.read(ini);

    Logger::Enable(ReadValue("mod", "debug_console", 0));

    freeCamera.SetHideHud(ReadValue("freecam", "hide_hud", 1));
    freeCamera.SetFreezeEntities(ReadValue("freecam", "freeze_entities", 1));
    freeCamera.SetFreezePlayer(ReadValue("freecam", "freeze_player", 1));
    freeCamera.SetDisablePlayerControls(ReadValue("freecam", "disable_player_controls", 1));
    freeCamera.SetSmoothCamera(ReadValue("freecam", "smooth_camera", 1));
    freeCamera.SetOnlyFreezeAnim(ReadValue("freecam", "only_freeze_animation", 0));

    freeCamera.SetDefaultSpeed(ReadValue("camera_settings", "default_speed", 10.0f));
    freeCamera.SetSpeedMult(ReadValue("camera_settings", "speed_multiplier", 2.5f));
    freeCamera.SetZoomSpeed(ReadValue("camera_settings", "zoom_speed", 0.7f));
    freeCamera.SetMinFov(Math::toRadians(ReadValue("camera_settings", "min_fov", Math::radToDegrees(0.0001f))));
    freeCamera.SetMaxFov(Math::toRadians(ReadValue("camera_settings", "max_fov", Math::radToDegrees(2.71f))));

    for (const Keybind& keybind : keybinds) {
        actionManager.BindAction(ReadKeybind(keybind));
    }

    if (fileExists) {
        if (!file.write(ini, true)) Logger::Warn("Failed to write to config file");
    }
    else {
        if (!file.generate(ini, true)) Logger::Warn("Failed to generate config file");
    }
}

bool Config::CreateModDirectory() {
    if (!std::filesystem::exists(configDirPath)) {
        Logger::Info("Creating config directory...");
        std::filesystem::create_directories(configDirPath);
    }

    return true;
}

template<typename T>
T Config::ReadValue(const std::string& section, const std::string& name, T defaultValue) {
    if (ini.has(section)) {
        auto& collection = ini[section];

        if (collection.has(name)) {
			auto& value = collection[name];

            if constexpr (std::is_same_v<T, int>) {
                return std::stoi(value);
            }
            else if constexpr (std::is_same_v<T, float>) {
                return std::stof(value);
            }
            else if constexpr (std::is_same_v<T, bool>) {
                return std::stoi(value) != 0;
            }
        }
    }

    ini[section][name] = std::to_string(defaultValue);
    return defaultValue;
}

Action Config::ReadKeybind(const Keybind& keybind) {
    if (ini.has(keybind.section)) {
        auto& collection = ini[keybind.section];

        if (collection.has(keybind.name)) {
            std::vector<int> keys;
            std::vector<int> restricted;

            std::string keyBindStr = collection[keybind.name];
            std::stringstream ss(keyBindStr);
            std::string token;

            while (std::getline(ss, token, '+')) {

                token.erase(
                    std::remove_if(token.begin(), token.end(), ::isspace),
                    token.end());

                bool isRestricted = false;

                if (!token.empty() && token[0] == '!') {
                    isRestricted = true;
                    token.erase(0, 1);
                }

                int key = ParseKey(token);

                if (key != -1) {
                    if (isRestricted)
                        restricted.push_back(key);
                    else
                        keys.push_back(key);
                }
            }

            return Action(keybind.type, keys, restricted);
        }
    }

    std::vector<std::string> allTokens;

    for (int k : keybind.defaultKeys)
        allTokens.push_back(KeyToString(k));

    for (int k : keybind.defaultRestricted)
        allTokens.push_back("!" + KeyToString(k));

    std::string finalStr;
    for (size_t i = 0; i < allTokens.size(); ++i) {
        finalStr += allTokens[i];
        if (i != allTokens.size() - 1)
            finalStr += " + ";
    }

    ini[keybind.section][keybind.name] = finalStr;

    return Action(keybind.type, keybind.defaultKeys, keybind.defaultRestricted);
}

bool Config::findDllPath(HMODULE hModule) {
    char path[MAX_PATH];

    if (!GetModuleFileNameA(hModule, path, sizeof(path))) {
        Logger::Error("Failed to get module file name");
        return false;
    }

    dllPath = path;

    size_t pos = dllPath.find_last_of("\\/");
    if (pos != std::string::npos)
        dllPath.erase(pos + 1);

    return true;
}

int Config::ParseKey(std::string key) {
    std::transform(key.begin(), key.end(), key.begin(), ::toupper);

    auto it = keyMap.find(key);
    if (it != keyMap.end())
        return it->second;

    if (key.size() == 1)
        return key[0];

    if (key.rfind("0X", 0) == 0)
        return std::stoi(key, nullptr, 16);

    try {
        return std::stoi(key);
    }
    catch (...) {
        return -1;
    }
}

std::string Config::KeyToString(int key) {
    if (reverseKeyMap.count(key))
        return reverseKeyMap[key];

    if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9'))
        return std::string(1, (char)key);

    std::stringstream ss;
    ss << "0x" << std::hex << std::uppercase << key;
    return ss.str();
}