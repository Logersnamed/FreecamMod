#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <algorithm>
#include <cmath>

#include "core/game_data_manager.h"

#include <DirectXMath.h>

enum class Format {
    SECONDS,
    SECONDS_MILLISECONDS,
    MINUTES_SECONDS,
    MINUTES_SECONDS_MILLISECONDS,
};

static inline std::string TimeToString(float seconds, Format format = Format::MINUTES_SECONDS) {
    seconds = std::max<float>(0.0f, seconds);

    const int totalSeconds = static_cast<int>(seconds);
    const int minutes = totalSeconds / 60;
    const int secs = totalSeconds % 60;
    const int milliseconds = static_cast<int>((seconds - totalSeconds) * 1000.0f);

    switch (format) {
    case Format::SECONDS:
        return std::format("{}", totalSeconds);

    case Format::SECONDS_MILLISECONDS:
        return std::format("{}.{:03}", totalSeconds, milliseconds);

    case Format::MINUTES_SECONDS:
        return std::format("{:02}:{:02}", minutes, secs);

    case Format::MINUTES_SECONDS_MILLISECONDS:
        return std::format("{:02}:{:02}.{:03}", minutes, secs, milliseconds);
    }
    return {};
}

static inline Quaternion EulerToQuaternion(const EulerAngles& e)
{
    using namespace DirectX;

    XMVECTOR q = XMQuaternionRotationRollPitchYaw(
        e.pitch, 
        e.yaw,   
        e.roll   
    );

    Quaternion result;
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), q);
    return result;
}

static inline EulerAngles QuaternionToEuler(const Quaternion& q)
{
    using namespace DirectX;

    const float x = q.x;
    const float y = q.y;
    const float z = q.z;
    const float w = q.w;

    EulerAngles e;

    float sinp = 2.0f * (w * x - y * z);
    if (std::abs(sinp) >= 1.0f)
        e.pitch = std::copysign(XM_PIDIV2, sinp);
    else
        e.pitch = std::asin(sinp);

    float siny = 2.0f * (w * y + z * x);
    float cosy = 1.0f - 2.0f * (x * x + y * y);
    e.yaw = std::atan2(siny, cosy);

    float sinr = 2.0f * (w * z + x * y);
    float cosr = 1.0f - 2.0f * (z * z + x * x);
    e.roll = std::atan2(sinr, cosr);

    return e;
}

static inline const int sidebar_width = 300;
static inline const int track_height = 25;

static inline constexpr float ONE_SEC_IN_PIXELS = 100.0f;
static inline constexpr int TimeToPixels(float time) {
    return (int)(time * ONE_SEC_IN_PIXELS);
}

static inline constexpr float PixelsToTime(int pixels) {
    return pixels * (1.0f / ONE_SEC_IN_PIXELS);
}

static float CubicHermite(float A, float B, float C, float D, float t)
{
    float a = -A / 2.0f + (3.0f * B) / 2.0f - (3.0f * C) / 2.0f + D / 2.0f;
    float b = A - (5.0f * B) / 2.0f + 2.0f * C - D / 2.0f;
    float c = -A / 2.0f + C / 2.0f;
    float d = B;

    return a * t * t * t + b * t * t + c * t + d;
}

template<typename T>
struct Keyframe {
    T data;
    float time;
};

#include <functional>

template<typename T>
class Track {
    std::vector<Keyframe<T>> keyframes;
    T data{};

    std::function<T()> getBound;
    std::function<void(const T&)> setBound;

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

    T lerp(const T& a, const T& b, float t) {
        if constexpr (std::is_same_v<T, Quaternion>) {
            return Quaternion::slerp(a, b, t);
        }
        else {
            t = std::clamp(t, 0.0f, 1.0f);
            return a + (b - a) * t;
        }
    }

    T GetData(float time) {
        if (keyframes.empty()) return data;
        if (keyframes.size() == 1) return keyframes[0].data;
        if (time <= keyframes[0].time) return keyframes[0].data;

        int i = 0;
        for (const auto& k : keyframes) {
            if (time < k.time) break;
            ++i;
        }
        if (i == 0 || (size_t)i >= keyframes.size()) return keyframes.back().data;

        const auto& a = keyframes[i - 1];
        const auto& b = keyframes[i];
        float t = (time - a.time) / (b.time - a.time);


        if constexpr (std::is_same_v<T, float>) {
            int ai = std::clamp<int>(i - 2, 0, keyframes.size() - 1);
            int bi = std::clamp<int>(i - 1, 0, keyframes.size() - 1);
            int ci = std::clamp<int>(i, 0, keyframes.size() - 1);
            int di = std::clamp<int>(i + 1, 0, keyframes.size() - 1);

            return CubicHermite(keyframes[ai].data, keyframes[bi].data, keyframes[ci].data, keyframes[di].data, t);
        }
        else if constexpr (std::is_same_v<T, float3>) {
            int ai = std::clamp<int>(i - 2, 0, keyframes.size() - 1);
            int bi = std::clamp<int>(i - 1, 0, keyframes.size() - 1);
            int ci = std::clamp<int>(i,     0, keyframes.size() - 1);
            int di = std::clamp<int>(i + 1, 0, keyframes.size() - 1);

            return float3(
                CubicHermite(keyframes[ai].data.x, keyframes[bi].data.x, keyframes[ci].data.x, keyframes[di].data.x, t),
                CubicHermite(keyframes[ai].data.y, keyframes[bi].data.y, keyframes[ci].data.y, keyframes[di].data.y, t),
                CubicHermite(keyframes[ai].data.z, keyframes[bi].data.z, keyframes[ci].data.z, keyframes[di].data.z, t)
            );
        }
        else if constexpr (std::is_same_v<T, Quaternion>) {
            using namespace DirectX;
            int q0i = std::clamp<int>(i - 2, 0, keyframes.size() - 1);
            int q1i = std::clamp<int>(i - 1, 0, keyframes.size() - 1);
            int q2i = std::clamp<int>(i, 0, keyframes.size() - 1);
            int q3i = std::clamp<int>(i + 1, 0, keyframes.size() - 1);

            XMVECTOR q0 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&keyframes[q0i].data));
            XMVECTOR q1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&keyframes[q1i].data));
            XMVECTOR q2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&keyframes[q2i].data));
            XMVECTOR q3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&keyframes[q3i].data));

            XMVECTOR a, b, c;
            XMQuaternionSquadSetup(&a, &b, &c, q0, q1, q2, q3);

            XMVECTOR res = XMQuaternionSquad(q1, a, b, c, t);

            Quaternion result;
            XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), res);
            return result;
        }

        return lerp(a.data, b.data, t);
    }

public:
    void Bind(std::function<T()> getter, std::function<void(const T&)> setter) {
        getBound = std::move(getter);
        setBound = std::move(setter);
    }

    void Unbind() {
        getBound = nullptr;
        setBound = nullptr;
    }

    void Render(float time, bool driveExternal) {
        data = GetData(time);

        ImGui::PushID(this);
        ImGui::BeginChild("##track", ImVec2(sidebar_width, track_height));

        if (ImGui::Button("+")) {
            T captured = (getBound && !driveExternal) ? getBound() : data;
            AddKeyframe(Keyframe<T>{captured, time});
        }
        ImGui::SameLine();

        bool edited = false;

        if constexpr (std::is_same_v<T, float>) {
            float d = data;
            if (ImGui::InputFloat("Data", &d)) {
                data = d;
                edited = true;
            }
        }
        else if constexpr (std::is_same_v<T, float3>) {
            float v[3] = { data.x, data.y, data.z };
            if (ImGui::InputFloat3("Data", v)) {
                data = T{ v[0], v[1], v[2] };
                edited = true;
            }
        }
        else if constexpr (std::is_same_v<T, Quaternion>) {

        }
        else {
            static_assert(sizeof(T) == 0, "Track<T>::Render: no InputField implemented for this T");
        }

        if (edited && !keyframes.empty()) {
            AddKeyframe(Keyframe<T>{data, time});
        }

        if (getBound && setBound) {
            if (driveExternal) setBound(data);
            else                data = getBound();
        }

        ImGui::EndChild();
        ImGui::SameLine();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        const int radius = 5;
        for (auto& k : keyframes) {
            draw_list->AddCircle(ImVec2(pos.x + TimeToPixels(k.time), pos.y + (track_height - radius) * 0.5f), radius, IM_COL32(255, 255, 255, 255), 4, 2);
        }

        ImGui::PopID();
    }
};

class Timeline {
    bool isPlaybackPlaying = false;
    float time = 0;
    bool was_clicked_in_timestamps_zone = false;

    void SetTime(float value) {
        time = std::max<float>(0.0f, value);
    }

    Track<float> fovTrack;
    Track<float3> posTrack;
    Track<Quaternion> rotTrack;

    FreeCamera& freeCamera;

public:
    Timeline(FreeCamera& freeCamera) : freeCamera(freeCamera) {}

    void Update(float dt) {
        if (isPlaybackPlaying) {
            time += dt;
        }
    }

    void Render(float dt) {
        Update(dt);
        GameData::Camera* cam = nullptr;
        if (auto* g = freeCamera.GetGameRend()) {
            cam = g->csDebugCam;
        }

        if (cam) {
            fovTrack.Bind(
                [cam]() { return cam->fov; },
                [cam](const float& v) { cam->fov = v; }
            );
            posTrack.Bind(
                [cam]() { return cam->matrix.position(); },
                [cam](const float3& v) { cam->matrix.position() = v; }
            );
            rotTrack.Bind(
                [this]() { return EulerToQuaternion(freeCamera.rotation); },
                [this](const Quaternion& v) { freeCamera.rotation = QuaternionToEuler(v); }
            );
        }
        else {
            fovTrack.Unbind();
            posTrack.Unbind();
            rotTrack.Unbind();
        }

        

        ImGui::Begin("Timeline");
        ImGui::BeginChild("##buttons", ImVec2(sidebar_width, track_height));
        if (ImGui::Button(isPlaybackPlaying ? "Stop" : "Start", ImVec2(ImGui::GetContentRegionAvail()))) {
            isPlaybackPlaying = !isPlaybackPlaying;
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();

        // Drawing border
        draw_list->AddLine(ImVec2(pos.x, pos.y), ImVec2(pos.x, pos.y + 1000), IM_COL32(255, 255, 255, 255), 1);

        // Drawing timestamps
        for (int i = 0; i < 2000; i += 10) {
            draw_list->AddLine(ImVec2(pos.x + i, pos.y), ImVec2(pos.x + i, pos.y + track_height * 0.3), IM_COL32(155, 155, 155, 255), 1);
            if (i % (int)ONE_SEC_IN_PIXELS == 0) {
                draw_list->AddLine(ImVec2(pos.x + i, pos.y), ImVec2(pos.x + i, pos.y + track_height * 0.5), IM_COL32(255, 255, 255, 255), 1);
                draw_list->AddText(ImVec2(pos.x + i + 10, pos.y + track_height * 0.3 + 2), IM_COL32(255, 255, 255, 255), TimeToString(i * 0.01f).c_str());
            }
        }

        // Drawing playhead
        const int tringle_radius = 8;
        draw_list->AddLine(ImVec2(pos.x + TimeToPixels(time), pos.y), ImVec2(pos.x + TimeToPixels(time), pos.y + 1000), IM_COL32(255, 255, 255, 255), 1);
        draw_list->AddTriangleFilled(
            ImVec2(pos.x + TimeToPixels(time) - tringle_radius, pos.y),
            ImVec2(pos.x + TimeToPixels(time) + tringle_radius, pos.y),
            ImVec2(pos.x + TimeToPixels(time), pos.y + tringle_radius),
            IM_COL32(255, 255, 255, 255)
        );
        draw_list->AddText(ImVec2(pos.x + TimeToPixels(time) + 12, pos.y + 20), IM_COL32(255, 255, 255, 255), TimeToString(time, Format::MINUTES_SECONDS_MILLISECONDS).c_str());

        bool is_dragging = false;
        bool s = was_clicked_in_timestamps_zone && ImGui::IsMouseDown(ImGuiMouseButton_Left);
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || s) {
            ImVec2 click_pos = ImGui::GetMousePos();

            bool is_in_timestamps_zone = click_pos.x > pos.x && click_pos.y > pos.y && click_pos.y < pos.y + track_height;
            if (is_in_timestamps_zone || s) {
                is_dragging = true;
                SetTime(PixelsToTime((int)(click_pos.x - pos.x)));
                was_clicked_in_timestamps_zone = true;
                isPlaybackPlaying = false;
            }
        }
        else {
            was_clicked_in_timestamps_zone = false;
        }


        ImGui::NewLine();
        fovTrack.Render(time, isPlaybackPlaying || is_dragging);

        ImGui::NewLine();
        posTrack.Render(time, isPlaybackPlaying || is_dragging);

        ImGui::NewLine();
        rotTrack.Render(time, isPlaybackPlaying || is_dragging);

        ImGui::End();
    }
};
