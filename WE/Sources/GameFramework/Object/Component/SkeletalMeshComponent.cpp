//#include "SkeletalMeshComponent.h"
//#include "Asset/AssetManager.h"
//#include "Asset/SkeletalMeshAsset.h"
//#include "GameFramework/Object/World/World.h"
//#include "GameFramework/RenderItemProxy.h"
//
//WSkeletalMeshComponent::WSkeletalMeshComponent()
//{
//}
//
//void WSkeletalMeshComponent::UpdateConstantBufferIndex()
//{
//	// 1. 기본 월드 행렬용 인덱스 할당 (WPrimitiveComponent)
//	Super::UpdateConstantBufferIndex();
//
//	if (mSkeletalMeshAsset == nullptr) return;
//
//	// 2. 스켈레탈 메시 전용 본 행렬(Skinning) 프록시 할당
//	// World 클래스에 AllocateBoneCBProxy()가 정의되어 있어야 합니다.
//	mBoneCBIndex = GetWorld()->AllocateBoneCBProxy();
//}
//
//void WSkeletalMeshComponent::UpdateProxies()
//{
//	// 1. 월드 행렬 업데이트 (Super 호출)
//	Super::UpdateProxies();
//
//	if (mSkeletalMeshAsset == nullptr || mBoneCBIndex == -1) return;
//
//	// 2. 본 행렬 데이터를 렌더 프록시에 복사
//	// 렌더러는 이 데이터를 받아 Constant Buffer에 Write합니다.
//	FBoneCBProxy* BoneProxy = GetWorld()->GetBoneCBProxy(mBoneCBIndex);
//	if (BoneProxy)
//	{
//		// 계산된 애니메이션 행렬들을 프록시 버퍼로 복사
//		size_t CopySize = std::min(mFinalTransforms.size(), (size_t)256); // 최대 본 개수 제한
//		std::memcpy(BoneProxy->BoneTransforms, mFinalTransforms.data(), sizeof(XMFLOAT4X4) * CopySize);
//	}
//
//	// 3. 메시 렌더링 정보 업데이트 (StaticMesh와 유사한 방식)
//	// 스켈레탈 전용 렌더 아이템 프록시가 있다면 여기서 설정합니다.
//	FSkeletalMeshProxy* MeshProxy = GetWorld()->GetSkeletalMeshProxy(mMeshCBIndex);
//	if (MeshProxy)
//	{
//		MeshProxy->BoneCBIndex = mBoneCBIndex;
//		// VertexBuffer, IndexBuffer 등 설정...
//	}
//}
//
//void WSkeletalMeshComponent::SetSkeletalMesh(const std::string& AssetName)
//{
//	if (mSkeletalMeshAsset = FAssetManager::GetAsset<FSkeletalMeshAsset>(AssetName))
//	{
//		// 에셋의 본 개수에 맞춰 행렬 배열 미리 할당
//		mFinalTransforms.resize(mSkeletalMeshAsset->GetSkeleton().size());
//
//		// 데이터가 바뀌었으므로 인덱스 다시 할당
//		UpdateConstantBufferIndex();
//	}
//}
//
//void WSkeletalMeshComponent::UpdateAnimation(float DeltaTime)
//{
//	if (!mSkeletalMeshAsset) return;
//
//	const auto& Skeleton = mSkeletalMeshAsset->GetSkeleton();
//
//	// TODO: 애니메이션 에셋으로부터 Keyframe을 읽어와서 mFinalTransforms 계산
//	// 지금은 테스트를 위해 Identity 행렬 혹은 기본 포즈(Inverse의 Inverse)를 넣을 수 있습니다.
//	for (size_t i = 0; i < Skeleton.size(); ++i)
//	{
//		// 실제 연산 시: Final = InverseBindPose * NodeAnimMatrix
//		// mFinalTransforms[i] = ...;
//	}
//}