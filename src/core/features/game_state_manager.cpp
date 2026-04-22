#include "core/features/game_state_manager.h"
#include "core/game_data_manager.h"
#include "utils/debug.h"

void GameStateManager::FreezeGame(bool enabled) {
	if (isGameFrozen != enabled) {
		GameDataManager::PauseGame(enabled);
		isGameFrozen = enabled;
	}
}

void GameStateManager::FreezeEntities(bool enabled) {
	if (areEntitiesFrozen == enabled) return;
	areEntitiesFrozen = enabled;

	GameData::WorldChrMan* world = GameDataManager::WorldChrMan.Get();
	if (!world) return;

	GameData::Players* players = world->players;
	if (!players) return;

	if (!players->IsPlayerAlone()) {
		LOG_INFO("Freezing entities in online is disabled");
		return;
	}

	GameData::ChrIns* player = players->player0;

	const size_t length = world->GetEntityListLength();
	LOG_INFO("Set FreezeEntity = %d to %zu entities", enabled, length);
	for (size_t i = 0; i < length; ++i) {
		GameData::ChrIns* entity = world->begin[i];
		if (entity && entity != player) FreezeEntity(entity, enabled);
	}
}

void GameStateManager::FreezePlayer(bool enabled) {
	if (isPlayerFrozen == enabled) return;
	isPlayerFrozen = enabled;

	GameData::ChrIns* player = GameDataManager::GetPlayer();
	if (player) FreezeEntity(player, enabled);
}

void GameStateManager::FreezeEntity(GameData::ChrIns* entity, bool enabled) {
	if (isZeroSpeedFreeze) {
		entity->chrModules->chrBehavior->animationSpeed = !enabled;
	}
	else {
		entity->flags2.noUpdate = enabled;
	}
	entity->flags1.noHit = enabled;
	entity->chrModules->chrData->flags.noDamage = enabled;
	entity->chrModules->chrData->flags.noDead = enabled;
}

void GameStateManager::SetOptionValueToRestore(OptionType type, std::optional<uint8_t> value) {
	switch (type) {
		case OptionType::HUD:			hud.valueToRestore = value; break;
		case OptionType::AA:			aa.valueToRestore = value; break;
		case OptionType::MotionBlur:	mb.valueToRestore = value; break;
	}
}

void GameStateManager::DisableOption(OptionType optionType, bool enabled) {
	GameData::OptionData* optionData = GameDataManager::GetOptionData();
	GameData::Window* window = GameDataManager::Window.Get();

	switch (optionType) {
		case OptionType::HUD:			if (optionData) hud.Disable(&optionData->HUD, enabled); break;
		case OptionType::AA:			if (window) aa.Disable(&window->AA, enabled); break;
		case OptionType::MotionBlur:	if (window) mb.Disable(&window->motionBlur, enabled); break;
	}
}

bool GameStateManager::RestoreOptions() {
	if (wereRestored) return wereRestored;

	if (GameData::OptionData* optionData = GameDataManager::GetOptionData()) {
		hud.Restore(&optionData->HUD);
	}
	if (GameData::Window* window = GameDataManager::Window.Get()) {
		aa.Restore(&window->AA);
		mb.Restore(&window->motionBlur);
	}

	wereRestored = true;
	return wereRestored;
}

void GameStateManager::Option::Disable(uint8_t* gameOption, bool enabled) {
	if (!gameOption) return;

	if (enabled) {
		savedValue = *gameOption;
		*gameOption = 0;
		isDisabled = true;

		SettingsBackup::SaveOptionValue(type, savedValue);
	}
	else {
		if (isDisabled) {
			*gameOption = savedValue;
			isDisabled = false;
		}
	}
}

void GameStateManager::Option::Restore(uint8_t* gameOption) {
	if (!gameOption || !valueToRestore.has_value()) return;
	if (*gameOption == valueToRestore.value()) return;

	static constexpr const size_t optionCount = static_cast<size_t>(OptionType::Count);
	static constexpr const char* optionNames[optionCount] = {
		"Freecam", "HUD", "Anti-aliasing", "Motion Blur"
	};

	static constexpr const char* valueNames[][optionCount] = {
		{ "OFF", "ON", "", "" },				// Freecam
		{ "OFF", "ON", "AUTO", "" },			// HUD
		{ "OFF", "LOW", "", "HIGH" },			// AA
		{ "OFF", "LOW", "MEDIUM", "HIGH"}		// MotionBlur
	};

	const size_t t = static_cast<size_t>(type);
	uint8_t oldValue = *gameOption;
	uint8_t newValue = valueToRestore.value();
	LOG_INFO("Restored \"%s\" option from \"%s\" to \"%s\"", 
		optionNames[t], 
		((t < std::size(valueNames) && oldValue < optionCount)	? valueNames[t][oldValue] : "?"),
		((t < std::size(valueNames) && newValue < optionCount)	? valueNames[t][newValue] : "?")
	);

	*gameOption = newValue;
	valueToRestore = std::nullopt;
}