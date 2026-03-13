#pragma once
#include "core/game_data/common.h"

namespace GameData {
#pragma pack(push, 1)
	struct ChrFlags1 {
		uint8_t unk0 : 1;					// 0
		uint8_t unk1 : 1;
		uint8_t unk2 : 1;
		uint8_t noHit : 1;					// 3
		uint8_t noAttack : 1;				// 4
		uint8_t noMove : 1;					// 5
		uint8_t unk6 : 1;
		uint8_t unk7 : 1;
	};

	struct ChrFlags2 {
		uint8_t noUpdate : 1;				// 0
		uint8_t pad : 7;
	};

	struct ChrIns {
		char pad1[0x530];
		ChrFlags1 flags1;					// 0x530
		ChrFlags2 flags2;					// 0x531
	};

	struct Players {
		ChrIns* player0;					// 0x00
		char pad1[0x08];					
		ChrIns* player1;					// 0x10

		bool IsPlayerAlone() const { return !player1; }
	};

	struct WorldChrMan {
		char pad1[0x10EF8];
		Players* players;					// 0x10EF8
		char pad2[0xE2B8];
		ChrIns** begin;						// 0x1F1B8
		ChrIns** end;						// 0x1F1C0

		size_t GetEntityListLenght() const { return end - begin; }
	};
#pragma pack(pop)

	ASSERT_SIZE(ChrFlags1, 1);
	ASSERT_SIZE(ChrFlags2, 1);
 
	ASSERT_OFFSET(ChrIns, flags1, 0x530);
	ASSERT_OFFSET(ChrIns, flags2, 0x531);

	ASSERT_OFFSET(Players, player0, 0x00);
	ASSERT_OFFSET(Players, player1, 0x10);

	ASSERT_OFFSET(WorldChrMan, players, 0x10EF8);
	ASSERT_OFFSET(WorldChrMan, begin, 0x1F1B8);
	ASSERT_OFFSET(WorldChrMan, end, 0x1F1C0);
}
