#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <optional>

#include <filesystem>
#include <fstream>
#include <optional>
#include <array>

enum class OptionType : uint8_t {
	Freecam		= 0,	// 0 = disabled, 1 = enabled
	HUD			= 1,	// 0 = off, 1 = on, 2 = auto
	AA			= 2,	// 0 = off, 1 = low, 2 = medium, 3 = high
	MotionBlur	= 3,	// 0 = off, 1 = low, 2 = medium, 3 = high
	Count = 4
};

class SettingsBackup {
	static inline std::filesystem::path path{};
	static inline constexpr size_t FILE_SIZE = static_cast<size_t>(OptionType::Count);

public:
	static void SetFolderPath(const std::filesystem::path& folderPath) {
		if (folderPath.empty()) return;
		path = folderPath / "option_data.bak";
	}

	static void SaveOptionValue(OptionType type, uint8_t value) {
		auto file = OpenFile();
		if (!file) return;

		size_t pos = static_cast<size_t>(type);
		if (pos >= FILE_SIZE) return;

		file->seekp(pos);
		file->put(static_cast<char>('0' + value));
	}

	static std::optional<uint8_t> RestoreOptionValue(OptionType type) {
		auto file = OpenFile();
		if (!file) return std::nullopt;

		size_t pos = static_cast<size_t>(type);
		if (pos >= FILE_SIZE) return std::nullopt;

		file->seekg(pos);
		char c{};
		file->get(c);
		if (!file->good()) return std::nullopt;

		return static_cast<uint8_t>(c - '0');
	}

private:
	static std::optional<std::fstream> OpenFile() {
		if (path.empty()) return std::nullopt;

		std::fstream check(path, std::ios::in | std::ios::binary);
		bool valid = false;

		if (check.is_open()) {
			check.seekg(0, std::ios::end);
			valid = check.tellg() == FILE_SIZE;
		}
		check.close();

		if (!valid) {
			std::ofstream out(path, std::ios::binary | std::ios::trunc);
			if (!out.is_open()) return std::nullopt;

			std::array<char, FILE_SIZE> init{};
			init.fill('0');
			out.write(init.data(), init.size());
		}

		std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
		if (!file.is_open()) return std::nullopt;

		file.seekg(0);
		file.seekp(0);
		return file;
	}
};