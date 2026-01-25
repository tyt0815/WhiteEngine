#include "PhysicsUserData.h"

namespace Physics
{
	extern UINT64 g_UpdateCount;

	FUserDataManager::FUserDataManager() {}
	FUserDataManager::~FUserDataManager() {}

	UINT64 FUserDataManager::CreateUserData_Internal(const TWeakPtr<WPhysicsComponent>& Comp)
	{
		TUniquePtr<FUserData> Data = MakeUnique<FUserData>();
		Data->Comp = Comp;
		Data->ID = mUserDatas.size();
		mUserDatas.emplace_back(std::move(Data));
		return mUserDatas.back()->ID;
	}

	void FUserDataManager::RemoveUserData_Internal()
	{
		while (!mRemoveQueue.empty() && mRemoveQueue.front().GenerationNumber > g_UpdateCount + 1)
		{
			UINT64 ID = mRemoveQueue.front().ID;
			if (ID < mUserDatas.size() - 1)
			{
				mUserDatas[ID] = std::move(mUserDatas.back());
				mUserDatas[ID]->ID = ID;
			}
			mUserDatas.pop_back();
			mRemoveQueue.pop();
		}
	}
	void FUserDataManager::EnqueueRemoveQ_Internal(FUserData* UserData)
	{
		if (UserData == nullptr)
		{
			return;
		}
		FRemoveQueueData Data;
		Data.ID = UserData->ID;
		Data.GenerationNumber = g_UpdateCount;
		mRemoveQueue.push(Data);
	}
}