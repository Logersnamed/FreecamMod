#pragma once
#include <cstddef>
#include <type_traits>

#include "utils/types.h"

#define ASSERT_OFFSET(type, field, offset) static_assert(offsetof(type, field) == offset, #type "::" #field " offset incorrect")
#define ASSERT_SIZE(type, size) static_assert(sizeof(type) == size, #type " size incorrect")

namespace GameData {
	enum FreecamMode {
		Disabled = 0,
		Enabled = 1,
		EnabledUpdating = 2,
		Fixed = 3,
	};

#pragma pack(push, 1)
	struct Camera {
		char pad1[0x10];
		matrix4x4 matrix;					// 0x10
		float fov;							// 0x50
		char pad3[0x08];
		float renderDistance;				// 0x5C
	};

	struct GameRend {
		char pad1[0x18];
		Camera* csPersCam0;					// 0x18
		Camera* csPersCam1;					// 0x20	
		Camera* csPersCam2;					// 0x28
		char pad2[0x98];
		FreecamMode freeCameraMode;			// 0xC8
		char pad3[0x04];
		Camera* csDebugCam;					// 0xD0

		bool isFreecamEnabled() const { return freeCameraMode != FreecamMode::Disabled; }
	};

	struct FieldArea {
		char pad1[0x20];
		GameRend* gameRend;					// 0x20
	};

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
#pragma pack(pop)

	ASSERT_OFFSET(Camera, matrix, 0x10);
	ASSERT_OFFSET(Camera, fov, 0x50);
	ASSERT_OFFSET(Camera, renderDistance, 0x5C);
	ASSERT_SIZE(Camera, 0x60);
	
	ASSERT_OFFSET(GameRend, csPersCam0, 0x18);
	ASSERT_OFFSET(GameRend, csPersCam1, 0x20);
	ASSERT_OFFSET(GameRend, csPersCam2, 0x28);
	ASSERT_OFFSET(GameRend, freeCameraMode, 0xC8);
	ASSERT_OFFSET(GameRend, csDebugCam, 0xD0);
	ASSERT_SIZE(GameRend, 0xD8);

	ASSERT_OFFSET(FieldArea, gameRend, 0x20);
	ASSERT_SIZE(FieldArea, 0x28);

	ASSERT_OFFSET(ChrIns, noMove, 0x531);

	ASSERT_OFFSET(Players, player0, 0x00);
	ASSERT_SIZE(Players, 0x08);

	ASSERT_OFFSET(WorldChrMan, players, 0x10EF8);
}