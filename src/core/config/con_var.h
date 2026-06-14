#pragma once
#include <string>
#include <vector>
#include <optional>
#include <type_traits>
#include <algorithm>

#include "utils/debug.h"

#include "imgui.h"

class IConVar {
public:
    static inline std::vector<IConVar*> allConVars;
    static inline bool anyChangeByUi = false;

    virtual ~IConVar() = default;

    virtual const char* GetName() const = 0;
    virtual const char* GetSection() const = 0;
    virtual void SetValueFromString(const std::string& value) = 0;
    virtual std::string GetDefaultValueString() const = 0;
    virtual std::string GetValueString() const = 0;

    virtual bool WasChangedByUI() const = 0;
    virtual bool ConsumeWasChangedByUI() = 0;

    virtual void Render() = 0;
};

template<typename T>
class ConVar : public IConVar {
    const char* name{};
    const char* section{};
    const T defaultValue{};

    T value{};

    std::optional<T> minValue{};
    std::optional<T> maxValue{};

    bool wasChangedByUI = false;

public:
    ConVar(
        const char* section, const char* name, T defaultValue,
        std::optional<T> minValue = std::nullopt,
        std::optional<T> maxValue = std::nullopt
    )
        : section(section), name(name), defaultValue(defaultValue), value(defaultValue),
            minValue(minValue), maxValue(maxValue) 
    {
        IConVar::allConVars.push_back(this);
    }

	ConVar(const ConVar&) = delete;
    ConVar& operator=(const ConVar&) = delete;
    operator T() const { return value; }

    ~ConVar() override {
        auto& conVars = IConVar::allConVars;
        conVars.erase(std::remove(conVars.begin(), conVars.end(), this), conVars.end());
    }

    const char* GetName() const override { return name; }
    const char* GetSection() const override { return section; }

    void SetValue(T newValue) {
        if (minValue) newValue = std::max<T>(newValue, *minValue);
        if (maxValue) newValue = std::min<T>(newValue, *maxValue);
        value = newValue;
    }

    void SetValueFromUI(T newValue) {
        SetValue(newValue);
        wasChangedByUI = true;
    }

    void SetValueFromString(const std::string& str) override {
		T parsedValue{};

        try{
            if constexpr (std::is_same_v<T, int>) parsedValue = std::stoi(str);
            else if constexpr (std::is_same_v<T, float>) parsedValue = std::stof(str);
            else if constexpr (std::is_same_v<T, bool>) parsedValue = (str == "true" || str == "1");
        }
        catch (...) { 
			LOG_ERROR("Failed to parse value \"%s\" for ConVar \"%s\", using default value \"%s\"", str.c_str(), name, GetDefaultValueString());
            parsedValue = defaultValue; 
        }

        SetValue(parsedValue);
    }

    std::string GetDefaultValueString() const override {
        if constexpr (std::is_same_v<T, bool>) {
            return defaultValue ? "true" : "false";
        }
        else {
            return std::to_string(defaultValue);
        }
    }

    std::string GetValueString() const override {
        if constexpr (std::is_same_v<T, bool>) {
            return value ? "true" : "false";
        }
        else {
            return std::to_string(value);
        }
    }

    bool WasChangedByUI() const override {
        return wasChangedByUI;
    }

    bool ConsumeWasChangedByUI() override {
        bool was = wasChangedByUI;
        wasChangedByUI = false;
        return was;
    }

    void Render() override {
        if constexpr (std::is_same_v<T, bool>) {
            bool v = value;
            if (ImGui::Checkbox(name, &v)) {
                SetValueFromUI(v);
            }
        }
        else if constexpr (std::is_same_v<T, int>) {
            int v = value;
            if (minValue && maxValue) {
                if (ImGui::SliderInt(name, &v, *minValue, *maxValue)) {
                    SetValueFromUI(v);
                }
            }
            else if (minValue) {
                if (ImGui::InputInt(name, &v)) {
                    if (v < *minValue) v = *minValue;
                    SetValueFromUI(v);
                }
            }
            else {
                if (ImGui::InputInt(name, &v)) {
                    SetValueFromUI(v);
                }
            }
        }
        else if constexpr (std::is_same_v<T, float>) {
            float v = value;
            if (minValue && maxValue) {
                if (ImGui::SliderFloat(name, &v, *minValue, *maxValue)) {
                    SetValueFromUI(v);
                }
            }
            else {
                if (ImGui::InputFloat(name, &v)) {
                    SetValueFromUI(v);
                }
            }
        }
    }
};