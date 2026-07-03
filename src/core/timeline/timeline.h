#pragma once
#include <algorithm>
#include <cmath>

#include "core/free_camera.h"
#include "core/events.h"
#include "core/timeline/track.h"
#include "utils/types.h"

class Timeline {
    float time = 0;
    float max_time = 30;

    bool is_playing = false;

    Track<float>      fovTrack;
    Track<float3>     posTrack;
    Track<Quaternion> rotTrack;

    FreeCamera& freeCamera;

public:
    Timeline(FreeCamera& freeCamera) : freeCamera(freeCamera) {
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

    void Update(float dt) {
        if (is_playing) {
            time += dt;

            if (time >= max_time) {
                time = max_time;
                is_playing = false;
            }
        }

        fovTrack.Update(time, is_playing);
        posTrack.Update(time, is_playing);
        rotTrack.Update(time, is_playing);
    }

    void Play() { is_playing = true; }
    void StopPlay() { is_playing = false; }

    void SetTime(float value) { time = std::max<float>(0.0f, value); }
    float GetTime() const { return time; }

    void SetMaxTime(float value) { max_time = value; }
    float GetMaxTime() const { return max_time; }
    
    bool IsPlaying() const { return is_playing; }

    Track<float>&      GetFovTrack() { return fovTrack; }
    Track<float3>&     GetPosTrack() { return posTrack; }
    Track<Quaternion>& GetRotTrack() { return rotTrack; }
};
