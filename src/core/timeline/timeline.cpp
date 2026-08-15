#include "core/timeline/timeline.h"
#include "core/free_camera.h"
#include "core/events.h"

Timeline::Timeline(FreeCamera& freeCamera) : freeCamera(freeCamera) {
    fovTrack.Bind(
        [&freeCamera](GameData::Camera* cam) {
            return cam ? cam->fov : 0.0f;
        },
        [&freeCamera](GameData::Camera* cam, const float& v) {
            if (cam) cam->fov = v;
        }
    );
    posTrack.Bind(
        [&freeCamera](GameData::Camera* cam) {
            return cam ? cam->matrix.position() : float3();
        },
        [&freeCamera](GameData::Camera* cam, const float3& v) {
            if (cam) cam->matrix.position() = v;
        }
    );
    rotTrack.Bind(
        [&freeCamera](GameData::Camera* cam) { return freeCamera.GetRotation().toQuaternion(); },
        [&freeCamera](GameData::Camera* cam, const Quaternion& v) { freeCamera.SetRotation(v.toEuler()); }
    );

    EventBus::Subscribe<Event::ToggleFreecam>([this](const Event::ToggleFreecam& event) {
        if (!event.isEnabled) {
            StopPlay();
        }
    });
}

void Timeline::Update(GameData::GameRend* gameRend, float dt) {
    auto* freeCamera = gameRend->csDebugCam;
    if (!freeCamera) return;

    if (is_playing) {
        time += dt;

        float last_time = GetLastKeyframeTime();
        if (last_time && last_time < time) {
            time = last_time;
            is_playing = false;
        }

        if (time >= max_time) {
            time = max_time;
            is_playing = false;
        }
    }

    fovTrack.Update(freeCamera, time, is_playing);
    posTrack.Update(freeCamera, time, is_playing);
    rotTrack.Update(freeCamera, time, is_playing);
}

void Timeline::AddAllKeyframes(float time) {
    fovTrack.AddKeyframe(time);
    posTrack.AddKeyframe(time);
    rotTrack.AddKeyframe(time);
}

void Timeline::SelectAllKeyframes() {
	for (auto& kf : fovTrack.GetKeyframes()) kf.is_selected = true;
	for (auto& kf : posTrack.GetKeyframes()) kf.is_selected = true;
	for (auto& kf : rotTrack.GetKeyframes()) kf.is_selected = true;
}

void Timeline::DeleteSelectedKeyframes() {
    fovTrack.DeleteSelectedKeyframes();
    posTrack.DeleteSelectedKeyframes();
    rotTrack.DeleteSelectedKeyframes();
}

float Timeline::GetLastKeyframeTime() {
    float t = 0;
    t = std::max(t, fovTrack.GetLastKeyframeTime());
    t = std::max(t, posTrack.GetLastKeyframeTime());
    t = std::max(t, rotTrack.GetLastKeyframeTime());
    return t;
}