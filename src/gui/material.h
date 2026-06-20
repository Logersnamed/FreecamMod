#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include "directxmath.h"
#include "d3dcompiler.h"
#pragma comment(lib, "d3dcompiler.lib")

class Material {
public:
	ComPtr<ID3D12PipelineState> pso{};
	ComPtr<ID3D12RootSignature> root_signature{};
};


static inline D3D12_BLEND_DESC get_blend_state() {
    D3D12_BLEND_DESC blend_desc = {};

    blend_desc.AlphaToCoverageEnable = FALSE;
    blend_desc.IndependentBlendEnable = FALSE;

    D3D12_RENDER_TARGET_BLEND_DESC default_render_target_blend_desc = {};
    default_render_target_blend_desc.BlendEnable = FALSE;
    default_render_target_blend_desc.LogicOpEnable = FALSE;
    default_render_target_blend_desc.SrcBlend = D3D12_BLEND_ONE;
    default_render_target_blend_desc.DestBlend = D3D12_BLEND_ZERO;
    default_render_target_blend_desc.BlendOp = D3D12_BLEND_OP_ADD;
    default_render_target_blend_desc.SrcBlendAlpha = D3D12_BLEND_ONE;
    default_render_target_blend_desc.DestBlendAlpha = D3D12_BLEND_ZERO;
    default_render_target_blend_desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    default_render_target_blend_desc.LogicOp = D3D12_LOGIC_OP_NOOP;
    default_render_target_blend_desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) {
        blend_desc.RenderTarget[i] = default_render_target_blend_desc;
    }

    return blend_desc;
}

static inline D3D12_RASTERIZER_DESC get_rasterizer_state() {
    D3D12_RASTERIZER_DESC rasterizer_desc = {};

    rasterizer_desc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizer_desc.CullMode = D3D12_CULL_MODE_BACK;
    rasterizer_desc.FrontCounterClockwise = FALSE;
    rasterizer_desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizer_desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizer_desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizer_desc.DepthClipEnable = TRUE;
    rasterizer_desc.MultisampleEnable = FALSE;
    rasterizer_desc.AntialiasedLineEnable = FALSE;
    rasterizer_desc.ForcedSampleCount = 0;
    rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    return rasterizer_desc;
}

static inline D3D12_DEPTH_STENCIL_DESC get_depth_stencil_state() {
    D3D12_DEPTH_STENCIL_DESC depth_stencil_desc = {};

    depth_stencil_desc.DepthEnable = FALSE;
    depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;

    depth_stencil_desc.StencilEnable = FALSE;
    depth_stencil_desc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depth_stencil_desc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

    depth_stencil_desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    depth_stencil_desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depth_stencil_desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

    depth_stencil_desc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depth_stencil_desc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    depth_stencil_desc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depth_stencil_desc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    return depth_stencil_desc;
}

static inline bool CreateMaterial(ID3D12Device* device, Material* material) {
    ComPtr<ID3DBlob> vertex_shader{};
    ComPtr<ID3DBlob> pixel_shader{};

    // init shaders
    {
        ComPtr<ID3DBlob> error;
        HRESULT hr = D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "VSMain", "vs_5_0", 0, 0, &vertex_shader, &error);
        if (FAILED(hr)) {
            if (error) OutputDebugStringA((char*)error->GetBufferPointer());
            else OutputDebugStringA("Shader file not found or unknown error\n");
            return false;
        }
        OutputDebugStringA("Vertex shader compiled successfully.\n");

        hr = D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "PSMain", "ps_5_0", 0, 0, &pixel_shader, &error);
        if (FAILED(hr)) {
            if (error) OutputDebugStringA((char*)error->GetBufferPointer());
            else OutputDebugStringA("Shader file not found or unknown error\n");
            return false;
        }
        OutputDebugStringA("Pixel shader compiled successfully.\n");

        constexpr int num_params = 1;
        D3D12_ROOT_PARAMETER root_params[num_params];
        root_params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        root_params[0].Descriptor.ShaderRegister = 0;
        root_params[0].Descriptor.RegisterSpace = 0;
        root_params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_SIGNATURE_DESC sig_desc{};
        sig_desc.NumParameters = 1;
        sig_desc.pParameters = root_params;
        sig_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        ComPtr<ID3DBlob> blob{};
        D3D12SerializeRootSignature(&sig_desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr);
        HR_CHECK(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&material->root_signature)));
    }

    // init pso
    {
        const D3D12_INPUT_ELEMENT_DESC element_descs[2] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        };

        D3D12_INPUT_LAYOUT_DESC input_layout_desc{};
        input_layout_desc.NumElements = 2;
        input_layout_desc.pInputElementDescs = element_descs;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc{};
        pipeline_state_desc.pRootSignature = material->root_signature.Get();
        pipeline_state_desc.VS = { vertex_shader->GetBufferPointer(), vertex_shader->GetBufferSize() };
        pipeline_state_desc.PS = { pixel_shader->GetBufferPointer(), pixel_shader->GetBufferSize() };
        pipeline_state_desc.StreamOutput = {};
        pipeline_state_desc.BlendState = get_blend_state();
        pipeline_state_desc.SampleMask = UINT_MAX;
        pipeline_state_desc.RasterizerState = get_rasterizer_state();
        pipeline_state_desc.DepthStencilState = get_depth_stencil_state();
        pipeline_state_desc.InputLayout = input_layout_desc;
        pipeline_state_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        pipeline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pipeline_state_desc.NumRenderTargets = 1;
        pipeline_state_desc.RTVFormats[0] = { DXGI_FORMAT_R8G8B8A8_UNORM };
        pipeline_state_desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
        pipeline_state_desc.SampleDesc = { 1,0 };
        pipeline_state_desc.NodeMask = 0;
        pipeline_state_desc.CachedPSO = { NULL, 0 };
        pipeline_state_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

        HR_CHECK(device->CreateGraphicsPipelineState(&pipeline_state_desc, IID_PPV_ARGS(&material->pso)));
    }

    return true;
}