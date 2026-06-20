#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <MinHook.h>

#include <functional>

#include "hook/dx12hook.h"

#include "gui/render_object.h"

namespace Overlay {
	using render_callback_t = std::function<void(void)>;
	using imgui_init_callback_t = std::function<void(void)>;

	bool IsInitialized();

	void InitializeOverlay();
	void UninitializeOverlay();

	void SetRenderCallback(render_callback_t callback);
	void SetImGuiInitCallback(imgui_init_callback_t callback);

	inline D3D12_VIEWPORT g_viewport{};

	inline int g_window_width = 1920;
	inline int g_window_height = 1080;

	inline Material cameraMaterial;
	inline std::vector<RenderObject> cameras;
}