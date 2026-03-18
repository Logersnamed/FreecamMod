#pragma once
#include <iostream>
#include <string>
#include <fstream>

#include "utils/debug.h"

class SettingsBackup {
	inline static std::string path{};
	inline static int savedHudValue = -1;

public:
	static void SetFolderPath(const std::string& folderPath) {
		if (path.empty()) return;
		path = folderPath + "settings.bak";
	}

	static int RestoreHudValue() {
		if (path.empty()) return -1;

		std::ifstream file(path);
		if (!file.is_open()) return -1;

		std::string data;
		if (!std::getline(file, data)) return -1;

		if (data.size() < 2) return -1;

		bool isEnabled = data[0] == '1';
		int hudValue = data[1] - '0';

		if (!isEnabled) return -1;
		if (hudValue > 2 || hudValue < 0) return -1;

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

		file << (enabled ? '1' : '0') << hudValue;
	}
};