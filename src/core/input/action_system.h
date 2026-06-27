#pragma once
#include <cstdint>
#include <array>
#include <optional>
#include <string>

#include "core/events.h"
#include "core/input/action_type.h"
#include "core/input/input.h"
#include "utils/types.h"

class Action {
public:
	static constexpr int MAX_KEYS = 4;
	using Keys = FixedVec<int, MAX_KEYS>;

private:
	ActionType type;
	Keys keysRequired;
	Keys keysRestricted;

public:
	Action(ActionType type, const Keys& keysRequired, const Keys& keysRestricted = {})
		: type(type), keysRequired(keysRequired), keysRestricted(keysRestricted) {	}

	ActionType GetType() const { return type; }
	const Keys& GetRequiredKeys() const { return keysRequired; }
	const Keys& GetRestrictedKeys() const { return keysRestricted; }
	const char* GetName() const { return ActionTypeName[static_cast<int8_t>(type)]; }

	bool IsPressed(const Input& input) const {
		if (keysRequired.empty() && keysRestricted.empty()) return false;
		for (int key : keysRequired) {
			if (!input.IsPressed(key)) return false;
		}
		for (int key : keysRestricted) {
			if (input.IsPressed(key)) return false;
		}
		return true;
	}

	bool IsJustPressed(const Input& input) const {
		if (keysRequired.empty() && keysRestricted.empty()) return false;
		bool anyJustPressed = false;
		for (int key : keysRequired) {
			if (!input.IsPressed(key)) return false;
			if (input.IsJustPressed(key)) anyJustPressed = true;
		}
		for (int key : keysRestricted) {
			if (input.IsPressed(key)) return false;
		}
		return anyJustPressed;
	}

private:
	static inline const char* ActionTypeName[] = {
		#define X(name) #name,
		ACTION_TYPES
		#undef X
		"Count"
	};
};

class ActionManager {
	std::array<std::optional<Action>, static_cast<size_t>(ActionType::Count)> actions{};
	std::array<bool, static_cast<size_t>(ActionType::Count)> blocked{};

public:
	ActionManager() {
		EventBus::Subscribe<Event::BlockActions>([this](const Event::BlockActions& event) {
			blocked = event.blocked;
		});
	}

	void Block(ActionType type) { blocked[static_cast<size_t>(type)] = true; }
	void Unblock(ActionType type) { blocked[static_cast<size_t>(type)] = false; }

	void BlockAll() { blocked.fill(true); }
	void UnblockAll() { blocked.fill(false); }

	bool IsPressed(ActionType type, const Input& input) const {
		if (blocked[static_cast<size_t>(type)]) return false;
		const auto& action = actions[static_cast<size_t>(type)];
		return action.has_value() && action->IsPressed(input);
	}

	bool IsJustPressed(ActionType type, const Input& input) const {
		if (blocked[static_cast<size_t>(type)]) return false;
		const auto& action = actions[static_cast<size_t>(type)];
		return action.has_value() && action->IsJustPressed(input);
	}

	void BindAction(const Action& action) {
		actions[static_cast<size_t>(action.GetType())] = action;
	}
};