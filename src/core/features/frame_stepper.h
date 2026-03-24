#pragma once
#include "core/features/freeze_controller.h"

class FrameStepper {
    FreezeController& freezeController;

    int framesToStep = 0;

    bool wasGameFrozen = false;
    bool wasEntitiesFrozen = false;
    bool wasPlayerFrozen = false;

public:
    FrameStepper(FreezeController& controller) : freezeController(controller) {}

    void StepFrames(int step) {
        if (step <= 0) return;

        if (framesToStep == 0) {
            wasGameFrozen = freezeController.IsGameFrozen();
            wasEntitiesFrozen = freezeController.AreEntitesFrozen();
            wasPlayerFrozen = freezeController.IsPlayerFrozen();

            if (wasGameFrozen)     freezeController.FreezeGame(false);
            if (wasEntitiesFrozen) freezeController.FreezeEntities(false);
            if (wasPlayerFrozen)   freezeController.FreezePlayer(false);
        }

        framesToStep += step + 1;   // https://cdn.7tv.app/emote/01JQK3VGYV3FBX6R7YGQPNG19J/4x.avif
    }

    void Update() {
        if (framesToStep <= 0) return;

        --framesToStep;

        if (framesToStep == 0) {
            if (wasGameFrozen)     freezeController.FreezeGame(true);
            if (wasEntitiesFrozen) freezeController.FreezeEntities(true);
            if (wasPlayerFrozen)   freezeController.FreezePlayer(true);

            wasGameFrozen = false;
            wasEntitiesFrozen = false;
            wasPlayerFrozen = false;
        }
    }
    
    void Reset() {
        framesToStep = 0;
    }
};