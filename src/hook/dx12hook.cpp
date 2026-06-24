#include "dx12hook.h"

#include "utils/debug.h"

namespace DX12Hook {
    void* g_executeCommandLists{};
    void* g_present{};
    void* g_resizeBuffers{};
    
    using present_t = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);
    using resize_buffers_t = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
    using execute_command_lists_t = void(__stdcall*)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);

    present_t g_origPresent{};
    resize_buffers_t  g_origResizeBuffers{};
    execute_command_lists_t g_origExecuteCommandLists{};
    
    bool g_after_first_present = false;

    class DummyHWND {
        HWND hwnd{};
    public:
        DummyHWND() { hwnd = CreateWindowExW(0, L"STATIC", L"dummy", WS_POPUP, 0, 0, 1, 1, NULL, NULL, NULL, NULL); }
        DummyHWND(const DummyHWND&) = delete;
        ~DummyHWND() { DestroyWindow(hwnd); }
        operator HWND() const { return hwnd; }
        DummyHWND& operator=(const DummyHWND&) = delete;
    };
    
    inline bool IsFailed(HRESULT result, LPCWSTR error_msg) {
        if (FAILED(result)) {
            LOG_ERROR("%ls (HRESULT=0x%08X)", error_msg, (UINT)result);
            return true;
        }
        return false;
    }
    
    inline bool Hook(LPVOID pTarget, LPVOID pDetour, LPVOID* ppOriginal) {
        LOG_INFO("MH_CreateHook target=%p detour=%p", pTarget, pDetour);
        if (MH_CreateHook(pTarget, pDetour, ppOriginal) != MH_OK) {
            LOG_ERROR("MH_CreateHook failed");
            return false;
        }
        if (MH_EnableHook(pTarget) != MH_OK) {
            LOG_ERROR("MH_EnableHook failed");
            return false;
        }
        return true;
    }
    
    inline HRESULT __stdcall hkPresent(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) {
        g_after_first_present = true;
        if (!g_device) swapChain->GetDevice(IID_PPV_ARGS(&g_device));
        if (!g_swap_chain) swapChain->QueryInterface(IID_PPV_ARGS(&g_swap_chain));
        if (g_presentCallback) g_presentCallback(swapChain, syncInterval, flags);
        return g_origPresent(swapChain, syncInterval, flags);
    }
    
    inline HRESULT __stdcall hkResizeBuffers(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
        g_is_resizing = true;

        if (g_beforeResizeBuffersCallback)
            g_beforeResizeBuffersCallback(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
    
        HRESULT hr = g_origResizeBuffers(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
    
        if (g_afterResizeBuffersCallback)
            g_afterResizeBuffersCallback(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
    
        g_is_resizing = false;

        return hr;
    }
    
    inline void __stdcall hkExecuteCommandLists(ID3D12CommandQueue* commandQueue, UINT numCommandLists, ID3D12CommandList* const* ppCommandLists) {
        if (!g_command_queue && g_after_first_present) {
            g_command_queue = commandQueue;
        }
        g_after_first_present = false;

        if (g_executeCommandListsCallback) g_executeCommandListsCallback(commandQueue, numCommandLists, ppCommandLists);
        g_origExecuteCommandLists(commandQueue, numCommandLists, ppCommandLists);
    }
    
    inline ComPtr<ID3D12Device> CreateDummyDevice() {
        ComPtr<ID3D12Device> device{};
        HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));
        if (IsFailed(hr, L"Failed CreateDummyDevice")) return nullptr;
        return device;
    }
    
    inline ComPtr<ID3D12CommandQueue> CreateDummyCommandQueue(ID3D12Device* device) {
        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    
        ComPtr<ID3D12CommandQueue> commandQueue;
        HRESULT hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue));
        if (IsFailed(hr, L"Failed CreateCommandQueue")) return nullptr;
        return commandQueue;
    }
    
    inline ComPtr<IDXGISwapChain> CreateDummySwapChain(ID3D12CommandQueue* commandQueue) {
        ComPtr<IDXGIFactory4> factory{};
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        if (IsFailed(hr, L"Failed to CreateDXGIFactory1")) return nullptr;
    
        DummyHWND dummyWindow{};
        if (!dummyWindow) return nullptr;
    
        DXGI_SWAP_CHAIN_DESC swapChainDesc{};
        swapChainDesc.BufferCount = 2;
        swapChainDesc.BufferDesc.Width = 100;
        swapChainDesc.BufferDesc.Height = 100;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.OutputWindow = dummyWindow;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.Windowed = TRUE;
    
        ComPtr<IDXGISwapChain> swapChain{};
        hr = factory->CreateSwapChain(commandQueue, &swapChainDesc, &swapChain);
        if (IsFailed(hr, L"Failed to CreateSwapChain")) return nullptr;
        return swapChain;
    }

    bool Initialize() {
        LOG_INFO("Initializing DX12Hook...");
        ComPtr<ID3D12Device> device = CreateDummyDevice();
        if (!device) return false;
        LOG_INFO("Device=%p", device.Get());

        ComPtr<ID3D12CommandQueue> commandQueue = CreateDummyCommandQueue(device.Get());
        if (!commandQueue) return false;
        LOG_INFO("CommandQueue=%p", commandQueue.Get());

        void** commandQueueVTable = *reinterpret_cast<void***>(commandQueue.Get());
        g_executeCommandLists = commandQueueVTable[10];
        LOG_INFO("ExecuteCommandLists = %p", g_executeCommandLists);

        ComPtr<IDXGISwapChain> swapChain = CreateDummySwapChain(commandQueue.Get());
        if (!swapChain) return false;
        LOG_INFO("SwapChain=%p", swapChain.Get());

        void** swapChainVTable = *reinterpret_cast<void***>(swapChain.Get());
        g_present = swapChainVTable[8];
        g_resizeBuffers = swapChainVTable[13];
        LOG_INFO("Present = %p", g_present);
        LOG_INFO("ResizeBuffers = %p", g_resizeBuffers);

        if (!Hook(g_present, &hkPresent, (void**)&g_origPresent))  return false;
        if (!Hook(g_resizeBuffers, &hkResizeBuffers, (void**)&g_origResizeBuffers)) return false;
        if (!Hook(g_executeCommandLists, &hkExecuteCommandLists, (void**)&g_origExecuteCommandLists)) return false;

        return true;
    }

    void Uninitialize() {
        LOG_INFO("Uninitializing DX12Hook...");
        MH_DisableHook(g_present);
        MH_DisableHook(g_resizeBuffers);
        MH_DisableHook(g_executeCommandLists);
        MH_RemoveHook(g_present);
        MH_RemoveHook(g_resizeBuffers);
        MH_RemoveHook(g_executeCommandLists);
        g_command_queue = nullptr;
        g_swap_chain.Reset();
        g_device.Reset();
    }
}