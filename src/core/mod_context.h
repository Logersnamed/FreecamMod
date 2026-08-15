#pragma once

class FreeCamera;
class Config;
class Input;
class ActionManager;
class HookManager;
class Speedhack;
class Timeline;

struct ModContext {
    FreeCamera& freeCamera;
    Config& cfg;
    Input& input;
    ActionManager& actionMgr;
    HookManager& hookManager;
    Speedhack& speedhack;
    Timeline& timeline;
};