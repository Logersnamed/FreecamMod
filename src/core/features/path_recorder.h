#pragma once
#include <vector>

#include "core/game_data/game_data.h"
#include "utils/types.h"
#include "utils/debug.h"

// todo: use interpolation to not record every frame
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
    void Record() { isRecording ? EndRecord() : StartRecord(); }
    void PlayRecord() { isPlaying ? EndPlay() : StartPlay(); }

    void StartRecord() {
        if (isPlaying) return;
        LOG_INFO("Recording started");
        Clear();
        isRecording = true;
    }

    void EndRecord() {
        LOG_INFO("Recording Ended:");
        LOG_INFO("\tTotal frames recorded: %d", framesRecorded);
        LOG_INFO("\tPositions recorded: %zu frames = %zu bytes",
            positions.framesData.size(), positions.framesData.size() * sizeof(float3)
        );
        LOG_INFO("\tRotations recorded: %zu frames = %zu bytes",
            rotations.framesData.size(), rotations.framesData.size() * sizeof(Quaternion)
        );
        LOG_INFO("\tFOVs recorded: %zu frames = %zu bytes",
            fovs.framesData.size(), fovs.framesData.size() * sizeof(float)
        );

        isRecording = false;
    }

    void StartPlay() {
        LOG_INFO("Recording play started");
        if (isRecording) {
            EndRecord();
        }

        framesPlayed = 0;
        isPlaying = true;
    }

    void EndPlay() {
        LOG_INFO("Recording play ended");
        isPlaying = false;
        framesPlayed = 0;

        positions.Restart();
        rotations.Restart();
        fovs.Restart();
    }

    bool IsRecording() const { return isRecording; }
    bool IsPlaying() const { return isPlaying; }

    void RecordFrame(const GameData::Camera* camera) {
        positions.Record(framesRecorded, camera->matrix.position());
        rotations.Record(framesRecorded, Quaternion::fromRotationMatrix(camera->matrix.rotation()));
        fovs.Record(framesRecorded, camera->fov);

        ++framesRecorded;
    }

    void PlayNextFrame(GameData::Camera* camera) {
        camera->matrix.position() = positions.GetNextFrameData(framesPlayed);
        camera->matrix.rotation() = rotations.GetNextFrameData(framesPlayed).toRotationMatrix();
        camera->fov = fovs.GetNextFrameData(framesPlayed);

        ++framesPlayed;

        if (positions.isFullyRestored && rotations.isFullyRestored && fovs.isFullyRestored) {
            EndPlay();
        }
    }

    void Clear() {
        positions.Clear();
        rotations.Clear();
        fovs.Clear();

        framesPlayed = 0;
        framesRecorded = 0;
    }
};