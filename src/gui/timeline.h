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

/*struct InstanceData {
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
}*/

static inline const int sidebar_width = 300;
static inline const int track_height = 25;


static inline constexpr float ONE_SEC_IN_PIXELS = 100.0f;
static inline constexpr int TimeToPixels(float time) {
    return (int)(time * ONE_SEC_IN_PIXELS);
}

static inline constexpr float PixelsToTime(int pixels) {
    return pixels * (1.0f / ONE_SEC_IN_PIXELS);
}

static inline const int snap_pixels = 5;
static inline const bool is_snap_enabled = true;

static inline constexpr int SnapPixelsToGrid(int pixels) {
    if (!is_snap_enabled || ImGui::IsKeyDown(ImGuiKey_LeftAlt)) return pixels;
    return ((pixels + snap_pixels / 2) / snap_pixels) * snap_pixels;
}

static inline constexpr float SnapTimeToGrid(float time) {
    if (!is_snap_enabled || ImGui::IsKeyDown(ImGuiKey_LeftAlt)) return time;
    return PixelsToTime(SnapPixelsToGrid(TimeToPixels(time)));
}

static inline const int track_max_time = 100;
static inline const int track_width = track_max_time * ONE_SEC_IN_PIXELS;

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

struct TrackInputEvent {
    float drag_delta_x;
    bool drag_released;  
    bool delete_pressed;   
    bool mouse_clicked;   
    bool shift_held;   
    float2 click_pos;  
    float2 origin_pos; 
};

template<typename T>
struct KeyframesController {
    static void Update(Track<T>& track, const TrackInputEvent& events) {
        ProcessDrag(track, events);
        ProcessDelete(track, events);
        ProcessSelect(track, events);
    }

    static void ProcessDrag(Track<T>& track, const TrackInputEvent& events) {
        if (events.drag_released && events.drag_delta_x) {
            for (auto& k : track.GetKeyframes()) {
                if (!k.is_selected)  continue;

                k.time += PixelsToTime(events.drag_delta_x);
                k.time = std::max<float>(k.time, 0.0f);
                if (events.drag_delta_x) k.time = SnapTimeToGrid(k.time);   // snapp only after drag
            }

            track.SortKeyframes();
        }
    }

    static void ProcessDelete(Track<T>& track, const TrackInputEvent& events) {
        if (events.delete_pressed) {
            auto& keyframes = track.GetKeyframes();
            for (int i = 0; i < keyframes.size();) {
                if (keyframes[i].is_selected) {
                    keyframes.erase(keyframes.begin() + i);
                }
                else {
                    ++i;
                }
            }
        }
    }

    static void ProcessSelect(Track<T>& track, const TrackInputEvent& events) {
        if (events.mouse_clicked) {
            const int radius = 5;
            auto& keyframes = track.GetKeyframes();
            for (auto& k : keyframes) {
                float2 keyframe_center_pos = float2(
                    events.origin_pos.x + TimeToPixels(k.time), 
                    events.origin_pos.y + (track_height - radius) * 0.5f
                );

                bool is_keyframe_clicked = abs(keyframe_center_pos.x - events.click_pos.x) <= radius
                    && abs(keyframe_center_pos.y - events.click_pos.y) <= radius;

                if (is_keyframe_clicked) k.is_selected = true;
                else {
                    if (k.is_selected && !events.shift_held) {
                        k.is_selected = false;
                    }
                }
            }
        }
    }
};

template<typename T>
class TrackWidget {
    Track<T> track;
    T data{};

    std::string name{};

    std::function<T()> getBound;
    std::function<void(const T&)> setBound;

    float delta_x = 0;

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

    void DrawKeyframes(std::vector<Keyframe<T>>& keyframes, const TrackInputEvent& events) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        const int radius = 5;
        for (auto& k : keyframes) {
            float drag_delta = (k.is_selected && !events.drag_released) ? events.drag_delta_x : 0;

            float delta =  std::max<float>(0.0f, TimeToPixels(k.time) + drag_delta);
            if (drag_delta) delta = SnapPixelsToGrid(delta);    // snapp only after drag
            draw_list->AddCircle(
                ImVec2(pos.x + delta, pos.y + track_height * 0.5f),
                radius, 
                k.is_selected ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 255, 255, 255)
                , 4, 2
            );
        }
    }

    TrackInputEvent GetEvents(ImVec2 origin) {
        bool released = false;
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            delta_x = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
        }
        else if (delta_x) {
            released = true;
        }
        ImVec2 mp = ImGui::GetMousePos();

        TrackInputEvent events {
            delta_x,
            released,
            ImGui::IsKeyPressed(ImGuiKey_X),
            ImGui::IsMouseClicked(ImGuiMouseButton_Left),
            ImGui::IsKeyDown(ImGuiKey_LeftShift),
            {mp.x, mp.y},
            {origin.x, origin.y}
        };

        return events;
    }

    void Update(float time, bool driveExternal) {
        data = track.Evaluate(time, data);

        if (getBound && setBound) {
            if (driveExternal) setBound(data);
            else               data = getBound();
        }
    }

    void DrawSidebar(float time, bool driveExternal) {
        ImGui::PushID(this);
        //ImGui::BeginChild("##track", ImVec2(sidebar_width, track_height));

        if (ImGui::Button("+", ImVec2(track_height, track_height)) || ImGui::IsKeyPressed(ImGuiKey_O)) {
            T captured = (getBound && !driveExternal) ? getBound() : data;
            track.AddKeyframe(Keyframe<T>{captured, time});
        }

        ImGui::SameLine();

        if (TrackEditor<T>::DrawValue(name, data)) {
            track.AddKeyframe(Keyframe<T>{data, time});
            if (setBound) setBound(data);
        }

        //ImGui::EndChild();
        ImGui::PopID();
    }

    void DrawLane(float time, bool driveExternal) {
        ImGui::PushID(this);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));
        ImGui::BeginChild("##lane", ImVec2(track_width, track_height));
        ImVec2 origin = ImGui::GetCursorScreenPos();
        TrackInputEvent events = GetEvents(origin);

        KeyframesController<T>::Update(track, events);
        DrawKeyframes(track.GetKeyframes(), events);

        if (events.drag_released)
            delta_x = 0;

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopID();
    }
};

class Timeline {
    bool is_playing = false;
    float time = 0;
    bool was_clicked_in_timestamps_zone = false;

    void SetTime(float value) {
        time = value;
        //time = std::max<float>(0.0f, value);
    }

    TrackWidget<float> fovTrack{"Fov"};
    TrackWidget<float3> posTrack {"Position"};
    TrackWidget<Quaternion> rotTrack {"Rotation"};

    FreeCamera& freeCamera;

public:
    static inline Timeline* instance{};
    bool is_visible = true;

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
        if (is_playing) {
            time += dt;

            if (time >= track_max_time) {
                time = track_max_time;
                is_playing = false;
            }
        }

        fovTrack.Update(time, is_playing);
        posTrack.Update(time, is_playing);
        rotTrack.Update(time, is_playing);
    }

    void Render() {
        if (!is_visible) return;

        ImGui::SetNextWindowSizeConstraints(ImVec2(600, 160), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Timeline", &is_visible);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));

        ImGui::BeginChild("##sidebar", ImVec2(sidebar_width, track_height * 4), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();

            if (ImGui::Button(is_playing ? "Stop" : "Start", ImVec2(sidebar_width, track_height))) {
                is_playing = !is_playing;
            }

            fovTrack.DrawSidebar(time, is_playing);
            posTrack.DrawSidebar(time, is_playing);
            rotTrack.DrawSidebar(time, is_playing);
        }
        ImGui::EndChild();

        ImGui::SameLine(0, 0);

        const int scroll_height = 14;
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));
        ImGui::BeginChild("##lanes", ImVec2(ImGui::GetContentRegionAvail().x, track_height * 4 + scroll_height), false, ImGuiWindowFlags_HorizontalScrollbar);
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float scroll_x = ImGui::GetScrollX();

            for (int i = 0; i < 4; ++i) {
                draw_list->AddRectFilled(
                    ImVec2(pos.x, pos.y + i * track_height), 
                    ImVec2(ImGui::GetContentRegionAvail().x + track_width, pos.y + (i + 1) * track_height),
                    IM_COL32(255, 255, 255, (i % 2 == 1) * 10)
                );
            }

            // Drawing timestamps
            ImGui::BeginChild("##timestamps", ImVec2(track_width, track_height));
            for (int i = 0; i < track_width; i += 10) {
                draw_list->AddLine(ImVec2(pos.x + i, pos.y), ImVec2(pos.x + i, pos.y + track_height * 0.3), IM_COL32(155, 155, 155, 255), 1);
                if (i % (int)ONE_SEC_IN_PIXELS == 0) {
                    draw_list->AddLine(ImVec2(pos.x + i, pos.y), ImVec2(pos.x + i, pos.y + track_height * 0.5), IM_COL32(255, 255, 255, 255), 1);
                    draw_list->AddText(ImVec2(pos.x + i + 10, pos.y + track_height * 0.3 + 2), IM_COL32(255, 255, 255, 255), TimeToString(i * 0.01f).c_str());
                }
            }
            ImGui::EndChild();

            fovTrack.DrawLane(time, is_playing);
            posTrack.DrawLane(time, is_playing);
            rotTrack.DrawLane(time, is_playing);

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

                int scroll_x = ImGui::GetScrollX();
                bool is_in_timestamps_zone = click_pos.x > pos.x + scroll_x && click_pos.y > pos.y && click_pos.y < pos.y + track_height;
                if (is_in_timestamps_zone || s) {
                    is_dragging = true;
                    int time_in_pixels = click_pos.x - pos.x;
                    SetTime(PixelsToTime(SnapPixelsToGrid(time_in_pixels)));
                    was_clicked_in_timestamps_zone = true;
                    is_playing = false;
                }
            }
            else {
                was_clicked_in_timestamps_zone = false;
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::PopStyleVar();
        ImGui::End();
    }

/*    std::vector<InstanceData> GetInstanceData() {
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
    }*/
};
