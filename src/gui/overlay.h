#pragma once
#include <functional>

namespace Overlay {
	using render_callback_t = std::function<void(void)>;
	using imgui_init_callback_t = std::function<void(void)>;

	bool IsInitialized();

	void InitializeOverlay();
	void UninitializeOverlay();

	void SetRenderCallback(render_callback_t callback);
	void SetImGuiInitCallback(imgui_init_callback_t callback);
}