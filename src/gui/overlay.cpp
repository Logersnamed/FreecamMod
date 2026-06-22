#include "gui/overlay.h"
#include "core/input/input.h"
#include "utils/debug.h"

#include "core/game_data_manager.h"

#include <wrl.h>

#include "directxmath.h"

using Microsoft::WRL::ComPtr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace Overlay {
    struct FrameContext {
        ComPtr<ID3D12CommandAllocator> command_allocator{};
        ComPtr<ID3D12Resource> render_target{};
        D3D12_CPU_DESCRIPTOR_HANDLE  render_target_descriptor{};
        UINT64 fence_value = 0;
    };

    render_callback_t g_render_callback{};
    imgui_init_callback_t g_imgui_init_callback{};
    bool g_is_initialized = false;

    HWND g_hwnd{};
    UINT g_buffer_count{};
    DXGI_FORMAT g_rtv_format{};

    FrameContext* g_frame_contexts{};

    ComPtr<ID3D12GraphicsCommandList> g_command_list{};
    ComPtr<ID3D12DescriptorHeap> g_rtv_heap{};
    ComPtr<ID3D12DescriptorHeap> g_srv_heap{};
    ComPtr<ID3D12Fence> g_fence{};
    HANDLE g_fence_event{};
    UINT64 g_fence_last_signaled = 0;

    UINT g_attempts_to_init = 0;
    UINT max_attempts_to_init = 10;

    bool IsInitialized() { return g_is_initialized; }
    void SetRenderCallback(render_callback_t callback) { g_render_callback = std::move(callback); }
    void SetImGuiInitCallback(imgui_init_callback_t callback) { g_imgui_init_callback = std::move(callback); }

    void WaitForGPU() {
        DX12Hook::g_command_queue->Signal(g_fence.Get(), ++g_fence_last_signaled);
        g_fence->SetEventOnCompletion(g_fence_last_signaled, g_fence_event);
        ::WaitForSingleObject(g_fence_event, INFINITE);
    }

    void CreateRenderTargets() {
        LOG_INFO("Creating render targets...");
        for (UINT i = 0; i < g_buffer_count; ++i) {
            ComPtr<ID3D12Resource> back_buffer{};
            DX12Hook::g_swap_chain->GetBuffer(i, IID_PPV_ARGS(&back_buffer));
            DX12Hook::g_device->CreateRenderTargetView(back_buffer.Get(), nullptr, g_frame_contexts[i].render_target_descriptor);
            g_frame_contexts[i].render_target = back_buffer;
        }
    }

    void ReleaseRenderTargets() {
        LOG_INFO("Releasing render targets...");
        for (UINT i = 0; i < g_buffer_count; ++i) {
            g_frame_contexts[i].render_target.Reset();
            g_frame_contexts[i].fence_value = 0;
        }
    }

    void Cleanup() {
        LOG_INFO("Cleaning up overlay...");
        if (g_frame_contexts) {
            for (UINT i = 0; i < g_buffer_count; ++i) {
                g_frame_contexts[i].command_allocator.Reset();
            }

            delete[] g_frame_contexts;
            g_frame_contexts = nullptr;
        }

        g_command_list.Reset();
        g_rtv_heap.Reset();
        g_srv_heap.Reset();
        g_fence.Reset();
        if (g_fence_event) { CloseHandle(g_fence_event); g_fence_event = nullptr; }
    }

    void ShutdownImGui() {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
    }

    void InitializeImGuiBackend(HWND hwnd) {
        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX12_Init(
            DX12Hook::g_device.Get(),
            g_buffer_count,
            g_rtv_format,
            g_srv_heap.Get(),
            g_srv_heap->GetCPUDescriptorHandleForHeapStart(),
            g_srv_heap->GetGPUDescriptorHandleForHeapStart()
        );
        ImGui_ImplDX12_CreateDeviceObjects();
    }

    void InitializeImGui() {
        LOG_INFO("Initializing ImGui...");
        ImGui_ImplWin32_EnableDpiAwareness();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

        if (g_imgui_init_callback)
            g_imgui_init_callback();

        io.Fonts->Build();
        InitializeImGuiBackend(g_hwnd);
    }

    static inline bool InitViewport() {
        g_viewport.TopLeftX = 0;
        g_viewport.TopLeftY = 0;
        g_viewport.Width = static_cast<float>(g_window_width);
        g_viewport.Height = static_cast<float>(g_window_height);
        g_viewport.MinDepth = 0.0f;
        g_viewport.MaxDepth = 1.0f;
        return true;
    }


    bool Initialize() {
        LOG_INFO("Initializing overlay... Attempt: %d", g_attempts_to_init);

        DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1{};
        if (FAILED(DX12Hook::g_swap_chain->GetDesc1(&swap_chain_desc1))) {
            LOG_ERROR("Failed to get swapchain description");
            return false;
        }
        g_buffer_count = swap_chain_desc1.BufferCount;
        g_rtv_format = swap_chain_desc1.Format;
        DX12Hook::g_swap_chain->GetHwnd(&g_hwnd);

        g_frame_contexts = new FrameContext[g_buffer_count]{};

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc{};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = g_buffer_count;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            desc.NodeMask = 1;
            if (FAILED(DX12Hook::g_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_rtv_heap)))) {
                LOG_ERROR("Failed to create RTV descriptor heap");
                return false;
            }

            SIZE_T increment = DX12Hook::g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE handle = g_rtv_heap->GetCPUDescriptorHandleForHeapStart();
            for (UINT i = 0; i < g_buffer_count; i++) {
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
                return false;
        }

        for (UINT i = 0; i < g_buffer_count; ++i) {
            if (FAILED(DX12Hook::g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frame_contexts[i].command_allocator))))
                return false;
        }

        if (FAILED(DX12Hook::g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frame_contexts[0].command_allocator.Get(), nullptr, IID_PPV_ARGS(&g_command_list))))
            return false;

        if (FAILED(g_command_list->Close())) 
            return false;

        if (FAILED(DX12Hook::g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence))))
            return false;

        g_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!g_fence_event) return false;

        CreateRenderTargets();
        InitializeImGui();

        InitViewport();

        if (!CreateMaterial(DX12Hook::g_device.Get(), &cameraMaterial))
            return false;

        cameraInstance.material = &cameraMaterial;
        cameraInstance.mesh = new Mesh();
        CreateCameraFrustumMesh(DX12Hook::g_device.Get(), cameraInstance.mesh);
        cameraInstance.Init(DX12Hook::g_device.Get());

        g_is_initialized = true;
        LOG_INFO("Overlay initialized successfully");
        return true;
    }

    void Render(RenderObject& object, const DirectX::XMMATRIX& view_proj) {
        DirectX::XMMATRIX model = DirectX::XMMatrixScaling(object.scale.x, object.scale.y, object.scale.z) *
            DirectX::XMMatrixRotationRollPitchYaw(object.rotation.x, object.rotation.y, object.rotation.z) *
            DirectX::XMMatrixTranslation(object.position.x, object.position.y, object.position.z);
        DirectX::XMMATRIX mvp_transposed = DirectX::XMMatrixTranspose(model * view_proj);
        object.UpdateConstantBuffer(mvp_transposed);

        g_command_list->SetGraphicsRootSignature(object.material->root_signature.Get());
        g_command_list->SetPipelineState(object.material->pso.Get());
        g_command_list->SetGraphicsRootConstantBufferView(0, object.constant_buffer->GetGPUVirtualAddress());

        g_command_list->IASetPrimitiveTopology(object.mesh->topology);
        g_command_list->IASetVertexBuffers(0, 1, &object.mesh->vertex_buffer_view);
        g_command_list->IASetIndexBuffer(&object.mesh->index_buffer_view);

        g_command_list->DrawIndexedInstanced(object.mesh->index_count, 1, 0, 0, 0);
    }

    void Render() {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (g_render_callback) g_render_callback();

        ImGui::Render();

        UINT back_buffer_idx = DX12Hook::g_swap_chain->GetCurrentBackBufferIndex();
        FrameContext* frame_context = &g_frame_contexts[back_buffer_idx];
        if (g_fence->GetCompletedValue() < frame_context->fence_value) {
            g_fence->SetEventOnCompletion(frame_context->fence_value, g_fence_event);
            ::WaitForSingleObject(g_fence_event, INFINITE);
        }

        frame_context->command_allocator->Reset();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = g_frame_contexts[back_buffer_idx].render_target.Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        ID3D12DescriptorHeap* heaps[] = { g_srv_heap.Get() };
        g_command_list->Reset(frame_context->command_allocator.Get(), nullptr);
        g_command_list->ResourceBarrier(1, &barrier);
        g_command_list->OMSetRenderTargets(1, &g_frame_contexts[back_buffer_idx].render_target_descriptor, FALSE, nullptr);
        g_command_list->SetDescriptorHeaps(1, heaps);

        g_command_list->RSSetViewports(1, &g_viewport);

        D3D12_RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = g_window_width;
        rect.bottom = g_window_height;
        g_command_list->RSSetScissorRects(1, &rect);

        {

            auto* fieldArea = GameDataManager::FieldArea.Get();
            if (fieldArea) {
                auto* gameRend = fieldArea->gameRend;
                if (gameRend) {
                    auto* camera = gameRend->IsFreecamEnabled() ? gameRend->csDebugCam : gameRend->csPersCam1;
                    if (camera) {
                        using namespace DirectX;

                        const float3 r = camera->right();
                        const float3 u = camera->up();
                        const float3 f = camera->forward();
                        const float3 p = camera->matrix.position();
                        XMMATRIX view = XMMATRIX(
                            r.x, u.x, f.x, 0.0f,
                            r.y, u.y, f.y, 0.0f,
                            r.z, u.z, f.z, 0.0f,

                            -float3::dot(r, p),
                            -float3::dot(u, p),
                            -float3::dot(f, p),
                            1.0f
                        );

                        float aspect_ratio = (float)g_window_width / (float)g_window_height;

                        XMMATRIX projection = XMMatrixPerspectiveFovLH(camera->fov, aspect_ratio, 0.1f, 10000.0f);

                        XMMATRIX vp = view * projection;

                        //Render(cube, vp);
                        for (auto& cam : cameras) {
                            Render(cam, vp);
                        }

                    }
                }
            }

        }

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_command_list.Get());

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        g_command_list->ResourceBarrier(1, &barrier);
        g_command_list->Close();

        ID3D12CommandList* lists[] = { g_command_list.Get() };
        DX12Hook::g_command_queue->ExecuteCommandLists(1, lists);
        DX12Hook::g_command_queue->Signal(g_fence.Get(), ++g_fence_last_signaled);
        frame_context->fence_value = g_fence_last_signaled;
    }


    void OnPresent(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) {
        if (!DX12Hook::g_swap_chain || !DX12Hook::g_device || !DX12Hook::g_command_queue) return;

        if (!DX12Hook::g_is_resizing) {
            if (!g_is_initialized && g_attempts_to_init < max_attempts_to_init) {
                if (!Initialize()) {
                    Cleanup();
                    ++g_attempts_to_init;
                }
            }

            if (g_is_initialized) {
                Render();

                Input* instance = Input::GetInstance();
                if (instance) instance->Reset();
            }
        }
    }

    void BeforeResizeBuffers(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
        LOG_INFO("Resizing buffers... (%ux%u fmt=%u)", width, height, newFormat);

        if (!DX12Hook::g_swap_chain || !DX12Hook::g_device || !DX12Hook::g_command_queue) return;

        if (g_is_initialized) {
            WaitForGPU();
            ShutdownImGui();
            ReleaseRenderTargets();
        }
    }

    void AfterResizeBuffers(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
        if (!DX12Hook::g_swap_chain || !DX12Hook::g_device || !DX12Hook::g_command_queue) return;

        if (g_is_initialized) {
            g_buffer_count = bufferCount;
            g_rtv_format = newFormat;

            if (DX12Hook::g_swap_chain)
                DX12Hook::g_swap_chain.Reset();

            if (FAILED(swapChain->QueryInterface(IID_PPV_ARGS(&DX12Hook::g_swap_chain))))
                return;

            CreateRenderTargets();

            DX12Hook::g_swap_chain->GetHwnd(&g_hwnd);
            InitializeImGuiBackend(g_hwnd);
        }

        LOG_INFO("Buffer resizing completed");
    }

    void OnExecuteCommandLists(ID3D12CommandQueue* commandQueue, UINT numCommandLists, ID3D12CommandList* const* ppCommandLists) {}

    void InitializeOverlay() {
        DX12Hook::g_presentCallback = OnPresent;
        DX12Hook::g_beforeResizeBuffersCallback = BeforeResizeBuffers;
        DX12Hook::g_afterResizeBuffersCallback = AfterResizeBuffers;
        DX12Hook::g_executeCommandListsCallback = OnExecuteCommandLists;
    }

    void UninitializeOverlay() {
        LOG_INFO("Uninitializing overlay...");

        DX12Hook::g_presentCallback = nullptr;
        DX12Hook::g_beforeResizeBuffersCallback = nullptr;
        DX12Hook::g_afterResizeBuffersCallback = nullptr;
        DX12Hook::g_executeCommandListsCallback = nullptr;

        if (g_is_initialized) {
            g_is_initialized = false;
            WaitForGPU();
            ShutdownImGui();
            ImGui::DestroyContext();

            ReleaseRenderTargets();
        }

        Cleanup();
    }
}