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

	GameData::WorldChrMan* world = GameDataManager::WorldChrMan.Get();
	if (!world) return;

	GameData::Players* players = world->players;
	if (!players) return;

	if (!players->IsPlayerAlone()) {
		LOG_INFO("Freezing entities in online is disabled");
		return;
	}

	areEntitiesFrozen = enabled;

	GameData::ChrIns* player = players->player0;
	const size_t length = world->GetEntityListLength();

	LOG_INFO("Set FreezeEntity = %d to %zu entities", enabled, length);

	for (size_t i = 0; i < length; ++i) {
		GameData::ChrIns* entity = world->begin[i];
		if (entity && entity != player) {
			FreezeEntity(entity, enabled);
		}
	}
}

void GameStateManager::FreezePlayer(bool enabled) {
	if (isPlayerFrozen == enabled) return;
	isPlayerFrozen = enabled;

	if (GameData::ChrIns* player = GameDataManager::GetPlayer()) {
		FreezeEntity(player, enabled);
	}
}

void GameStateManager::FreezeEntity(GameData::ChrIns* entity, bool enabled) {
	if (!entity) return;

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
	if (Option* option = GetOption(type)) {
		option->valueToRestore = value;
	}
}

void GameStateManager::DisableOption(OptionType type, bool enabled) {
	GameData::OptionData* optionData = GameDataManager::GetOptionData();
	GameData::Window* window = GameDataManager::Window.Get();

	switch (type) {
		case OptionType::HUD:			if (optionData) hud.Disable(&optionData->HUD, enabled); break;
		case OptionType::AA:			if (window) aa.Disable(&window->AA, enabled); break;
		case OptionType::MotionBlur:	if (window) mb.Disable(&window->motionBlur, enabled); break;
	}
}

bool GameStateManager::RestoreOptions() {
	if (wereRestored) return true;

	if (GameData::OptionData* optionData = GameDataManager::GetOptionData()) {
		hud.Restore(&optionData->HUD);
	}
	if (GameData::Window* window = GameDataManager::Window.Get()) {
		aa.Restore(&window->AA);
		mb.Restore(&window->motionBlur);
	}

	wereRestored = true;
	return true;
}

void GameStateManager::Option::Disable(uint8_t* gameOption, bool enabled) {
	if (!gameOption || enabled == isDisabled) return;

	if (enabled) {
		savedValue = *gameOption;
		*gameOption = 0;
		SettingsBackup::SaveOptionValue(type, savedValue);
	}
	else {
		*gameOption = savedValue;
	}

	isDisabled = enabled;
}

void GameStateManager::Option::Restore(uint8_t* gameOption) {
	if (!gameOption || !valueToRestore) return;

	uint8_t newValue = *valueToRestore;
	if (*gameOption == newValue) return;

	static constexpr const char* optionNames[] = {
		"Freecam", "HUD", "Anti-aliasing", "Motion Blur"
	};

	static constexpr const char* valueNames[][4] = {
		{ "OFF", "ON", "", "" },
		{ "OFF", "ON", "AUTO", "" },
		{ "OFF", "LOW", "", "HIGH" },
		{ "OFF", "LOW", "MEDIUM", "HIGH" }
	};

	const size_t t = static_cast<size_t>(type);
	uint8_t oldValue = *gameOption;

	const char* oldStr = (t < std::size(valueNames) && oldValue < 4) ? valueNames[t][oldValue] : "?";
	const char* newStr = (t < std::size(valueNames) && newValue < 4) ? valueNames[t][newValue] : "?";

	LOG_INFO("Restored \"%s\" option from \"%s\" to \"%s\"", optionNames[t], oldStr, newStr);

	*gameOption = newValue;
	valueToRestore.reset();
}