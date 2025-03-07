//***************************************************************************************
// d3dUtil.h by Frank Luna (C) 2015 All Rights Reserved.
//
// General helper code.
//***************************************************************************************

#pragma once

#include <dxgi1_4.h>
#include <string>
#include <wrl.h>
#include "d3dx12.h"

extern const int NumFrameResources;

class FDXUtility
{
public:
    static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);
    static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* cmdList,
        const void* initData,
        UINT64 byteSize,
        Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);
	static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);
    inline static UINT CalcConstantBufferByteSize(UINT byteSize)
    {
        // Constant buffers must be a multiple of the minimum hardware
        // allocation size (usually 256 bytes).  So round up to nearest
        // multiple of 256.  We do this by adding 255 and then masking off
        // the lower 2 bytes which store all bits < 256.
        // Example: Suppose byteSize = 300.
        // (300 + 255) & ~255
        // 555 & ~255
        // 0x022B & ~0x00ff
        // 0x022B & 0xff00
        // 0x0200
        // 512
        return (byteSize + 255) & ~255;
    }
    inline static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleByIndex(
        int i,
        ID3D12DescriptorHeap* DescriptorHeap,
        UINT DescriptorSize
    )
    {
        D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle = DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        DescriptorHandle.ptr += i * DescriptorSize;
        return DescriptorHandle;
    }
    template<typename T>
    inline void SetDebugName(T* Object, const char* Name)
    {
        if (Object)
        {
            Object->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(Name), Name);
        }
    }
    inline void SetDebugNameOfDXGIObject(IDXGIObject* obj, const char* name)
    {
        SetDebugName(obj, name);
    }
    inline void SetDebugNameOfDevice(ID3D12Device* obj, const char* name)
    {
        SetDebugName(obj, name);
    }
    inline void SetDebugNameOfDeviceChild(ID3D12DeviceChild* obj, const char* name)
    {
        SetDebugName(obj, name);
    }
};