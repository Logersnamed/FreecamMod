#pragma once
#include "core/game_data/common.h"

namespace GameData {
	struct ChrIns {
		char pad1[0x531];
		bool noMove;						// 0x531
	};

	struct Players {
		ChrIns* player0;					// 0x00
	};

	struct WorldChrMan {
		char pad1[0x10EF8];
		Players* players;					// 0x10EF8
	};

	ASSERT_OFFSET(ChrIns, noMove, 0x531);
	ASSERT_OFFSET(Players, player0, 0x00);
	ASSERT_OFFSET(WorldChrMan, players, 0x10EF8);
}
