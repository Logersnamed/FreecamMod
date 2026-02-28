#include "core/game_data_manager.h"
#include "utils/memory.h"

uintptr_t GameDataManager::fieldAreaSig = 0;
uintptr_t GameDataManager::worldChrManSig = 0;

bool GameDataManager::Init() {
	fieldAreaSig = Signature("48 8B 3D ? ? ? ? 49 8B D8 48 8B F2 4C 8B F1 48 85 FF").Scan().Add(3).Rip().As<uint64_t>();
	if (!fieldAreaSig) return false;

	worldChrManSig = Signature("48 8B 05 ? ? ? ? 48 85 C0 74 0F 48 39 88").Scan().Add(3).Rip().As<uint64_t>();
	if (!worldChrManSig) return false;

	return true;
}

GameData::FieldArea* GameDataManager::GetFieldArea() {
	uintptr_t fieldAreaPtr = Memory::RPM<uintptr_t>(fieldAreaSig);
	if (!fieldAreaPtr) return nullptr;

	return reinterpret_cast<GameData::FieldArea*>(fieldAreaPtr);
}

GameData::WorldChrMan* GameDataManager::GetWorldChrMan() {
	uintptr_t fieldAreaPtr = Memory::RPM<uintptr_t>(worldChrManSig);
	if (!fieldAreaPtr) return nullptr;

	return reinterpret_cast<GameData::WorldChrMan*>(fieldAreaPtr);
}

GameData::ChrIns* GameDataManager::GetPlayer() {
	GameData::WorldChrMan* worldCharMan = GameDataManager::GetWorldChrMan();
	if (!worldCharMan) return nullptr;

	GameData::Players* players = worldCharMan->players;
	if (!players) return nullptr;

	return players->player0;
}