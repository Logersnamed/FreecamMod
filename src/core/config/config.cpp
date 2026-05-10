#include "config.h"

#include <cctype>
#include <stdio.h>
#include <string_view>

#include "utils/math.h"

bool Config::Initialize(HMODULE hModule, ActionManager& actionMgr) {
    if (!findDllPath(hModule)) return false;

    modDirectoryPath = dllPath / modDirectoryName;
    configFilePath = modDirectoryPath / configFileName;

    std::filesystem::create_directories(modDirectoryPath);

    Logger::InitFile(modDirectoryPath);
    LOG_INFO("Initializing Config...");
    LOG_INFO("DLL Path: %s", dllPath.string().c_str());
    LOG_INFO("Config path: %s", configFilePath.string().c_str());

    file = mINI::INIFile(configFilePath);
    if (!file) {
        LOG_ERROR("Failed to initialize INI file");
        return false;
	}

    this->actionMgr = &actionMgr;
    if (!this->actionMgr) {
        LOG_ERROR("ActionManager pointer is null");
        return false;
	}

    return true;
}

void Config::Reload() {
    std::filesystem::create_directories(modDirectoryPath);

    ini.clear();
    bool fileExists = file->read(ini);

    for (auto* conVar : IConVar::allConVars) {
        UpdateConVar(conVar);
	}

    for (const Keybind& keybind : keybinds) {
        actionMgr->BindAction(ReadKeybind(keybind));
    }

    if (fileExists) {
        if (!file->write(ini, true)) LOG_WARN("Failed to write to config file");
    }
    else {
        if (!file->generate(ini, true)) LOG_WARN("Failed to generate config file");
    }

    for (auto& callback : onReloadCallbacks) callback();
}

void Config::UpdateConVar(IConVar* conVar) {
    const char* section = conVar->GetSection();
    const char* name = conVar->GetName();

    if (ini.has(section)) {
        auto& collection = ini[section];

        if (collection.has(name)) {
            conVar->SetValueFromString(collection[name]);
            return;
        }
    }

	conVar->SetValueFromString(conVar->GetDefaultValueString());

    if (std::string_view(section) != "hidden") ini[section][name] = conVar->GetDefaultValueString().c_str();
}

Action Config::ReadKeybind(const Keybind& keybind) {
	static const char* keybindSection = "keybinds";
	const char* keybindName = keybind.name;
    const Action defaultAction = keybind.defaultAction;

    if (ini.has(keybindSection)) {
        auto& collection = ini[keybindSection];

        if (collection.has(keybindName)) {
            Action::Keys required;
            Action::Keys restricted;

            std::string keyBindStr = collection[keybindName];
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
                    if (isRestricted) {
                        if (!restricted.try_push_back(key))
                            LOG_WARN("Too many restricted keys for action \"%s\"", defaultAction.GetName());
                    }
                    else {
                        if (!required.try_push_back(key))
                            LOG_WARN("Too many required keys for action \"%s\"", defaultAction.GetName());
                    }
                }
            }

            return Action(defaultAction.GetType(), required, restricted);
        }
    }

    std::vector<std::string> allTokens;

    for (auto key : defaultAction.GetRequiredKeys())
        allTokens.push_back(KeyToString(key));

    for (auto key : defaultAction.GetRestrictedKeys())
        allTokens.push_back("!" + KeyToString(key));

    std::string finalStr;
    for (size_t i = 0; i < allTokens.size(); ++i) {
        finalStr += allTokens[i];
        if (i != allTokens.size() - 1)
            finalStr += " + ";
    }

    ini[keybindSection][keybindName] = finalStr;

    return defaultAction;
}

bool Config::findDllPath(HMODULE hModule) {
    char path[MAX_PATH];
    if (!GetModuleFileNameA(hModule, path, sizeof(path))) {
        LOG_ERROR("Failed to get module file name");
        return false;
    }

    std::filesystem::path p(path);
    dllPath = p.parent_path();
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