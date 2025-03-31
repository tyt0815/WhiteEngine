#pragma once

#include <d3d12.h>
#include <vector>

std::vector<D3D12_INPUT_ELEMENT_DESC> GetDrawingSphereInputLayouts();
void DrawSphere(ID3D12GraphicsCommandList* CommandList);

std::vector<D3D12_INPUT_ELEMENT_DESC> GetDrawingRectInputLayouts();
void DrawRect(ID3D12GraphicsCommandList* CommandList);