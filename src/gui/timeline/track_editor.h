#pragma once
#include "utils/types.h"
#include "imgui.h"

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