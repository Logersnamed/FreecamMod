#include "gui/overlay.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace Overlay {
    struct FrameContext {
        ID3D12CommandAllocator* command_allocator{};
        ID3D12Resource* render_target{};
        D3D12_CPU_DESCRIPTOR_HANDLE  render_target_descriptor{};
        UINT64 fence_value = 0;
    };

    render_callback_t g_render_callback{};
    bool g_is_initialized = false;

    HWND g_hwnd{};
    UINT g_buffers_count{};
    DXGI_FORMAT g_rtv_format{};

    FrameContext* g_frame_contexts{};       // delete this
    ID3D12GraphicsCommandList* g_command_list{};
    ID3D12DescriptorHeap* g_rtv_heap{};
    ID3D12DescriptorHeap* g_srv_heap{};
    ID3D12Fence* g_fence{};
    HANDLE g_fence_event{};
    UINT64 g_fence_last_signaled = 0;
    UINT g_frame_index = 0;

    bool IsInitialized() { return g_is_initialized; }
    void SetRenderCallback(render_callback_t callback) { g_render_callback = callback; }

    bool ImGuiWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (!g_is_initialized) return false;

        ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

        const auto& io = ImGui::GetIO();

        if (io.WantCaptureMouse) {
            switch (msg) {
            case WM_INPUT:
            case WM_LBUTTONDOWN: case WM_LBUTTONUP:
            case WM_RBUTTONDOWN: case WM_RBUTTONUP:
            case WM_MBUTTONDOWN: case WM_MBUTTONUP:
            case WM_MOUSEWHEEL:
            case WM_MOUSEMOVE:
                return true;
            }
        }

        if (io.WantCaptureKeyboard) {
            switch (msg) {
            case WM_INPUT:
            case WM_KEYDOWN: case WM_KEYUP:
            case WM_SYSKEYDOWN: case WM_SYSKEYUP:
            case WM_CHAR:
                return true;
            }
        }
        return false;
    }

    void WaitForPendingOperations() {
        DX12Hook::g_command_queue->Signal(g_fence, ++g_fence_last_signaled);
        g_fence->SetEventOnCompletion(g_fence_last_signaled, g_fence_event);
        ::WaitForSingleObject(g_fence_event, INFINITE);
    }

    FrameContext* WaitForNextFrameContext() {
        FrameContext* frame_context = &g_frame_contexts[g_frame_index % g_buffers_count];
        if (g_fence->GetCompletedValue() < frame_context->fence_value) {
            g_fence->SetEventOnCompletion(frame_context->fence_value, g_fence_event);
            ::WaitForSingleObject(g_fence_event, INFINITE);
        }
        return frame_context;
    }

    void CreateRenderTarget() {
        for (UINT i = 0; i < g_buffers_count; ++i) {
            ID3D12Resource* back_buffer{};
            DX12Hook::g_swap_chain->GetBuffer(i, IID_PPV_ARGS(&back_buffer));
            DX12Hook::g_device->CreateRenderTargetView(back_buffer, nullptr, g_frame_contexts[i].render_target_descriptor);
            g_frame_contexts[i].render_target = back_buffer;
        }
    }

    void ReleaseRenderTarget() {
        for (UINT i = 0; i < g_buffers_count; ++i) {
            if (g_frame_contexts[i].render_target) {
                g_frame_contexts[i].render_target->Release();
                g_frame_contexts[i].render_target = nullptr;
            }
            g_frame_contexts[i].fence_value = 0;
        }
    }

    bool InitializeImGui() {
        ImGui_ImplWin32_EnableDpiAwareness();
        float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);       
        style.FontScaleDpi = main_scale;

        ImGui_ImplWin32_Init(g_hwnd);
        ImGui_ImplDX12_Init(
            DX12Hook::g_device.Get(),
            g_buffers_count,
            g_rtv_format,
            g_srv_heap,
            g_srv_heap->GetCPUDescriptorHandleForHeapStart(),
            g_srv_heap->GetGPUDescriptorHandleForHeapStart()
        );
        io.Fonts->Build();
        ImGui_ImplDX12_CreateDeviceObjects();

        return true;
    }

    void Initialize() {
        DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1{};
        if (FAILED(DX12Hook::g_swap_chain->GetDesc1(&swap_chain_desc1))) return;
        g_buffers_count = swap_chain_desc1.BufferCount;
        g_rtv_format = swap_chain_desc1.Format;
        DX12Hook::g_swap_chain->GetHwnd(&g_hwnd);

        g_frame_contexts = new FrameContext[g_buffers_count]{};

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc{};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = g_buffers_count;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            desc.NodeMask = 1;
            if (FAILED(DX12Hook::g_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_rtv_heap)))) 
                return;

            SIZE_T increment = DX12Hook::g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE handle = g_rtv_heap->GetCPUDescriptorHandleForHeapStart();
            for (UINT i = 0; i < g_buffers_count; i++) {
                g_frame_contexts[i].render_target_descriptor = handle;
                handle.ptr += increment;
            }
        }

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc{};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = 64;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = 1;
            if (FAILED(DX12Hook::g_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_srv_heap)))) 
                return;
        }

        for (UINT i = 0; i < g_buffers_count; ++i) {
            if (FAILED(DX12Hook::g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frame_contexts[i].command_allocator)))) 
                return;
        }

        if (FAILED(DX12Hook::g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frame_contexts[0].command_allocator, nullptr, IID_PPV_ARGS(&g_command_list)))) 
            return;

        if (FAILED(g_command_list->Close())) 
            return;

        if (FAILED(DX12Hook::g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)))) 
            return;

        g_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!g_fence_event) return;

        CreateRenderTarget();
        InitializeImGui();

        g_is_initialized = true;
        return;
    }

    void Render() {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (g_render_callback) g_render_callback();

        ImGui::Render();

        FrameContext* frame_context = WaitForNextFrameContext();
        UINT back_buffer_idx = DX12Hook::g_swap_chain->GetCurrentBackBufferIndex();
        frame_context->command_allocator->Reset();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = g_frame_contexts[back_buffer_idx].render_target;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        g_command_list->Reset(frame_context->command_allocator, nullptr);
        g_command_list->ResourceBarrier(1, &barrier);
        g_command_list->OMSetRenderTargets(1, &g_frame_contexts[back_buffer_idx].render_target_descriptor, FALSE, nullptr);
        g_command_list->SetDescriptorHeaps(1, &g_srv_heap);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_command_list);

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        g_command_list->ResourceBarrier(1, &barrier);
        g_command_list->Close();

        DX12Hook::g_command_queue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&g_command_list);
        DX12Hook::g_command_queue->Signal(g_fence, ++g_fence_last_signaled);
        frame_context->fence_value = g_fence_last_signaled;
        g_frame_index++;
    }

    void OnPresent(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) {
        if (!DX12Hook::g_swap_chain || !DX12Hook::g_device || !DX12Hook::g_command_queue) return;

        if (!g_is_initialized) {
            Initialize();
        }

        if (g_is_initialized) {
            Render();
        }
    }

    void BeforeResizeBuffers(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
        if (!DX12Hook::g_swap_chain || !DX12Hook::g_device || !DX12Hook::g_command_queue) return;

        if (g_is_initialized) {
            g_is_initialized = false;
            WaitForPendingOperations();
            ImGui_ImplDX12_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ReleaseRenderTarget();
        }
    }

    void AfterResizeBuffers(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
        if (!DX12Hook::g_swap_chain || !DX12Hook::g_device || !DX12Hook::g_command_queue) return;

        if (!g_is_initialized) {
            g_buffers_count = bufferCount;
            g_rtv_format = newFormat;

            if (DX12Hook::g_swap_chain)
                DX12Hook::g_swap_chain.Reset();

            if (FAILED(swapChain->QueryInterface(IID_PPV_ARGS(&DX12Hook::g_swap_chain))))
                return;

            CreateRenderTarget();

            DX12Hook::g_swap_chain->GetHwnd(&g_hwnd);
            ImGui_ImplWin32_Init(g_hwnd);
            ImGui_ImplDX12_Init(
                DX12Hook::g_device.Get(),
                g_buffers_count,
                g_rtv_format,
                g_srv_heap,
                g_srv_heap->GetCPUDescriptorHandleForHeapStart(),
                g_srv_heap->GetGPUDescriptorHandleForHeapStart()
            );
            ImGui_ImplDX12_CreateDeviceObjects();
            g_is_initialized = true;
        }
    }

    void OnExecuteCommandLists(ID3D12CommandQueue* commandQueue, UINT numCommandLists, ID3D12CommandList* const* ppCommandLists) {}

    void InitializeOverlay() {
        DX12Hook::g_presentCallback = OnPresent;
        DX12Hook::g_beforeResizeBuffersCallback = BeforeResizeBuffers;
        DX12Hook::g_afterResizeBuffersCallback = AfterResizeBuffers;
        DX12Hook::g_executeCommandListsCallback = OnExecuteCommandLists;
    }

    void UninitializeOverlay() {
        DX12Hook::g_presentCallback = nullptr;
        DX12Hook::g_beforeResizeBuffersCallback = nullptr;
        DX12Hook::g_afterResizeBuffersCallback = nullptr;
        DX12Hook::g_executeCommandListsCallback = nullptr;

        if (g_is_initialized) {
            g_is_initialized = false;
            WaitForPendingOperations();
            ImGui_ImplDX12_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();

            ReleaseRenderTarget();
        }

        if (g_frame_contexts) {
            for (UINT i = 0; i < g_buffers_count; ++i) {
                if (g_frame_contexts[i].command_allocator) {
                    g_frame_contexts[i].command_allocator->Release();
                    g_frame_contexts[i].command_allocator = nullptr;
                }
            }

            delete[] g_frame_contexts;
            g_frame_contexts = nullptr;
        }

        if (g_command_list) { g_command_list->Release(); g_command_list = nullptr; }
        if (g_rtv_heap) { g_rtv_heap->Release(); g_rtv_heap = nullptr; }
        if (g_srv_heap) { g_srv_heap->Release(); g_srv_heap = nullptr; }
        if (g_fence) { g_fence->Release(); g_fence = nullptr; }
        if (g_fence_event) { CloseHandle(g_fence_event); g_fence_event = nullptr; }
    }
}