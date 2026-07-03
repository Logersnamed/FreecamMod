#include "core/features/frame_stepper.h"

#include "core/features/game_state_manager.h"
#include "core/events.h"

void FrameStepper::StepFrames() {
    if (step <= 0) return;

    if (framesToStep == 0) {
        wasGameFrozen = gameStateManager.IsGameFrozen();
        wasEntitiesFrozen = gameStateManager.AreEntitesFrozen();
        wasPlayerFrozen = gameStateManager.IsPlayerFrozen();

        if (wasGameFrozen)     gameStateManager.FreezeGame(false);
        if (wasEntitiesFrozen) gameStateManager.FreezeEntities(false);
        if (wasPlayerFrozen)   gameStateManager.FreezePlayer(false);
    }

    framesToStep += step + (framesToStep == 0);

    EventBus::Emit(Event::FrameStepped{ .framesStepped = step });
}

void FrameStepper::Update() {
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