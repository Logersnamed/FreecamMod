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

	static void PauseGame(bool enabled);

private:
	static bool isGamePaused;

	struct SigEntry {
		const char* name;
		const char* pattern;
		const int offset;
		bool isRip;
		bool isObligatory;
		uintptr_t* result;
	};

	static uintptr_t fieldAreaSig;
	static uintptr_t worldChrManSig;
	static uintptr_t gameDataManSig;
	static uintptr_t gamePauseSig;
};