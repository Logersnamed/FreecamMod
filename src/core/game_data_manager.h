#pragma once
#include "core/game_data/game_data.h"

class GameDataManager {
public:
	static bool Init();
	static GameData::FieldArea* GetFieldArea();
	static GameData::WorldChrMan* GetWorldChrMan();
	static GameData::GameDataMan* GetGameDataMan();
	static GameData::OptionData* GetOptionData();
	static GameData::ChrIns* GetPlayer();

	struct SigEntry {
		const char* name;
		const char* pattern;
		const int offset;
		uintptr_t* result;
	};

private:
	static uintptr_t fieldAreaSig;
	static uintptr_t worldChrManSig;
	static uintptr_t gameDataManSig;
};