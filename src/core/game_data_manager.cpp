#include "core/game_data_manager.h"

#include "utils/memory.h"
#include "utils/debug.h"

uintptr_t GameDataManager::fieldAreaSig = 0;
uintptr_t GameDataManager::worldChrManSig = 0;
uintptr_t GameDataManager::gameDataManSig = 0;
uintptr_t GameDataManager::chrDbgFlagsSig = 0;

bool GameDataManager::Init() {
	Logger::Info("Initializing GameDataManager...");

	SigEntry signatures[] = {
		{ "fieldAreaSig",   "48 8B 3D ? ? ? ? 49 8B D8 48 8B F2 4C 8B F1 48 85 FF", 3, &fieldAreaSig },
		{ "worldChrManSig", "48 8B 05 ? ? ? ? 48 85 C0 74 0F 48 39 88",				3, &worldChrManSig },
		{ "gameDataManSig",	"48 8B 05 ? ? ? ? 48 85 C0 74 05 48 8B 40 58 C3 C3",	3, &gameDataManSig },
		{ "chrDbgFlagsSig",	"80 3D ? ? ? ? 00 0F 85 ? ? ? ? 32 C0 48",				2, &chrDbgFlagsSig },
	};

	for (SigEntry& sig : signatures) {
		Logger::Info("Scanning for %s, AOB: \"%s\", offset: %d...", sig.name, sig.pattern, sig.offset);

		*sig.result = Signature(sig.pattern).Scan().Add(sig.offset).Rip().As<uint64_t>();
		if (!*sig.result) {
			Logger::Error("Failed to find %s", sig.name);
			return false;
		}

		Logger::Info("%s: %p", sig.name, *sig.result);
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

std::byte* GameDataManager::GetChrDbgFlag(GameData::ChrDbgFlags flag) {
	if (!chrDbgFlagsSig) return nullptr;
	return reinterpret_cast<std::byte*>(chrDbgFlagsSig + (std::uint8_t)flag + 1);
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