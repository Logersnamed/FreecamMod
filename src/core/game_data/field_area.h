#pragma once
#include "core/game_data/common.h"

#include "utils/math.h"

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

		float3 right() const { return matrix.c0.xyz(); }
		float3 up() const { return matrix.c1.xyz(); }
		float3 forward() const { return matrix.c2.xyz(); }

		EulerAngles GetEuler() const {
			const float3 r = right();
			const float3 u = up();
			const float3 f = forward();

			const float yaw = std::atan2(f.x, f.z);
			const float pitch = std::asin(-f.y);
			const float roll = std::atan2(-r.y, u.y);

			return EulerAngles(yaw, pitch, roll);
		}
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

		bool IsFreecamEnabled() const { return freeCameraMode != FreecamMode::Disabled; }
		void DisableFreecam() { freeCameraMode = FreecamMode::Disabled; }
		void EnableFreecam(bool isDisablePlayerControls) {
			freeCameraMode = isDisablePlayerControls ? FreecamMode::EnabledUpdating : FreecamMode::Fixed;
		}
		Camera* GetActiveCamera() const { return (IsFreecamEnabled() ? csDebugCam : csPersCam1); }
	};

	struct FieldArea {
		char pad1[0x20];
		GameRend* gameRend;					// 0x20
	};
#pragma pack(pop)

	ASSERT_OFFSET(Camera, matrix, 0x10);
	ASSERT_OFFSET(Camera, fov, 0x50);
	ASSERT_OFFSET(Camera, renderDistance, 0x5C);

	ASSERT_OFFSET(GameRend, csPersCam0, 0x18);
	ASSERT_OFFSET(GameRend, csPersCam1, 0x20);
	ASSERT_OFFSET(GameRend, csPersCam2, 0x28);
	ASSERT_OFFSET(GameRend, freeCameraMode, 0xC8);
	ASSERT_OFFSET(GameRend, csDebugCam, 0xD0);

	ASSERT_OFFSET(FieldArea, gameRend, 0x20);
}