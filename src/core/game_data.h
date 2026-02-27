#pragma once
#include "utils/types.h"
#include <cstddef>
#include <type_traits>

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

		inline bool isFreecamEnabled() const { return freeCameraMode; }
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

	static_assert(offsetof(Camera, matrix) == 0x10, "Camera::matrix offset incorrect");
	static_assert(offsetof(Camera, fov) == 0x50, "Camera::fov offset incorrect");
	static_assert(offsetof(Camera, renderDistance) == 0x5C, "Camera::renderDistance offset incorrect");
	static_assert(sizeof(Camera) == 0x60, "Camera size incorrect");

	static_assert(offsetof(GameRend, csPersCam0) == 0x18, "GameRend::csPersCam0 offset incorrect");
	static_assert(offsetof(GameRend, csPersCam1) == 0x20, "GameRend::csPersCam1 offset incorrect");
	static_assert(offsetof(GameRend, csPersCam2) == 0x28, "GameRend::csPersCam2 offset incorrect");
	static_assert(offsetof(GameRend, freeCameraMode) == 0xC8, "GameRend::freeCameraMode offset incorrect");
	static_assert(offsetof(GameRend, csDebugCam) == 0xD0, "GameRend::csDebugCam offset incorrect");
	static_assert(sizeof(GameRend) == 0xD8, "GameRend size incorrect");

	static_assert(offsetof(FieldArea, gameRend) == 0x20, "FieldArea::gameRend offset incorrect");
	static_assert(sizeof(FieldArea) == 0x28, "FieldArea size incorrect");

	static_assert(offsetof(ChrIns, noMove) == 0x531, "ChrIns::noMove offset incorrect");

	static_assert(offsetof(Players, player0) == 0x00, "Players::player0 offset incorrect");
	static_assert(sizeof(Players) == 0x08, "Players size incorrect");

	static_assert(offsetof(WorldChrMan, players) == 0x10EF8, "WorldChrMan::players offset incorrect");
}