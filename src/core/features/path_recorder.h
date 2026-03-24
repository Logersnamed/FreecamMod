#pragma once
#include <vector>

#include "core/game_data/game_data.h"
#include "utils/types.h"
#include "utils/debug.h"

class PathRecorder {
    struct Data {
        int frame;
        matrix4x4 cameraMatrix;
        float fov;
    };

    int framesRecorded = 0;
    int framesPlayed = 0;

    bool isRecording = false;
    bool isPlaying = false;

    std::vector<Data> data;
    int dataRestored = 0;

public:
    void Record() { isRecording ? EndRecord() : StartRecord(); }
    void PlayRecord() { isPlaying ? EndPlay() : StartPlay(); }

    void StartRecord() {
        if (isPlaying) return;
        Clear();
        isRecording = true;
        Logger::Info("Recording started", isRecording);
    }

    void EndRecord() {
        isRecording = false;
        Logger::Info("Recording ended");
    }

    void StartPlay() {
        if (isRecording) {
            EndRecord();
        }

        framesPlayed = 0;
        isPlaying = true;
    }

    void EndPlay() {
        isPlaying = false;
        framesPlayed = 0;
        dataRestored = 0;
    }

    bool IsRecording() const { return isRecording; }
    bool IsPlaying() const { return isPlaying; }

    void RecordFrame(const GameData::Camera* camera) {
        if (!data.size() || data.back().cameraMatrix != camera->matrix) {
            data.push_back(Data{
                framesRecorded,
                camera->matrix,
                camera->fov
                });
        }
        ++framesRecorded;
    }

    void PlayNextFrame(GameData::Camera* camera) {
        Data& frameData = data[dataRestored];
        camera->matrix = frameData.cameraMatrix;
        camera->fov = frameData.fov;

        if (framesPlayed == data[dataRestored].frame) {
            ++dataRestored;
        }

        if (++framesPlayed == framesRecorded || dataRestored >= data.size()) {
            EndPlay();
        }
    }

    void Clear() {
        data.clear();
        framesPlayed = 0;
        framesRecorded = 0;
        dataRestored = 0;
    }
};