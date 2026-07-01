#pragma once
#include "core/timeline/keyframe.h"
#include "core/timeline/interpolation.h"

#include <vector>
#include <algorithm>
#include <functional>

template<typename T>
class Track {
    std::vector<Keyframe<T>> keyframes;
    T data{};

    std::function<T()> read_camera;
    std::function<void(const T&)> write_camera;

    InterpolationType interpolation_type = InterpolationType::CatmullRom;

public:
    std::vector<Keyframe<T>>& GetKeyframes() { return keyframes; }

    T GetData() const { return data; }
    void SetData(const T& value) { data = value; }

    InterpolationType GetInterpolationType() const { return interpolation_type; }
    void SetInterpolationType(InterpolationType type) { interpolation_type = type; }

    void Bind(std::function<T()> getter, std::function<void(const T&)> setter) {
        read_camera = std::move(getter);
        write_camera = std::move(setter);
    }

    void Unbind() {
        read_camera = nullptr;
        write_camera = nullptr;
    }

    void WriteCameraValue(const T& value) {
        if (write_camera) write_camera(value);
    }

    std::optional<T> ReadCameraValue() {
        if (read_camera) return read_camera();
        return std::nullopt;
    }

    void Update(float time, bool is_playing) {
        data = Evaluate(time);

        if (is_playing) {
            WriteCameraValue(data);
        }
        else {
            data = ReadCameraValue().value_or(data);
        }
    }

    void SortKeyframes() {
        std::sort(keyframes.begin(), keyframes.end(),
            [](const Keyframe<T>& a, const Keyframe<T>& b) {
                return a.time < b.time;
            });
    }

    void AddKeyframe(const Keyframe<T>& keyframe) {
        for (auto& k : keyframes) {
            if (std::abs(k.time - keyframe.time) < 0.001f) {
                k.data = keyframe.data;
                return;
            }
        }
        int insertPos = 0;
        for (auto& k : keyframes) {
            if (k.time > keyframe.time) {
                keyframes.insert(keyframes.begin() + insertPos, keyframe);
                return;
            }
            ++insertPos;
        }
        keyframes.push_back(keyframe);
    }

    void AddKeyframe(float time) {
        AddKeyframe(Keyframe<T>{data, time});
    }

    Keyframe<T>& GetKeyframe(int i) {
        i = std::clamp<int>(i, 0, keyframes.size() - 1);
        return keyframes[i];
    }

    T Evaluate(float time) {
        if (keyframes.empty()) return data;
        if (keyframes.size() == 1) return keyframes[0].data;
        if (time <= keyframes[0].time) return keyframes[0].data;

        int i = 0;
        for (const auto& k : keyframes) {
            if (time < k.time) break;
            ++i;
        }
        if (i == 0 || (size_t)i >= keyframes.size()) return keyframes.back().data;

        const auto& prev = GetKeyframe(i - 2);
        const auto& p0 = GetKeyframe(i - 1);
        const auto& p1 = GetKeyframe(i);
        const auto& next = GetKeyframe(i + 1);
        float t = (time - p0.time) / (p1.time - p0.time);

        if (interpolation_type == InterpolationType::CatmullRom) {
            return CatmullRomInterpolation<T>::Evaluate(prev.data, p0.data, p1.data, next.data, t);
        }

        return LinearInterpolation<T>::Evaluate(p0.data, p1.data, t);
    }
};