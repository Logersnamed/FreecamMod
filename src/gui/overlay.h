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

namespace Overlay {
	using render_callback_t = std::function<void(void)>;

	bool IsInitialized();

	void InitializeOverlay();

	void UninitializeOverlay();

	void SetRenderCallback(render_callback_t callback);

	bool ImGuiWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}