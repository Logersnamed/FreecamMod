#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include "directxmath.h"
#include "d3dcompiler.h"
#pragma comment(lib, "d3dcompiler.lib")

#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <cmath>
#include <functional>

#include "core/free_camera.h"
#include "gui/interpolation.h"
#include "utils/types.h"
#include "utils/time.h"

struct InstanceData {
    DirectX::FXMMATRIX model;
    float fovScale;
};

static inline DirectX::XMMATRIX ToXMMATRIX(const matrix3x3& m) {
    return DirectX::XMMATRIX(
        m.c0.x, m.c0.y, m.c0.z, 0.0f,
        m.c1.x, m.c1.y, m.c1.z, 0.0f,
        m.c2.x, m.c2.y, m.c2.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
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

template<typename T>
struct Keyframe {
    T data;
    float time;

    bool is_selected = false;
};

template<typename T>
class Track {
    std::vector<Keyframe<T>> keyframes;
    
public:
    std::vector<Keyframe<T>>& GetKeyframes() { return keyframes; }
    
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

    Keyframe<T>& GetKeyframe(int i) {
        i = std::clamp<int>(i, 0, keyframes.size() - 1);
        return keyframes[i];
    }

    T Evaluate(float time, T data) {
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

        return CatmullRomInterpolation<T>::Evaluate(prev.data, p0.data, p1.data, next.data, t);
    }
};

template<typename T>
struct TrackEditor;

template<>
struct TrackEditor<float> {
    static bool DrawValue(const std::string& name, float& value) {
        return ImGui::InputFloat(name.c_str(), &value);
    }
};

template<>
struct TrackEditor<float3> {
    static bool DrawValue(const std::string& name, float3& value) {
        float v[3] = { value.x, value.y, value.z };

        if (!ImGui::InputFloat3(name.c_str(), v))
            return false;

        value = { v[0], v[1], v[2] };
        return true;
    }
};

template<>
struct TrackEditor<Quaternion> {
    static bool DrawValue(const std::string& name, Quaternion& value) {
        ImGui::Text(name.c_str());
        return false;
    }
};

template<typename T>
class TrackWidget {
    Track<T> track;
    T data{};

    std::string name{};

    std::function<T()> getBound;
    std::function<void(const T&)> setBound;

public:
    TrackWidget(std::string name) : name(name) {}

    void Bind(std::function<T()> getter, std::function<void(const T&)> setter) {
        getBound = std::move(getter);
        setBound = std::move(setter);
    }

    void Unbind() {
        getBound = nullptr;
        setBound = nullptr;
    }

    std::vector<Keyframe<T>>& GetKeyframes() { 
        return track.GetKeyframes(); 
    }

    bool is_dragging_keyframes = false;
    float delta_x = 0;

    void DrawKeyframes(std::vector<Keyframe<T>>& keyframes) {
        // Drag
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            delta_x = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
        }
        else {
            // Apply drag
            if (delta_x) {
                for (auto& k : keyframes) {
                    if (!k.is_selected)  continue;

                    k.time += PixelsToTime(delta_x);
                    k.time = std::max<float>(k.time, 0.0f);
                }

                track.SortKeyframes();

                delta_x = 0;
            }
        }

        // Delete 
        if (ImGui::IsKeyPressed(ImGuiKey_X)) {
            for (int i = 0; i < keyframes.size();) {
                if (keyframes[i].is_selected) {
                    keyframes.erase(keyframes.begin() + i);
                }
                else {
                    ++i;
                }
            }
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        const int radius = 5;
        for (auto& k : keyframes) {
            // Select
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                ImVec2 click_pos = ImGui::GetMousePos();
                ImVec2 pos = ImGui::GetCursorScreenPos();

                ImVec2 keyframe_center_pos = ImVec2(pos.x + TimeToPixels(k.time), pos.y + (track_height - radius) * 0.5f);

                bool is_keyframe_clicked = abs(keyframe_center_pos.x - click_pos.x) <= radius
                    && abs(keyframe_center_pos.y - click_pos.y) <= radius;

                if (is_keyframe_clicked) k.is_selected = true;
                else {
                    if (k.is_selected && !ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
                        k.is_selected = false;
                    }
                }
            }

            float drag_delta = k.is_selected ? delta_x : 0;

            // Draw
            draw_list->AddCircle(
                ImVec2(pos.x + TimeToPixels(k.time) + drag_delta, pos.y + (track_height - radius) * 0.5f),
                radius, 
                k.is_selected ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 255, 255, 255)
                , 4, 2
            );
        }
    }

    void Render(float time, bool driveExternal) {
        data = track.Evaluate(time, data);

        ImGui::PushID(this);
        ImGui::BeginChild("##track", ImVec2(sidebar_width, track_height));

        if (ImGui::Button("+") || ImGui::IsKeyPressed(ImGuiKey_O)) {
            T captured = (getBound && !driveExternal) ? getBound() : data;
            track.AddKeyframe(Keyframe<T>{captured, time});
        }

        ImGui::SameLine();

        if (TrackEditor<T>::DrawValue(name, data)) {
            track.AddKeyframe(Keyframe<T>{data, time});
            if (setBound) setBound(data);
        }

        if (getBound && setBound) {
            if (driveExternal) setBound(data);
            else               data = getBound();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        DrawKeyframes(track.GetKeyframes());

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

    TrackWidget<float> fovTrack{"Fov"};
    TrackWidget<float3> posTrack {"Position"};
    TrackWidget<Quaternion> rotTrack {"Rotation"};

    FreeCamera& freeCamera;

public:
    static inline Timeline* instance;

    Timeline(FreeCamera& freeCamera) : freeCamera(freeCamera) {
        instance = this;

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
    }

    void Update(float dt) {
        if (isPlaybackPlaying) {
            time += dt;
        }
    }

    void Render() {
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
        draw_list->AddText(ImVec2(pos.x + TimeToPixels(time) + 12, pos.y + 20), IM_COL32(255, 255, 255, 255), TimeToString(time, TimeFormat::MINUTES_SECONDS_MILLISECONDS).c_str());

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

    std::vector<InstanceData> GetInstanceData() {
        const auto& pos_keyframes = posTrack.GetKeyframes();
        const auto& rot_keyframes = rotTrack.GetKeyframes();
        auto fov_keyframes = fovTrack.GetKeyframes();   // improve

        std::vector<InstanceData> result;
        if (pos_keyframes.empty() || rot_keyframes.empty()) return result;

        if (fov_keyframes.empty()) {
            fov_keyframes.push_back(Keyframe<float>{ DirectX::XM_PIDIV2, 0 });
        }

        int pos_i = 0;
        int rot_i = 0;
        int fov_i = 0;

        while (true) {
            auto& pos = pos_keyframes[pos_i];
            auto& rot = rot_keyframes[rot_i];
            auto& fov = fov_keyframes[fov_i];

            matrix3x3 rot_mat = rot.data.toRotationMatrix();
            DirectX::XMMATRIX rotation = ToXMMATRIX(rot_mat);
            DirectX::XMMATRIX model = DirectX::XMMatrixScaling(1, 1, 1) *
                rotation * DirectX::XMMatrixTranslation(pos.data.x, pos.data.y, pos.data.z);
            result.push_back(InstanceData{ model, fov.data });

            bool is_any_inc = false;
            if (pos.time <= rot.time && pos.time <= fov.time) {
                if (pos_keyframes.size() - 1 > pos_i) {
                    ++pos_i;
                    is_any_inc = true;
                }
            }

            if (rot.time <= pos.time && rot.time <= fov.time) {
                if (rot_keyframes.size() - 1 > rot_i) {
                    ++rot_i;
                    is_any_inc = true;
                }
            }

            if (fov.time <= pos.time && fov.time <= rot.time) {
                if (fov_keyframes.size() - 1 > fov_i) {
                    ++fov_i;
                    is_any_inc = true;
                }
            }

            if (!is_any_inc) break;
        }

        return result;
    }
};
