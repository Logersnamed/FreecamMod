#include "core/features/path_recorder.h"

#include "core/events.h"
#include "utils/debug.h"

void PathRecorder::Record() {
    isRecording ? EndRecord() : StartRecord();
    EventBus::Emit(Event::Record{ .isEnabled = isRecording });
}

void PathRecorder::PlayRecord() {
    isPlaying ? EndPlay() : StartPlay();
}

void PathRecorder::StartRecord() {
    if (isPlaying) return;
    LOG_INFO("Recording started");
    Clear();
    isRecording = true;
}

void PathRecorder::EndRecord() {
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

void PathRecorder::StartPlay() {
    LOG_INFO("Recording play started");
    if (isRecording) {
        EndRecord();
    }

    if (!framesRecorded) {
        LOG_INFO("Replay is empty");
        return;
    }

    framesPlayed = 0;
    isPlaying = true;
    EventBus::Emit(Event::PlayRecord{ .isEnabled = true });
}

void PathRecorder::EndPlay() {
    LOG_INFO("Recording play ended");
    isPlaying = false;
    framesPlayed = 0;

    positions.Restart();
    rotations.Restart();
    fovs.Restart();
    EventBus::Emit(Event::PlayRecord{ .isEnabled = false });
}

void PathRecorder::RecordFrame(const GameData::Camera* camera) {
    positions.Record(framesRecorded, camera->matrix.position());
    rotations.Record(framesRecorded, Quaternion::fromRotationMatrix(camera->matrix.rotation()));
    fovs.Record(framesRecorded, camera->fov);

    ++framesRecorded;
}

void PathRecorder::PlayNextFrame(GameData::Camera* camera) {
    camera->matrix.position() = positions.GetNextFrameData(framesPlayed);
    camera->matrix.rotation() = rotations.GetNextFrameData(framesPlayed).toRotationMatrix();
    camera->fov = fovs.GetNextFrameData(framesPlayed);

    ++framesPlayed;

    if (positions.isFullyRestored && rotations.isFullyRestored && fovs.isFullyRestored) {
        EndPlay();
    }
}

void PathRecorder::Clear() {
    positions.Clear();
    rotations.Clear();
    fovs.Clear();

    framesPlayed = 0;
    framesRecorded = 0;
}