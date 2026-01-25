#pragma once

#include "Utility/Class.h"

#include "GameFramework/Object/Component/PhysicsComponent.h"
#include "Utility/Memory.h"
#include "Utility/Container.h"
#include <queue>

using UINT64 = unsigned long long;

namespace Physics
{
	struct FUserData
	{
		TWeakPtr<WPhysicsComponent> Comp;

	private:
		UINT64 ID;

		friend class FUserDataManager;
	};

	class FUserDataManager
	{
		SINGLETON(FUserDataManager);
	private:
		struct FRemoveQueueData
		{
			UINT64 ID;
			UINT64 GenerationNumber;
		};
	
		UINT64 CreateUserData_Internal(const TWeakPtr<WPhysicsComponent>& Comp);

		void RemoveUserData_Internal();

		void EnqueueRemoveQ_Internal(FUserData* UserData);

		TArray<TUniquePtr<FUserData>> mUserDatas;

		std::queue<FRemoveQueueData> mRemoveQueue;

	
		__forceinline FUserData* GetUserData_Internal(UINT64 ID) const
		{
			return mUserDatas[ID].get();
		}
	public:
		static __forceinline FUserData* GetUserData(UINT64 ID) 
		{
			return GetInstance()->GetUserData_Internal(ID);
		}

		static __forceinline UINT64 CreateUserData(const TWeakPtr<WPhysicsComponent>& UserData)
		{
			return GetInstance()->CreateUserData_Internal(UserData);
		}

		static __forceinline void RemoveUserData()
		{
			GetInstance()->RemoveUserData_Internal();
		}

		static __forceinline void EnqueueRemoveQ(FUserData* UserData)
		{
			GetInstance()->EnqueueRemoveQ_Internal(UserData);
		}
	};


}