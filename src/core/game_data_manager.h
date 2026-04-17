#pragma once
#include "core/game_data/game_data.h"

#include "ModUtils.h"

#include "utils/memory.h"
#include "utils/debug.h"

class GameDataManager {
	template<typename T>
	struct PatternEntry {
		int offset;
		bool resolveRip;
		bool required;
		const char* name;
		const char* pattern;

		uintptr_t address{};

		bool Scan() {
			address = Signature(pattern).Scan().Add(offset).Rip(resolveRip).As<uintptr_t>();
			if (!address) {
				if (required) {
					LOG_ERROR("Failed to find %s", name);
				}
				else {
					LOG_WARN("Failed to find %s. Some features may not work", name);
				}
				return false;
			}
			LOG_INFO("Found %s: %p", name, address);
			return true;
		}

		T Get() const {
			if constexpr (std::is_pointer_v<T>) {
				uintptr_t ptr = Memory::RPM<uintptr_t>(address);
				return ptr ? reinterpret_cast<T>(ptr) : nullptr;
			}
			else if constexpr (std::is_same_v<T, uintptr_t>) {
				return address;
			}
			else {
				return static_cast<T>(address);
			}
		}

		void* operator &() const {
			return reinterpret_cast<void*>(address);
		}
	};

public:
	static bool Init() {
		LOG_INFO("Initializing GameDataManager...");

		if (!FieldArea.Scan()) return false;
		if (!WorldChrMan.Scan()) return false;
		if (!GameDataMan.Scan()) return false;
		if (!FrametimeLimit.Scan()) return false;
		if (!FullscreenLimit.Scan()) return false;
		if (!GamePausePatch.Scan()) return false;
		if (!UpdateCameraMatrixFunc.Scan()) return false;
		if (!DaytimeUpdateFunc.Scan()) return false;

		return true;
	}

#define REQUIRED true
#define RESOLVE_RIP true
#define NOT_REQUIRED false
#define NOT_RESOLVE_RIP false

	static inline PatternEntry<GameData::FieldArea*> FieldArea { 3, RESOLVE_RIP, REQUIRED, "FieldArea", 
		"48 8B 3D ? ? ? ? 49 8B D8 48 8B F2 4C 8B F1 48 85 FF" };
	static inline PatternEntry<GameData::WorldChrMan*> WorldChrMan { 3, RESOLVE_RIP, REQUIRED, "WorldChrMan", 
		"48 8B 05 ? ? ? ? 48 85 C0 74 0F 48 39 88" };
	static inline PatternEntry<GameData::GameDataMan*> GameDataMan { 3, RESOLVE_RIP, NOT_REQUIRED, "GameDataMan", 
		"48 8B 05 ? ? ? ? 48 85 C0 74 05 48 8B 40 58 C3 C3" };
	static inline PatternEntry<uintptr_t> FrametimeLimit { 3, NOT_RESOLVE_RIP, NOT_REQUIRED, "FrametimeLimit", 
		"C7 ? ? ? ? ? ? EB ? 89 ? 18 EB ? 89 ? 18 C7" };
	static inline PatternEntry<uintptr_t> FullscreenLimit { 0, NOT_RESOLVE_RIP, NOT_REQUIRED, "FullscreenLimit", 
		"C7 ? EF ? 00 00 00 C7 ? F3 01 00 00 00 8B 87" };
	static inline PatternEntry<uintptr_t> GamePausePatch { 1, NOT_RESOLVE_RIP, NOT_REQUIRED, "GamePausePatch", 
		"0F 84 ? ? ? ? C6 ? ? ? ? ? 00 ? 8D ? ? ? ? ? ? 89 ? ? 89 ? ? ? 8B ? ? ? ? ? ? 85 ? 75" };
	static inline PatternEntry<void*> UpdateCameraMatrixFunc { 0, NOT_RESOLVE_RIP, REQUIRED, "UpdateCameraMatrixFunc", 
		"4C 8B 49 18 4C 8B D1 8B 42 50 41 89 41 50 8B 42" };
	static inline PatternEntry<uintptr_t> DaytimeUpdateFunc { 0, NOT_RESOLVE_RIP, NOT_REQUIRED, "DaytimeUpdateFunc", 
		"F3 0F 2C D0 85 D2 7E" };

	static GameData::ChrIns* GetPlayer() {
		GameData::WorldChrMan* world = WorldChrMan.Get();
		if (!world) return nullptr;

		GameData::Players* players = world->players;
		if (!players) return nullptr;

		return players->player0;
	}

	static GameData::OptionData* GetOptionData() {
		GameData::GameDataMan* gameDataMan = GameDataMan.Get();
		if (!gameDataMan) return nullptr;

		return gameDataMan->optionData;
	}

	static void PauseGame(bool enabled) {
		static bool isPaused = false;

		if (!GamePausePatch.address) return;
		if (isPaused == enabled) return;

		if (enabled) {
			ModUtils::ReplaceExpectedBytesAtAddress(GamePausePatch.address, "84", "85");
		}
		else {
			ModUtils::ReplaceExpectedBytesAtAddress(GamePausePatch.address, "85", "84");
		}

		isPaused = enabled;
	}
};