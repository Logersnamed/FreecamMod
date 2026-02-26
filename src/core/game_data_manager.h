#pragma once
#include "core/game_data.h"

class GameDataManager {
public:
	static bool Init();
	static GameData::FieldArea* GetFieldArea();
	static GameData::WorldChrMan* GetWorldChrMan();

private:
	static uintptr_t fieldAreaSig;
	static uintptr_t worldChrManSig;
};