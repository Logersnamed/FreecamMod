#include "core/game_data_manager.h"

#include "ModUtils.h"

#include "utils/memory.h"
#include "utils/debug.h"

bool GameDataManager::Init() {
	LOG_INFO("Initializing GameDataManager...");

	SigEntry signatures[] = {
		{ "fieldAreaSig",   "48 8B 3D ? ? ? ? 49 8B D8 48 8B F2 4C 8B F1 48 85 FF", 3, true, true, &fieldAreaSig },
		{ "worldChrManSig", "48 8B 05 ? ? ? ? 48 85 C0 74 0F 48 39 88",				3, true, true, &worldChrManSig },
		{ "gameDataManSig",	"48 8B 05 ? ? ? ? 48 85 C0 74 05 48 8B 40 58 C3 C3",	3, true, false, &gameDataManSig },
		{ "frametimeLimitSig",	"C7 ? ? ? ? ? ? EB ? 89 ? 18 EB ? 89 ? 18 C7",		3, false, false, &frametimeLimitSig },
		{ "fullscreenLimitSig",	"C7 ? EF ? 00 00 00 C7 ? F3 01 00 00 00 8B 87",		0, false, false, &fullscreenLimitSig },
		{ "gamePauseSig",		"0F 84 ? ? ? ? C6 ? ? ? ? ? 00 ? 8D ? ? ? ? ? ? 89 ? ? 89 ? ? ? 8B ? ? ? ? ? ? 85 ? 75", 
			1, false, false, &gamePauseSig },
		{ "updateCameraMatrixFuncSig",	"4C 8B 49 18 4C 8B D1 8B 42 50 41 89 41 50 8B 42", 0, false, true, &updateCameraMatrixFuncSig },
		{ "daytimeUpdateSig",	"F3 0F 2C D0 85 D2 7E", 0, false, false, &daytimeUpdateSig },
	};

	for (SigEntry& sig : signatures) {
		LOG_INFO("Scanning for %s, AOB: \"%s\", offset: %d...", sig.name, sig.pattern, sig.offset);

		*sig.result = Signature(sig.pattern).Scan().Add(sig.offset).Rip(sig.isRip).As<uintptr_t>();
		if (!*sig.result) {
			if (sig.isObligatory) {
				LOG_ERROR("Failed to find %s", sig.name);
				return false;
			}

			LOG_ERROR("Failed to find %s. Some features may won't work", sig.name);
			continue;
		}

		LOG_INFO("%s: %p", sig.name, *sig.result);
	}
	
	return true;
}

GameData::FieldArea* GameDataManager::GetFieldArea() {
	uintptr_t fieldAreaPtr = Memory::RPM<uintptr_t>(fieldAreaSig);
	if (!fieldAreaPtr) return nullptr;

	return reinterpret_cast<GameData::FieldArea*>(fieldAreaPtr);
}

GameData::WorldChrMan* GameDataManager::GetWorldChrMan() {
	uintptr_t worldChrManPtr = Memory::RPM<uintptr_t>(worldChrManSig);
	if (!worldChrManPtr) return nullptr;

	return reinterpret_cast<GameData::WorldChrMan*>(worldChrManPtr);
}

GameData::GameDataMan* GameDataManager::GetGameDataMan() {
	uintptr_t gameDataManPtr = Memory::RPM<uintptr_t>(gameDataManSig);
	if (!gameDataManPtr) return nullptr;

	return reinterpret_cast<GameData::GameDataMan*>(gameDataManPtr);
}

GameData::ChrIns* GameDataManager::GetPlayer() {
	GameData::WorldChrMan* worldCharMan = GetWorldChrMan();
	if (!worldCharMan) return nullptr;

	GameData::Players* players = worldCharMan->players;
	if (!players) return nullptr;

	return players->player0;
}

GameData::OptionData* GameDataManager::GetOptionData() {
	GameData::GameDataMan* gameDataMan = GetGameDataMan();
	if (!gameDataMan) return nullptr;

	return gameDataMan->optionData;
}

void* GameDataManager::GetUpdateCameraMatrixFunc() {
	return reinterpret_cast<void*>(updateCameraMatrixFuncSig);
}

uintptr_t GameDataManager::GetDaytimeUpdateFunc() {
	return daytimeUpdateSig;
}

uintptr_t GameDataManager::GetFrametimeLimitAddress() {
	return frametimeLimitSig;
}

uintptr_t GameDataManager::GetFullscreenLimit() {
	return fullscreenLimitSig;
}

void GameDataManager::PauseGame(bool enabled) {
	if (!gamePauseSig) {
		LOG_WARN("Game pausing is not possible, gamePauseSig wasn't found.");
		return;
	}

	if (isGamePaused == enabled) return;

	LOG_INFO("Pausing Game...");
	if (enabled) {
		ModUtils::ReplaceExpectedBytesAtAddress(gamePauseSig, "84", "85");
	}
	else {
		ModUtils::ReplaceExpectedBytesAtAddress(gamePauseSig, "85", "84");
	}

	isGamePaused = enabled;
}