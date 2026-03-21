#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <optional>

class SettingsBackup {
	static inline std::string path{};
	static inline int savedHudValue = -1;

public:
	static void SetFolderPath(const std::string& folderPath) {
		if (folderPath.empty()) return;
		path = folderPath + "settings.bak";
	}

	static std::optional<int> RestoreHudValue() {
		if (path.empty()) return std::nullopt;

		std::ifstream file(path);
		if (!file.is_open()) return std::nullopt;

		std::string data;
		if (!std::getline(file, data)) return std::nullopt;

		if (data.size() < 2) return std::nullopt;

		bool isEnabled = data[0] == '1';
		int hudValue = data[1] - '0';

		if (!isEnabled) return std::nullopt;
		if (hudValue > 2 || hudValue < 0) return std::nullopt;

		return hudValue;
	}

	static void SaveHudValue(int value) {
		savedHudValue = value;
		WriteData(true, savedHudValue);
	}

	static void SetEnabled(bool enabled) {
		WriteData(enabled, savedHudValue);
	}

	static void WriteData(bool enabled, int hudValue) {
		if (path.empty()) return;
		std::ofstream file(path);
		if (!file.is_open()) return;
		if (hudValue > 2 || hudValue < 0) {
			file << '0' << '0';
			return;
		}

		file << (enabled ? '1' : '0') << hudValue;
	}
};