#include "Trace.h"
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include "JPHUtility.h"

using namespace JPH;

namespace Physics
{
	const NarrowPhaseQuery& GetNarrowPhaseQuery();

	BodyInterface& GetBodyInterface();

	const BodyLockInterface& GetBodyLockInterface();

	void LineTrace(XMFLOAT3 Start, XMFLOAT3 End, FHitResult& HitResult)
	{
		HitResult = {};
		RVec3 Origin = ToJPHPosition(Start);
		RVec3 Direction = ToJPHPosition(End) - Origin;
		RRayCast Ray { Origin, Direction };

		// 단일 타격만 원할 경우:
		RayCastResult Result;

		// 3. 쿼리 실행 (NarrowPhaseQuery 사용)
		const NarrowPhaseQuery& Query = GetNarrowPhaseQuery();
		if (Query.CastRay(Ray, Result))
		{
			// result.mBodyID를 통해 충돌한 바디 정보를 가져옴
			BodyLockRead Lock(GetBodyLockInterface(), Result.mBodyID);
			if (Lock.Succeeded())
			{
				const Body& HitBody = Lock.GetBody();

				HitResult.HitComponent = *reinterpret_cast<TWeakPtr<WPhysicsComponent>*>(HitBody.GetUserData());
			}
		}
	}
}