#pragma once
#include "core/config/con_var.h"

class GameStateManager;

class FrameStepper {
    GameStateManager& gameStateManager;

    int framesToStep = 0;

    bool wasGameFrozen = false;
    bool wasEntitiesFrozen = false;
    bool wasPlayerFrozen = false;

    ConVar<int> step{ "frame_stepper", "step", 1, 1 };

public:
    FrameStepper(GameStateManager& gameStateMgr) : gameStateManager(gameStateMgr) {}

    void StepFrames();
    void Update();
    void Reset() { framesToStep = 0; }

    int GetStep() const { return step; }
    int GetFramesToStep() const { return framesToStep; }
    void SetStepFromUI(int value) { step.SetValueFromUI(value); }
};