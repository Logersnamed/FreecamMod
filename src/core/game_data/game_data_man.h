#pragma once
#include "core/game_data/common.h"

namespace GameData {
#pragma pack(push, 1)
	struct OptionData {
		char pad1[0x09];
		std::byte HUD;						// 0x09
	};

	struct GameDataMan {
		char pad1[0x58];
		OptionData* optionData;				// 0x58
	};
#pragma pack(pop)

	ASSERT_OFFSET(OptionData, HUD, 0x09);
	ASSERT_OFFSET(GameDataMan, optionData, 0x58);
}