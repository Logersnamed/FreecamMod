#pragma once
#include "core/timeline/track.h"

class FreeCamera;

class Timeline {
    float time = 0;
    float max_time = 30;

    bool is_playing = false;

    Track<float>      fovTrack;
    Track<float3>     posTrack;
    Track<Quaternion> rotTrack;

    FreeCamera& freeCamera;

    float GetLastKeyframeTime();

public:
    Timeline(FreeCamera& freeCamera);

    void Update(float dt);

    void Play() { is_playing = true; }
    void StopPlay() { is_playing = false; }

	void AddAllKeyframes(float time);
	void SelectAllKeyframes();
	void DeleteSelectedKeyframes();

    void SetTime(float value) { time = std::max<float>(0.0f, value); }
    float GetTime() const { return time; }

    void SetMaxTime(float value) { max_time = value; }
    float GetMaxTime() const { return max_time; }
    
    bool IsPlaying() const { return is_playing; }

    Track<float>&      GetFovTrack() { return fovTrack; }
    Track<float3>&     GetPosTrack() { return posTrack; }
    Track<Quaternion>& GetRotTrack() { return rotTrack; }
};
