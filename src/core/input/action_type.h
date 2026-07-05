#pragma once
#include <cstdint>

#define ACTION_TYPES \
    X(Toggle) \
    X(ToggleMenu) \
    X(ReloadConfig) \
    X(ResetSettings) \
    X(ToggleFreeze) \
    X(TeleportToCamera) \
    X(CycleWeatherTime) \
    X(ExitMod) \
    X(StartEndRecording) \
    X(StartEndPlayingRecording) \
    X(StepFrames) \
    X(MoveForward) \
    X(MoveBackward) \
    X(MoveLeft) \
    X(MoveRight) \
    X(MoveUp) \
    X(MoveDown) \
    X(Sprint) \
    X(ZoomIn) \
    X(ZoomOut) \
    X(TiltLeft) \
    X(TiltRight) \
    X(ScrollZoomModifier) \
    X(ScrollCameraSpeedModifier) \
    X(ScrollSpeedhackModifier) \
    X(ToggleSpeedhack) \
    X(ResetSpeedhackSpeed) \
    X(TimelinePlayPause) \
    X(TimelineAddAllKeyframes) \
    X(TimelineDeleteSelectedKeyframes) \
    X(TimelineSelectAllKeyframes)

enum class ActionType : int8_t {
#define X(name) name,
    ACTION_TYPES
#undef X
    Count
};