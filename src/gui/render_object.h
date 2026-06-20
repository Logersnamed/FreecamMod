#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include <d3d12.h>
#include <dxgi1_6.h>
#include "directxmath.h"
#include "d3dcompiler.h"
#pragma comment(lib, "d3dcompiler.lib")

#include <cstdint>

#include "utils/types.h"
#include "gui/helpers.h"
#include "gui/mesh.h"
#include "gui/material.h"

struct TransformData {
    DirectX::XMMATRIX mvp;
};

class RenderObject {
public:
	Mesh* mesh{};
	Material* material{};

    DirectX::XMFLOAT3 position{ 0, 0, 0 };
    DirectX::XMFLOAT3 rotation{ 0, 0, 0 };
    DirectX::XMFLOAT3 scale{ 1, 1, 1 };

    ComPtr<ID3D12Resource> constant_buffer{};

    void* constant_buffer_mapped{};

    bool Init(ID3D12Device* device) {
        // creating constant buffer
        D3D12_HEAP_PROPERTIES heap_properties{};
        heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heap_properties.CreationNodeMask = 0;
        heap_properties.VisibleNodeMask = 0;

        D3D12_RESOURCE_DESC  res_desc{};
        res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        res_desc.Alignment = 0;
        res_desc.Width = (sizeof(TransformData) + 255) & ~255;
        res_desc.Height = 1;
        res_desc.DepthOrArraySize = 1;
        res_desc.MipLevels = 1;
        res_desc.Format = DXGI_FORMAT_UNKNOWN;
        res_desc.SampleDesc = { 1, 0 };
        res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        HR_CHECK(device->CreateCommittedResource(
            &heap_properties, D3D12_HEAP_FLAG_NONE, &res_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constant_buffer)
        ));

        constant_buffer->Map(0, nullptr, &constant_buffer_mapped);
    }

    void UpdateConstantBuffer(const DirectX::XMMATRIX& view_proj) {
        memcpy(constant_buffer_mapped, &view_proj, sizeof(TransformData));
    }
};