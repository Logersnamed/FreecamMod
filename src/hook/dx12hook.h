#pragma once
#include <Windows.h>
#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#include <functional>

#include <MinHook.h>

using Microsoft::WRL::ComPtr;

namespace DX12Hook {
    inline ComPtr<ID3D12Device> g_device{};
    inline ComPtr<IDXGISwapChain3> g_swap_chain{};
    inline ComPtr<ID3D12CommandQueue> g_command_queue{};

    using present_callback_t = std::function<void(IDXGISwapChain*, UINT, UINT)>;
    using resize_buffers_callback_t = std::function<void(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)>;
    using execute_command_lists_callback_t = std::function<void(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*)>;

    inline present_callback_t g_presentCallback{};
    inline resize_buffers_callback_t g_beforeResizeBuffersCallback{};
    inline resize_buffers_callback_t g_afterResizeBuffersCallback{};
    inline execute_command_lists_callback_t g_executeCommandListsCallback{};

    bool Initialize();
    void Uninitialize();
}