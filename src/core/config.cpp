#include "core/config.h"

#include <filesystem>
#include <cctype>
#include <stdio.h>

#include "utils/math.h"

bool Config::Initialize(HMODULE hModule) {
    if (!findDllPath(hModule)) return false;

    configDirPath = dllPath + configDirectoryName;
    Logger::InitFile(configDirPath);
    LOG_INFO("Initializing Config...");
    LOG_INFO("DLL Path: %s", dllPath.c_str());

    if (configDirPath.empty()) {
        LOG_ERROR("Failed to determine config directory path");
		return false;
    }
    CreateModDirectory();

    configFilePath = configDirPath + configFileName;
    LOG_INFO("Config path: %s", configFilePath.c_str());

    file = mINI::INIFile(configFilePath);

    return true;
}

void Config::Reload(ActionManager &actionManager, FreeCamera &freeCamera) {
    CreateModDirectory();

    ini.clear();
    bool fileExists = file.read(ini);

#define READ(sec, key, var) var = ReadValue(sec, key, var)

    FreeCamera::Settings settings{};

    READ("freecam", "hide_hud", settings.flags.hideHud);
    READ("freecam", "freeze_game", settings.flags.freezeGame);
    READ("freecam", "freeze_entities", settings.flags.freezeEntities);
    READ("freecam", "freeze_player", settings.flags.freezePlayer);
    READ("freecam", "disable_player_controls", settings.flags.disablePlayerControls);
    READ("freecam", "reset_camera_settings", settings.flags.resetCameraSettings);
    READ("freecam", "always_use_custom_rotation", settings.flags.alwaysUseCustomRotation);

    READ("camera_settings", "sensitivity", settings.sensitivity);
    READ("camera_settings", "default_speed", settings.defaultSpeed);
    READ("camera_settings", "tilt_speed", settings.tiltSpeed);
    READ("camera_settings", "speed_multiplier", settings.speedMult);
    READ("camera_settings", "zoom_speed", settings.zoomSpeed);

    settings.minFov = Math::toRadians(ReadValue("camera_settings", "min_fov_degrees", Math::radToDegrees(settings.minFov)));
    settings.maxFov = Math::toRadians(ReadValue("camera_settings", "max_fov_degrees", Math::radToDegrees(settings.maxFov)));
    settings.pitchLimit = Math::toRadians(ReadValue("camera_settings", "pitch_limit_degrees", Math::radToDegrees(settings.pitchLimit)));

    READ("smooth_camera_settings", "smooth_camera_movement", settings.flags.smoothCameraMovement);
    READ("smooth_camera_settings", "smooth_camera_rotation", settings.flags.smoothCameraRotation);
    READ("smooth_camera_settings", "sensitivity", settings.smoothSensitivity);
    READ("smooth_camera_settings", "tilt_speed", settings.smoothTiltSpeed);

    READ("frame_stepper", "step", settings.step);

    READ("state_saver", "interpolation_time", settings.interpolationTime);

    Logger::Enable(ReadValue("hidden", "debug_console", 0));
    READ("hidden", "freeze_by_setting_zero_speed", settings.flags.zeroSpeedFreeze);

    freeCamera.SetSettings(settings);

    for (const Keybind& keybind : keybinds) {
        actionManager.BindAction(ReadKeybind(keybind));
    }

    if (fileExists) {
        if (!file.write(ini, true)) LOG_WARN("Failed to write to config file");
    }
    else {
        if (!file.generate(ini, true)) LOG_WARN("Failed to generate config file");
    }
}

bool Config::CreateModDirectory() {
    if (!std::filesystem::exists(configDirPath)) {
        LOG_INFO("Creating config directory...");
        try {
            std::filesystem::create_directories(configDirPath);
        }
        catch (...) {
            LOG_ERROR("Failed to create config directory");
            return false;
        }
    }

    return true;
}

template<typename T>
T Config::ReadValue(const std::string& section, const std::string& name, T defaultValue) {
    if (ini.has(section)) {
        auto& collection = ini[section];

        if (collection.has(name)) {
			auto& value = collection[name];

            try {
                if constexpr (std::is_same_v<T, int>) {
                    return std::stoi(value);
                }
                else if constexpr (std::is_same_v<T, float>) {
                    return std::stof(value);
                }
                else if constexpr (std::is_same_v<T, bool>) {
                    return value == "1" || value == "true";
                }
            }
            catch (...) {
                return defaultValue;
            }
        }
    }

    if (section != "hidden") ini[section][name] = std::to_string(defaultValue);
    return defaultValue;
}

Action Config::ReadKeybind(const Keybind& keybind) {
    if (ini.has(keybind.section)) {
        auto& collection = ini[keybind.section];

        if (collection.has(keybind.name)) {
            std::vector<int> keyStates;
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
                        keyStates.push_back(key);
                }
            }

            return Action(keybind.type, keyStates, restricted);
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
        LOG_ERROR("Failed to get module file name");
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

    for (const auto& k : keyMap)
        if (key == k.name) return k.vk;

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
    for (const auto& k : keyMap)
        if (key == k.vk) return k.name;

    if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9'))
        return std::string(1, (char)key);

    std::stringstream ss;
    ss << "0x" << std::hex << std::uppercase << key;
    return ss.str();
}