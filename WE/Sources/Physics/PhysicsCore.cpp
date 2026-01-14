#ifndef JPH_DEBUG_RENDERER
#define JPH_DEBUG_RENDERER
#endif

#include "PhysicsCore.h"

#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include "JPHUtility.h"
#include "PhysicsDebugRenderer.h"

#include <windows.h>
#include <cstdarg>
#include <sstream>
#include <cassert>



using namespace JPH;
using namespace JPH::literals;

// An example contact listener
class MyContactListener : public ContactListener
{
public:
	// See: ContactListener
	virtual ValidateResult	OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult) override
	{
		OutputDebugStringA("Contact validate callback\n");

		// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
		return ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	virtual void			OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
	{
		OutputDebugStringA("A contact was added\n");
	}

	virtual void			OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
	{
		OutputDebugStringA("A contact was persisted\n");
	}

	virtual void			OnContactRemoved(const SubShapeIDPair& inSubShapePair) override
	{
		OutputDebugStringA("A contact was removed\n");
	}
};

// An example activation listener
class MyBodyActivationListener : public BodyActivationListener
{
public:
	virtual void		OnBodyActivated(const BodyID& inBodyID, uint64 inBodyUserData) override
	{
		OutputDebugStringA("A body got activated\n");
	}

	virtual void		OnBodyDeactivated(const BodyID& inBodyID, uint64 inBodyUserData) override
	{
		OutputDebugStringA("A body went to sleep\n");
	}
};

namespace Physics
{
	std::unique_ptr<PhysicsSystem> g_PhysicsSystem;

	std::unique_ptr<TempAllocatorImpl> g_TempAllocator;

	std::unique_ptr<JobSystemThreadPool> g_JobSystem;

	std::unique_ptr<FBroadPhaseLayerInterface> g_BroadPhaseLayerInterface;

	std::unique_ptr<FObjectVsBroadPhaseLayerFilter> g_ObjectVsBroadPhaseLayerFilter;

	std::unique_ptr<FObjectLayerPairFilter> g_ObjectLayerPairFilter;

	std::unique_ptr<MyBodyActivationListener> g_BodyActivationListener;

	std::unique_ptr<MyContactListener> g_ContactListener;
}

namespace Physics
{
	// Callback for traces, connect this to your own trace function if you have one
	static void TraceImpl(const char* inFMT, ...)
	{
		// Format the message
		va_list list;
		va_start(list, inFMT);
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), inFMT, list);
		va_end(list);

		OutputDebugStringA(buffer);
	}

	// Callback for asserts, connect this to your own assert handler if you have one
	static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint inLine)
	{
		// Print to the TTY
		std::string DebugStr;
		DebugStr += inFile;
		DebugStr += ":";
		DebugStr += inLine;
		DebugStr += ": (";
		DebugStr += inExpression; DebugStr += ") ";
		DebugStr += (inMessage != nullptr ? inMessage : "");
		DebugStr += '\n';
		OutputDebugStringA(DebugStr.c_str());

		// Breakpoint
		return true;
	};

	inline BodyInterface* GetBodyInterface()
	{
		return &g_PhysicsSystem->GetBodyInterface();
	}
}

// 함수 본문
inline EMotionType ToMotionType(EObjectType ObjectType) 
{
	return static_cast<EMotionType>(ObjectType);
}

FBody::FBody(const BodyCreationSettings& Settings)
{
	mBody = Physics::GetBodyInterface()->CreateBody(Settings);
}

FBody::~FBody()
{
	RemoveBody();
	Physics::GetBodyInterface()->DestroyBody(mBody->GetID());
}

void FBody::AddBody(bool bActivate)
{
	Physics::GetBodyInterface()->AddBody(mBody->GetID(), bActivate ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
}

void FBody::RemoveBody()
{
	Physics::GetBodyInterface()->RemoveBody(mBody->GetID());
}

void FBody::SetPosition(XMFLOAT3 Position)
{
	Physics::GetBodyInterface()->SetPosition(mBody->GetID(), ToJPHPosition(Position), EActivation::Activate);
}

void FBody::SetMotiontype(EObjectType ObjectType)
{
	Physics::GetBodyInterface()->SetMotionType(mBody->GetID(), ToMotionType(ObjectType), JPH::EActivation::Activate);
}

void FBody::SetActivate(bool bActivate)
{
	bActivate ?
		Physics::GetBodyInterface()->ActivateBody(mBody->GetID()) :
		Physics::GetBodyInterface()->DeactivateBody(mBody->GetID());
}

FTransform FBody::GetTransform() const
{
	RVec3 Location = Physics::GetBodyInterface()->GetCenterOfMassPosition(mBody->GetID());
	JPH::Quat QuatRotation = Physics::GetBodyInterface()->GetRotation(mBody->GetID());

	FTransform Transform = FTransform::Default;
	Transform.Translation = ToDXLocation(Location);
	Transform.SetRotationByQuat(ToDXQuatRotation(QuatRotation));

	return Transform;
}

void FBody::SetTransform(const FTransform& Transform)
{
	RVec3 Pos = ToJPHPosition(Transform.Translation);
	Quat Quat = TOJPHQuatRotation(Transform.GetQuaternionRotationFloat4());

	BodyInterface* BI = Physics::GetBodyInterface();

	JPH::EActivation Activation = BI->IsActive(mBody->GetID()) ? EActivation::Activate : EActivation::DontActivate;
	Physics::GetBodyInterface()->SetPositionAndRotation(mBody->GetID(), Pos, Quat, Activation);
}

void Physics::Startup()
{
	// Register allocation hook. In this example we'll just let Jolt use malloc / free but you can override these if you want (see Memory.h).
	// This needs to be done before any other Jolt function is called.
	RegisterDefaultAllocator();

	// Install trace and assert callbacks
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

	// Create a factory, this class is responsible for creating instances of classes based on their name or hash and is mainly used for deserialization of saved data.
	// It is not directly used in this example but still required.
	Factory::sInstance = new Factory();

	// Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
	// If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
	// If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
	RegisterTypes();

	// We need a temp allocator for temporary allocations during the physics update. We're
	// pre-allocating 10 MB to avoid having to do allocations during the physics update.
	// B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
	// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
	// malloc / free.
	g_TempAllocator = std::make_unique<TempAllocatorImpl>(10 * 1024 * 1024);
	
	// We need a job system that will execute physics jobs on multiple threads. Typically
	// you would implement the JobSystem interface yourself and let Jolt Physics run on top
	// of your own job scheduler. JobSystemThreadPool is an example implementation.
	g_JobSystem = std::make_unique<JobSystemThreadPool>(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

	// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const uint cMaxBodies = 65536;

	// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
	const uint cNumBodyMutexes = 0;

	// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
	// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
	// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const uint cMaxBodyPairs = 65536;

	// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
	// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
	const uint cMaxContactConstraints = 10240;

	// Create mapping table from object layer to broadphase layer
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	// Also have a look at BroadPhaseLayerInterfaceTable or BroadPhaseLayerInterfaceMask for a simpler interface.
	g_BroadPhaseLayerInterface = std::make_unique<FBroadPhaseLayerInterface>();

	// Create class that filters object vs broadphase layers
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	// Also have a look at ObjectVsBroadPhaseLayerFilterTable or ObjectVsBroadPhaseLayerFilterMask for a simpler interface.
	g_ObjectVsBroadPhaseLayerFilter = std::make_unique<FObjectVsBroadPhaseLayerFilter>();

	// Create class that filters object vs object layers
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	// Also have a look at ObjectLayerPairFilterTable or ObjectLayerPairFilterMask for a simpler interface.
	g_ObjectLayerPairFilter = std::make_unique<FObjectLayerPairFilter>();

	// Now we can create the actual physics system.
	g_PhysicsSystem = std::make_unique<PhysicsSystem>();
	g_PhysicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, *g_BroadPhaseLayerInterface, *g_ObjectVsBroadPhaseLayerFilter, *g_ObjectLayerPairFilter);

	// A body activation listener gets notified when bodies activate and go to sleep
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	g_BodyActivationListener = std::make_unique<MyBodyActivationListener>();
	g_PhysicsSystem->SetBodyActivationListener(g_BodyActivationListener.get());

	// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	g_ContactListener = std::make_unique<MyContactListener>();
	g_PhysicsSystem->SetContactListener(g_ContactListener.get());

	// Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
	// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
	// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
	g_PhysicsSystem->OptimizeBroadPhase();

	g_DebugRenderer = std::make_unique<FPhysicsDebugRenderer>();
}

void Physics::Cleanup()
{
	// Unregisters all types with the factory and cleans up the default material
	UnregisterTypes();

	// Destroy the factory
	delete Factory::sInstance;
	Factory::sInstance = nullptr;
}

void Physics::Tick(float DeltaTime)
{
	constexpr float FixedTimeStep = 1.0f / 60.0f;
	static float AccumulatedTime = 0;
	AccumulatedTime += DeltaTime;

	for(int Step = 0; Step < 5 && AccumulatedTime >= FixedTimeStep; ++Step, AccumulatedTime -= FixedTimeStep)
	{
		g_PhysicsSystem->Update(FixedTimeStep, 1, g_TempAllocator.get(), g_JobSystem.get());
	}

	g_DebugRenderer->Clear();
	JPH::BodyManager::DrawSettings Settings;
	Settings.mDrawShape = true;
	//Settings.mDrawBoundingBox = true;

	g_PhysicsSystem->DrawBodies(Settings, g_DebugRenderer.get());
}

std::unique_ptr<FBody> CreateBody(ShapeSettings::ShapeResult ShapeResult, EObjectType ObjectType, JPH::ObjectLayer ObjectChannel)
{
	ShapeRefC SphereShape = ShapeResult.Get();
	BodyCreationSettings Settings(SphereShape, RVec3(0.0f, 0.0f, 0.0f), Quat::sIdentity(), ToMotionType(ObjectType), ObjectChannel);
	return std::move(std::make_unique<FBody>(Settings));
}

std::unique_ptr<FBody> CreateBoxBody(XMFLOAT3 Size, EObjectType ObjectType)
{
	BoxShapeSettings BoxSettings(RVec3(Size.x, Size.y, Size.z));
	BoxSettings.SetEmbedded();
	return std::move(CreateBody(BoxSettings.Create(), ObjectType, EObjectChannel::NON_MOVING));
}

std::unique_ptr<FBody> CreateSphereBody(float Radius, EObjectType ObjectType)
{
	SphereShapeSettings SphereSettings(Radius);
	SphereSettings.SetEmbedded();

	return std::move(CreateBody(SphereSettings.Create(), ObjectType, EObjectChannel::MOVING));
}