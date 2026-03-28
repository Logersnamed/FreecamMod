#pragma once
#include "core/game_data/game_data.h"
#include "core/game_data_manager.h"
#include "utils/debug.h"

class GameStateManager {
public:
	void FreezeGame(bool enabled) {
		if (isGameFrozen != enabled) {
			GameDataManager::PauseGame(enabled);
			isGameFrozen = enabled;
		}
	}

	void FreezeEntities(bool enabled) {
		if (areEntitiesFrozen == enabled) return;
		areEntitiesFrozen = enabled;

		GameData::WorldChrMan* world = GameDataManager::GetWorldChrMan();
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

	void FreezePlayer(bool enabled) {
		if (isPlayerFrozen == enabled) return;
		isPlayerFrozen = enabled;

		GameData::ChrIns* player = GameDataManager::GetPlayer();
		if (player) FreezeEntity(player, enabled);
	}

	void FreezeEntity(GameData::ChrIns* entity, bool enabled) {
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

	bool IsGameFrozen() const { return isGameFrozen; }
	bool AreEntitesFrozen() const { return areEntitiesFrozen; }
	bool IsPlayerFrozen() const { return isPlayerFrozen; }

	void SetZeroSpeedFreeze(bool enabled) { isZeroSpeedFreeze = enabled; }

private:
	bool isGameFrozen = false;
	bool areEntitiesFrozen = false;
	bool isPlayerFrozen = false;

	bool isZeroSpeedFreeze = false;
};