#include "core/timeline/timeline.h"
#include "core/free_camera.h"
#include "core/events.h"

Timeline::Timeline(FreeCamera& freeCamera) : freeCamera(freeCamera) {
    fovTrack.Bind(
        [&freeCamera]() {
            auto* cam = freeCamera.GetCamera();
            return cam ? cam->fov : 0.0f;
        },
        [&freeCamera](const float& v) {
            auto* cam = freeCamera.GetCamera();
            if (cam) cam->fov = v;
        }
    );
    posTrack.Bind(
        [&freeCamera]() {
            auto* cam = freeCamera.GetCamera();
            return cam ? cam->matrix.position() : float3();
        },
        [&freeCamera](const float3& v) {
            auto* cam = freeCamera.GetCamera();
            if (cam) cam->matrix.position() = v;
        }
    );
    rotTrack.Bind(
        [&freeCamera]() { return freeCamera.GetRotation().toQuaternion(); },
        [&freeCamera](const Quaternion& v) { freeCamera.SetRotation(v.toEuler()); }
    );

    EventBus::Subscribe<Event::ToggleFreecam>([this](const Event::ToggleFreecam& event) {
        if (!event.isEnabled) {
            StopPlay();
        }
    });
}

void Timeline::Update(float dt) {
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

    fovTrack.Update(time, is_playing);
    posTrack.Update(time, is_playing);
    rotTrack.Update(time, is_playing);
}

float Timeline::GetLastKeyframeTime() {
    float t = 0;
    t = std::max(t, fovTrack.GetLastKeyframeTime());
    t = std::max(t, posTrack.GetLastKeyframeTime());
    t = std::max(t, rotTrack.GetLastKeyframeTime());
    return t;
}