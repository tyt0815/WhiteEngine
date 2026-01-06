#include "GausiaanBlurFilter.h"
#include <assert.h>
#include "DirectX/d3dx12.h"
#include "DirectX/DXUtility.h"
#include "DirectX/DXException.h"

FGaussianBlurFilter::FGaussianBlurFilter(ID3D12Device* Device, UINT Width, UINT Height)
{
	mBlurMap0 = std::make_unique<FUnorderedAccessTexture2D>(Device, Width, Height);
	mBlurMap1 = std::make_unique<FUnorderedAccessTexture2D>(Device, Width, Height);

	BuildRootSignature();
	BuildShaders();
	BuildPipelineStates(Device);

	UpdateConstantBuffers(2.5);
}

void FGaussianBlurFilter::Execute(ID3D12GraphicsCommandList* CommandList, FResource* InputTexture, int BlurCount)
{
	CommandList->SetComputeRootSignature(mRootSignature.Get());
	CommandList->SetComputeRoot32BitConstants(0, 1, &mBlurRadius, 0);
	CommandList->SetComputeRoot32BitConstants(0, (UINT)mWeights.size(), mWeights.data(), 1);
	InputTexture->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
	mBlurMap0->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_COPY_DEST);
	CommandList->CopyResource(mBlurMap0->Get(), InputTexture->Get());


	for (int i = 0; i < BlurCount; ++i)
	{
		mBlurMap0->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_GENERIC_READ);
		mBlurMap1->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		CommandList->SetPipelineState(mHorizontalPipelineState.Get());
		CommandList->SetComputeRootDescriptorTable(1, mBlurMap0->GetSRV());
		CommandList->SetComputeRootDescriptorTable(2, mBlurMap1->GetSRV());

		UINT NumGroupsX = (UINT)ceilf(mBlurMap1->GetWidth() / 256.0f);
		CommandList->Dispatch(NumGroupsX, static_cast<UINT>(mBlurMap1->GetHeight()), 1);

		mBlurMap0->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		mBlurMap1->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_GENERIC_READ);

		CommandList->SetPipelineState(mVerticalPipelineState.Get());
		CommandList->SetComputeRootDescriptorTable(1, mBlurMap1->GetSRV());
		CommandList->SetComputeRootDescriptorTable(2, mBlurMap0->GetSRV());

		UINT NumGroupsY = (UINT)ceilf(mBlurMap0->GetHeight() / 256.0f);
		CommandList->Dispatch(static_cast<UINT>(mBlurMap0->GetWidth()), NumGroupsY, 1);
	}

	InputTexture->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_COPY_DEST);
	CommandList->CopyResource(InputTexture->Get(), mBlurMap0->Get());
}

void FGaussianBlurFilter::UpdateConstantBuffers(float Sigma)
{
	float TwoSigma2 = 2.0f * Sigma * Sigma;

	mBlurRadius = (int)ceil(2.0f * Sigma);

	assert(mBlurRadius <= BLUR_RADIUS_MAX);

	mWeights.resize(2 * mBlurRadius + 1);

	float WeightSum = 0.0f;
	for (int i = -mBlurRadius; i <= mBlurRadius; ++i)
	{
		float x = (float)i;
		mWeights[i + mBlurRadius] = expf(-x * x / TwoSigma2);
		WeightSum += mWeights[i + mBlurRadius];
	}
	for (float& Weight : mWeights)
	{
		Weight /= WeightSum;
	}
}

void FGaussianBlurFilter::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE InputTable;
	InputTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	CD3DX12_DESCRIPTOR_RANGE OutputTable;
	OutputTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_ROOT_PARAMETER RootParameters[3];
	RootParameters[0].InitAsConstants(12, 0);
	RootParameters[1].InitAsDescriptorTable(1, &InputTable);
	RootParameters[2].InitAsDescriptorTable(1, &OutputTable);

	FDXUtility::BuildRootSignature(RootParameters, _countof(RootParameters), mRootSignature.GetAddressOf());
}

void FGaussianBlurFilter::BuildShaders()
{
	mHorizontalComputeShader = FDXUtility::CompileShader(
		L"Shaders\\HorizontalGaussianBlurComputeShader.hlsl",
		nullptr,
		"MainCS",
		"cs_5_1"
	);
	mVerticalComputeShader = FDXUtility::CompileShader(
		L"Shaders\\VerticalGaussianBlurComputeShader.hlsl",
		nullptr,
		"MainCS",
		"cs_5_1"
	);
}

void FGaussianBlurFilter::BuildPipelineStates(ID3D12Device* Device)
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC HorizontalGaussianBlurPipelineStateDesc;
	ZeroMemory(&HorizontalGaussianBlurPipelineStateDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
	HorizontalGaussianBlurPipelineStateDesc.pRootSignature = mRootSignature.Get();
	HorizontalGaussianBlurPipelineStateDesc.CS =
	{
		reinterpret_cast<BYTE*>(mHorizontalComputeShader->GetBufferPointer()),
		mHorizontalComputeShader->GetBufferSize()
	};
	HorizontalGaussianBlurPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	THROW_IF_FAILED(
		Device->CreateComputePipelineState(
			&HorizontalGaussianBlurPipelineStateDesc,
			IID_PPV_ARGS(mHorizontalPipelineState.GetAddressOf())
		)
	);

	D3D12_COMPUTE_PIPELINE_STATE_DESC VerticalGaussianBlurPipelineStateDesc = HorizontalGaussianBlurPipelineStateDesc;
	VerticalGaussianBlurPipelineStateDesc.CS =
	{
		reinterpret_cast<BYTE*>(mVerticalComputeShader->GetBufferPointer()),
		mVerticalComputeShader->GetBufferSize()
	};

	THROW_IF_FAILED(
		Device->CreateComputePipelineState(
			&VerticalGaussianBlurPipelineStateDesc,
			IID_PPV_ARGS(mVerticalPipelineState.GetAddressOf())
		)
	);
}
