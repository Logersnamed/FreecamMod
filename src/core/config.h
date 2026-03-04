#pragma once
#include <windows.h>
#include <string>
#include <filesystem>
#include "mini/ini.h"
#include "utils/debug.h"

class Config {
public:
    std::string dllPath = "";
    std::string configFilePath = "";
	const std::string configDirectoryName = "Freecam\\";
	const std::string configFileName = "freecam.ini";

	bool Initialize(HMODULE hModule) {
		Logger::Info("Initializing Config...");
        if (!findDllPath(hModule)) return false;
		Logger::Info("DLL Path: %s", dllPath.c_str());

		std::string configPath = dllPath + configDirectoryName;
        if (!std::filesystem::exists(configPath)) {
			Logger::Info("Creating config directory...");

            if (!CreateDirectoryA(configPath.c_str(), NULL)) {
                Logger::Error("Failed to create config directory, error code: %d", GetLastError());
				return false;
            }
        }

		configFilePath = configPath + configFileName;
        Logger::Info("Config path: %s", configFilePath.c_str());

		return true;
	}

    float Read(std::string section, std::string name, float defaultValue, mINI::INIStructure &ini) {
        if (ini.has(section)) {
            auto& collection = ini[section];
            if (collection.has(name)) {
                return std::stof(collection[name]);
            }
        }

        ini[section][name] = std::to_string(defaultValue);
		return defaultValue;
    }

    int Read(std::string section, std::string name, int defaultValue, mINI::INIStructure& ini) {
        if (ini.has(section)) {
            auto& collection = ini[section];
            if (collection.has(name)) {
                return std::stoi(collection[name]);
            }
        }
    
        ini[section][name] = std::to_string(defaultValue);
        return defaultValue;
    }

    bool findDllPath(HMODULE hModule) {
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
};