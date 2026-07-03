#pragma once

class Config;
class Input;
class ActionManager;
class HookManager;
class Speedhack;
class FreeCamera;

struct ModContext {
    Config& config;
    Input& input;
    ActionManager& actionMgr;
    HookManager& hookManager;
    Speedhack& speedhack;
    FreeCamera& freeCamera;
};