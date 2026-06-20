#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include "directxmath.h"
#include "d3dcompiler.h"
#pragma comment(lib, "d3dcompiler.lib")

class Mesh {
public:
	ComPtr<ID3D12Resource> vertex_buffer{};
	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view{};

	ComPtr<ID3D12Resource> index_buffer{};
	D3D12_INDEX_BUFFER_VIEW index_buffer_view{};

	uint32_t index_count = 0;
	uint32_t vertex_count = 0;

	D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

template<size_t VertexCount, size_t IndexCount>
bool CreateMesh(
    ID3D12Device* device,
    Mesh* mesh,
    const Vertex(&vertices)[VertexCount],
    const UINT16(&indices)[IndexCount]
) {
    // vertex buffer
    {
        mesh->vertex_count = static_cast<uint32_t>(VertexCount);

        D3D12_HEAP_PROPERTIES heap_properties{};
        heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heap_properties.CreationNodeMask = 0;
        heap_properties.VisibleNodeMask = 0;

        D3D12_RESOURCE_DESC  res_desc{};
        res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        res_desc.Alignment = 0;
        res_desc.Width = sizeof(vertices);
        res_desc.Height = 1;
        res_desc.DepthOrArraySize = 1;
        res_desc.MipLevels = 1;
        res_desc.Format = DXGI_FORMAT_UNKNOWN;
        res_desc.SampleDesc = { 1, 0 };
        res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        HR_CHECK(device->CreateCommittedResource(
            &heap_properties, D3D12_HEAP_FLAG_NONE, &res_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mesh->vertex_buffer)
        ));

        void* data;
        mesh->vertex_buffer->Map(0, nullptr, &data);
        memcpy(data, vertices, sizeof(vertices));
        mesh->vertex_buffer->Unmap(0, nullptr);

        mesh->vertex_buffer_view.BufferLocation = mesh->vertex_buffer->GetGPUVirtualAddress();
        mesh->vertex_buffer_view.SizeInBytes = sizeof(vertices);
        mesh->vertex_buffer_view.StrideInBytes = sizeof(Vertex);
    }

    // index buffer
    {
        mesh->index_count = static_cast<uint32_t>(IndexCount);

        D3D12_HEAP_PROPERTIES heap_properties{};
        heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heap_properties.CreationNodeMask = 0;
        heap_properties.VisibleNodeMask = 0;

        D3D12_RESOURCE_DESC  res_desc{};
        res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        res_desc.Alignment = 0;
        res_desc.Width = sizeof(indices);
        res_desc.Height = 1;
        res_desc.DepthOrArraySize = 1;
        res_desc.MipLevels = 1;
        res_desc.Format = DXGI_FORMAT_UNKNOWN;
        res_desc.SampleDesc = { 1, 0 };
        res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        HR_CHECK(device->CreateCommittedResource(
            &heap_properties, D3D12_HEAP_FLAG_NONE,
            &res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mesh->index_buffer)
        ));

        void* data;
        mesh->index_buffer->Map(0, nullptr, &data);
        memcpy(data, indices, sizeof(indices));
        mesh->index_buffer->Unmap(0, nullptr);

        mesh->index_buffer_view.BufferLocation = mesh->index_buffer->GetGPUVirtualAddress();
        mesh->index_buffer_view.Format = DXGI_FORMAT_R16_UINT;
        mesh->index_buffer_view.SizeInBytes = sizeof(indices);
    }

    return true;
}

static inline bool CreateCameraMesh(ID3D12Device* device, Mesh* mesh, float fov)
{
    const float rayLength = 0.7f;

    float halfHeight = tanf(fov * 0.5f);
    float halfWidth = halfHeight;

    DirectX::XMFLOAT3 corners[4] =
    {
        { -halfWidth, -halfHeight, 1.0f },
        {  halfWidth, -halfHeight, 1.0f },
        {  halfWidth,  halfHeight, 1.0f },
        { -halfWidth,  halfHeight, 1.0f }
    };

    for (auto& c : corners)
    {
        float len = sqrtf(c.x * c.x + c.y * c.y + c.z * c.z);

        c.x *= rayLength / len;
        c.y *= rayLength / len;
        c.z *= rayLength / len;
    }

    Vertex vertices[] =
    {
        {{0.0f, 0.0f, 0.0f}, {1,1,1,1}},

        {{corners[0].x, corners[0].y, corners[0].z}, {0,1,0,1}},
        {{corners[1].x, corners[1].y, corners[1].z}, {0,1,0,1}},
        {{corners[2].x, corners[2].y, corners[2].z}, {0,1,0,1}},
        {{corners[3].x, corners[3].y, corners[3].z}, {0,1,0,1}},
    };

    uint16_t indices[] =
    {
        0, 1,
        0, 2,
        0, 3,
        0, 4,

        1, 2,
        2, 3,
        3, 4,
        4, 1
    };

    mesh->topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    mesh->vertex_count = _countof(vertices);
    mesh->index_count = _countof(indices);

    return CreateMesh(device, mesh, vertices, indices);
}
