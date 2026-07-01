#pragma once

template<typename T>
struct Keyframe {
    T data;
    float time;

    bool is_selected = false;
};