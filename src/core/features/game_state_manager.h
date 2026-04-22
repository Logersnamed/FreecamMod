#pragma once
#include <cstddef>

#include "core/game_data/game_data.h"
#include "core/settings_backup.h"

class GameStateManager {
	bool isGameFrozen = false;
	bool areEntitiesFrozen = false;
	bool isPlayerFrozen = false;

	bool isZeroSpeedFreeze = false;

public:
	void FreezeGame(bool enabled);
	void FreezeEntities(bool enabled);
	void FreezePlayer(bool enabled);
	void FreezeEntity(GameData::ChrIns* entity, bool enabled);

	bool IsGameFrozen() const { return isGameFrozen; }
	bool AreEntitesFrozen() const { return areEntitiesFrozen; }
	bool IsPlayerFrozen() const { return isPlayerFrozen; }

	void SetZeroSpeedFreeze(bool enabled) { isZeroSpeedFreeze = enabled; }

private:
	struct Option {
		Option(OptionType type) : type(type) {}

		OptionType type = OptionType::None;

		bool isDisabled = false;
		uint8_t savedValue = 1;
		std::optional<uint8_t> valueToRestore = std::nullopt;

		void Disable(uint8_t* gameOption, bool enabled);
		void Restore(uint8_t* gameOption);
	};

	Option hud { OptionType::HUD };
	Option aa  { OptionType::AA };
	Option mb  { OptionType::MotionBlur };

	bool wereRestored = false;

public:
	void SetOptionValueToRestore(OptionType type, std::optional<uint8_t> value);
	void DisableOption(OptionType optionType, bool enabled);
	bool RestoreOptions();
};