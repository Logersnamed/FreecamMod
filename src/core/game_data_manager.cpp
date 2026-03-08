#include "core/game_data_manager.h"

#include "utils/memory.h"
#include "utils/debug.h"

uintptr_t GameDataManager::fieldAreaSig = 0;
uintptr_t GameDataManager::worldChrManSig = 0;
uintptr_t GameDataManager::gameDataManSig = 0;

bool GameDataManager::Init() {
	Logger::Info("Initializing GameDataManager...");
	Logger::Info("Scanning for fieldAreaSig...");
	fieldAreaSig = Signature("48 8B 3D ? ? ? ? 49 8B D8 48 8B F2 4C 8B F1 48 85 FF").Scan().Add(3).Rip().As<uint64_t>();
	if (!fieldAreaSig) {
		Logger::Error("Failed to find FieldArea signature");
		return false;
	}
	Logger::Info("fieldAreaSig: %p", fieldAreaSig);

	Logger::Info("Scanning for worldChrManSig...");
	worldChrManSig = Signature("48 8B 05 ? ? ? ? 48 85 C0 74 0F 48 39 88").Scan().Add(3).Rip().As<uint64_t>();
	if (!worldChrManSig) {
		Logger::Error("Failed to find WorldChrMan signature");
		return false;
	}
	Logger::Info("worldChrManSig: %p", worldChrManSig);

	Logger::Info("Scanning for gameDataMan...");
	gameDataManSig = Signature("48 8B 05 ? ? ? ? 48 85 C0 74 05 48 8B 40 58 C3 C3").Scan().Add(3).Rip().As<uint64_t>();
	if (!gameDataManSig) {
		Logger::Error("Failed to find GameDataManSig signature");
		return false;
	}
	Logger::Info("gameDataManSig: %p", gameDataManSig);

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