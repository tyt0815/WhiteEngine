#pragma once

#include "PrimitiveComponent.h"
#include <vector>

class FSkeletalMeshAsset;

class WSkeletalMeshComponent : public WPrimitiveComponent
{
	using Super = WPrimitiveComponent;

public:
	WSkeletalMeshComponent();
	virtual ~WSkeletalMeshComponent() override {}

	// 부모 가상 함수 오버라이드
	virtual void UpdateConstantBufferIndex() override;
	virtual void UpdateProxies() override;

	void SetSkeletalMesh(const std::string& AssetName);

	// 애니메이션 업데이트 (애니메이션 에셋 연동 전 임시 테스트용)
	void UpdateAnimation(float DeltaTime);

private:
	FSkeletalMeshAsset* mSkeletalMeshAsset = nullptr;

	// 스킨닝 행렬을 저장할 컨스턴트 버퍼 프록시 인덱스
	uint32_t mBoneCBIndex = -1;

	// 매 프레임 계산된 최종 본 행렬 리스트 (Local -> World(Bone) -> Root)
	// Shader의 스킨닝 연산에 사용됨: FinalMatrix = InverseBindPose * AnimMatrix
	std::vector<XMFLOAT4X4> mFinalTransforms;
};