#pragma once
#include "core/features/game_state_manager.h"

class FrameStepper {
    GameStateManager& gameStateManager;

    int framesToStep = 0;

    bool wasGameFrozen = false;
    bool wasEntitiesFrozen = false;
    bool wasPlayerFrozen = false;

public:
    FrameStepper(GameStateManager& gameStateMgr) : gameStateManager(gameStateMgr) {}

    void StepFrames(int step) {
        if (step <= 0) return;

        if (framesToStep == 0) {
            wasGameFrozen = gameStateManager.IsGameFrozen();
            wasEntitiesFrozen = gameStateManager.AreEntitesFrozen();
            wasPlayerFrozen = gameStateManager.IsPlayerFrozen();

            if (wasGameFrozen)     gameStateManager.FreezeGame(false);
            if (wasEntitiesFrozen) gameStateManager.FreezeEntities(false);
            if (wasPlayerFrozen)   gameStateManager.FreezePlayer(false);
        }

        framesToStep += step + 1;   // https://cdn.7tv.app/emote/01JQK3VGYV3FBX6R7YGQPNG19J/4x.avif
    }

    void Update() {
        if (framesToStep <= 0) return;

        --framesToStep;

        if (framesToStep == 0) {
            if (wasGameFrozen)     gameStateManager.FreezeGame(true);
            if (wasEntitiesFrozen) gameStateManager.FreezeEntities(true);
            if (wasPlayerFrozen)   gameStateManager.FreezePlayer(true);

            wasGameFrozen = false;
            wasEntitiesFrozen = false;
            wasPlayerFrozen = false;
        }
    }
    
    void Reset() {
        framesToStep = 0;
    }
};