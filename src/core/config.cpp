#include "core/config.h"


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

    return true;
}

void Config::Reload(ActionManager &actionManager, FreeCamera &freeCamera) {
    Logger::Info("Loading config...");
    CreateModDirectory();

    mINI::INIFile file(GetConfigFilePath());
    mINI::INIStructure ini;

    bool fileExists = file.read(ini);

    Logger::Enable(ReadValue("mod", "debug_console", 0, ini));
    freeCamera.SetDefaultSpeed(ReadValue("settings", "default_camera_speed", 10.0f, ini));
    freeCamera.SetSpeedMult(ReadValue("settings", "speed_multiplier", 2.5f, ini));
    freeCamera.SetZoomSpeed(ReadValue("settings", "zoom_speed", 0.7f, ini));
    freeCamera.SetMinFov(ReadValue("settings", "min_fov(in radians)", 0.0001f, ini));
    freeCamera.SetMaxFov(ReadValue("settings", "max_fov(in radians)", 2.71f, ini));
    freeCamera.SetHideHud(ReadValue("settings", "hide_hud", 1, ini));
    freeCamera.SetFreezeEntities(ReadValue("settings", "freeze_entities", 1, ini));
    freeCamera.SetSmoothCamera(ReadValue("settings", "smooth_camera", 1, ini));

    for (const auto& kb : keybinds) {
        actionManager.BindAction(ReadKeybind(
            kb.section, kb.name, kb.type, kb.defaultKeys, kb.defaultModifiers, ini
        ));
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

Action Config::ReadKeybind(
    std::string section,
    std::string name,
    Action::Type actionType,
    const std::vector<int>& defaultkeyBind,
    const std::vector<int>& defaultRestricted,
    mINI::INIStructure& ini
) {
    if (ini.has(section)) {
        auto& collection = ini[section];

        if (collection.has(name)) {
            std::vector<int> keyBind;
            std::vector<int> restricted;

            std::string keyBindStr = collection[name];
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
                        keyBind.push_back(key);
                }
            }

            return Action(actionType, keyBind, restricted);
        }
    }

    std::vector<std::string> allTokens;

    for (int k : defaultkeyBind)
        allTokens.push_back(KeyToString(k));

    for (int k : defaultRestricted)
        allTokens.push_back("!" + KeyToString(k));

    std::string finalStr;
    for (size_t i = 0; i < allTokens.size(); ++i) {
        finalStr += allTokens[i];
        if (i != allTokens.size() - 1)
            finalStr += " + ";
    }

    ini[section][name] = finalStr;

    return Action(actionType, defaultkeyBind, defaultRestricted);
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