#pragma once
#include <vector>

#include "core/game_data/game_data.h"

class PathRecorder {
    template<typename T>
    struct FrameDataBuffer {
        static constexpr size_t MAX_FRAMES = 100000;

        struct FrameData {
            int frame = 0;
            T data;
        };

        int framesRestored = 0;
        std::vector<FrameData> framesData;
        bool isFullyRestored = false;

        void Record(int frame, T data) {
            if (framesData.empty() || framesData.back().data != data) {
                if (framesData.size() >= MAX_FRAMES) return;
                framesData.push_back(FrameData{ frame, data });
            }
        }

        T& GetNextFrameData(int frame) {
            int size = framesData.size();
            if (framesRestored >= size) {
                isFullyRestored = true;
                return framesData[size - 1].data;
            }

            FrameData& frameData = framesData[framesRestored];
            if (frame == frameData.frame) {
                ++framesRestored;
            }

            return frameData.data;
        }

        void Restart() {
            framesRestored = 0;
            isFullyRestored = false;
        }

        void Clear() {
            Restart();
            framesData.clear();
        }
    };

    int framesRecorded = 0;
    int framesPlayed = 0;

    FrameDataBuffer<float3> positions;
    FrameDataBuffer<Quaternion> rotations;
    FrameDataBuffer<float> fovs;

    bool isRecording = false;
    bool isPlaying = false;

public:
    void Record();
    void PlayRecord();
    void StartRecord();
    void EndRecord();
    void StartPlay();
    void EndPlay();

    bool IsRecording() const { return isRecording; }
    bool IsPlaying() const { return isPlaying; }

    int GetFramesRecorded() const { return framesRecorded; }
    int GetFramesPlayed() const { return framesPlayed; }

    void RecordFrame(const GameData::Camera* camera);
    void PlayNextFrame(GameData::Camera* camera);
    void Clear();
};