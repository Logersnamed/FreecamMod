#pragma once
#include "core/game_data/common.h"

namespace GameData {
#pragma pack(push, 1)
	struct OptionData {
		char pad1[0x09];
		uint8_t HUD;						// 0x09
	};

	struct GameDataMan {
		char pad1[0x58];
		OptionData* optionData;				// 0x58
	};

	struct Window {
		char pad1[0xFC];
		uint8_t AA;							// 0xFC
		char pad2[0x0B];
		uint8_t motionBlur;					// 0x108
	};
#pragma pack(pop)

	ASSERT_OFFSET(OptionData, HUD, 0x09);
	ASSERT_OFFSET(GameDataMan, optionData, 0x58);

	ASSERT_OFFSET(Window, AA, 0xFC);
	ASSERT_OFFSET(Window, motionBlur, 0x108);
}