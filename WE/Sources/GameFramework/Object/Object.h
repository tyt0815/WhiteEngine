#pragma once
#include "Utility/Memory.h"
#include "Asset/BlueprintAsset.h"
#include <functional>

extern class WWorld* g_World;

enum class ETickGroup : unsigned int
{
	ETG_PrePhysics = 0,
	ETG_PostPhysics,
	ETG_None
};

enum class ETickPriority : unsigned int
{
	ETP_High = 0,
	ETP_Middle,
	ETP_Low,
	ETP_None
};

class WObject : public std::enable_shared_from_this<WObject>
{
public:
	WObject();

	virtual void Tick(float DeltaSecond);

	void SetTickGroup(ETickGroup TickGroup, ETickPriority TickPriority);

	virtual void Destroy() = 0;

	virtual void Activate() = 0;

	virtual void Deactivate() = 0;

	template<typename T>
	T GetWProperty(const std::string& Name);

	template <typename T>
	T* GetWPropertyPtr(const std::string& Name);

	template <typename T>
	T* GetWPropertyPtrSafe(const std::string& Name);

	template <typename T>
	void SetWProperty(const std::string& Name, const T& Value);

	template <typename T>
	void RegisterWPropertySafe(const std::string& Name, T* ValuePtr);

protected:
	struct WFunctionParameter
	{
		std::string Name;
		TSharedPtr<void> Value;

		template<typename T>
		T Get() const
		{
			if (!Value)
			{
				assert(false);
				return T();
			}

			TSharedPtr<T> TypedPtr = std::static_pointer_cast<T>(Value);

			if (!TypedPtr)
			{
				assert(false && "Invalid Type Cast in WFunctionParameter::Get");
				return T();
			}

			return *TypedPtr;
		}
	};

	using WFunctionParams = TArray<TSharedPtr<WFunctionParameter>>;
	using WFunctionReturn = TSharedPtr<void>;
	using WFunction = std::function<WFunctionReturn(WFunctionParams)>;

	// 매크로에서 사용되는 헬퍼 함수.
	template<typename T>
	inline T GetWParam(const WFunctionParams& Params, const std::string& Name) {
		auto it = std::find_if(Params.begin(), Params.end(), [&](const TSharedPtr<WFunctionParameter>& P) {
			return P && P->Name == Name;
			});

		if (it != Params.end()) {
			return (*it)->Get<T>();
		}
		return T();
	}

	class WEvent
	{
	public:
		void LoadEvent(WObject* Context, const BlueprintAsset::FEventNode& Event);

		WFunctionReturn CallFunction(WObject* Context, const BlueprintAsset::FFunctionNode* FuncNode);

		void Dispatch() const;

	private:
		TArray<std::function<void()>> mFunctions;
	};

	virtual void OnDestroy();

	virtual void OnActivate();

	virtual void OnDeactivate();

	void LoadWProperties(const TArray<BlueprintAsset::FProperty>& Properties);

	void LoadWVariables(const TArray<BlueprintAsset::FProperty>& Variables);

	void LoadEvents(WObject* Context, const TArray<BlueprintAsset::FEventNode>& Events);

	template <typename T>
	void RegisterWProperty(const std::string& Name, T* ValuePtr);

	void RegisterWFunction(const std::string& Name, WFunction Func);

private:
	ETickGroup mTickGroup = ETickGroup::ETG_None;

	ETickPriority mTickPriority = ETickPriority::ETP_None;

	std::unordered_map<std::string, void*> mBlueprintPropertiesMap;

	std::vector<TSharedPtr<void>> mWVariables;

	std::unordered_map<std::string, WFunction> mWFunctions;

	std::unordered_map<std::string, WEvent> mWEvents;

	const WEvent* mTickEvent;

	int mTickId = -1;

public:
	__forceinline const WEvent* RegisterEvent(const std::string& Name)
	{
		return &mWEvents[Name];
	}

	template<typename T>
	__forceinline TWeakPtr<T> GetWeakPtr()
	{
		return Cast<T>(shared_from_this());
	}

	__forceinline WWorld* GetWorld() const
	{
		return g_World;
	}


	friend class WWorld;
};

template<typename T>
inline void WObject::RegisterWProperty(const std::string& Name, T* ValuePtr)
{
	if (mBlueprintPropertiesMap.count(Name) > 0)
	{
		assert(mBlueprintPropertiesMap[Name] != ValuePtr && "Alreay registered Name");
	}

	mBlueprintPropertiesMap[Name] = ValuePtr;
}

template<typename T>
inline T WObject::GetWProperty(const std::string& Name)
{
	return *GetWPropertyPtr<T>(Name);
}

template<typename T>
inline T* WObject::GetWPropertyPtr(const std::string& Name)
{
	assert(mBlueprintPropertiesMap.count(Name) > 0 && "Property not found.");
	return static_cast<T*>(mBlueprintPropertiesMap.at(Name));
}

template<typename T>
inline T* WObject::GetWPropertyPtrSafe(const std::string& Name)
{
	return mBlueprintPropertiesMap.count(Name) > 0 ? GetWPropertyPtr<T>(Name) : nullptr;
}

template<typename T>
inline void WObject::SetWProperty(const std::string& Name, const T& Value)
{
	T* Ref = GetWPropertyPtr<T>(Name);
	*Ref = Value;
}

template<typename T>
inline void WObject::RegisterWPropertySafe(const std::string& Name, T* ValuePtr)
{
	if (mBlueprintPropertiesMap.count(Name) == 0)
	{
		RegisterWProperty<T>(Name, ValuePtr);
	}
	else
	{
		mBlueprintPropertiesMap[Name] = ValuePtr;
	}
}

#define BEGIN_WFUNCTION(Name) RegisterWFunction(#Name, [this](const WFunctionParams& Params)
#define END_WFUNCTION );

#define _GET_WP(Type, Name) Type Name = GetWParam<Type>(Params, #Name)

// 0개
#define REGISTER_WFUNC_0(FuncName) \
    RegisterWFunction(#FuncName, [this](const WFunctionParams& Params) { \
        this->FuncName(); return nullptr; \
    })

// 1개
#define REGISTER_WFUNC_1(FuncName, P1N, P1T) \
    RegisterWFunction(#FuncName, [this](const WFunctionParams& Params) { \
        _GET_WP(P1T, P1N); \
        this->FuncName(P1N); return nullptr; \
    })

// 2개
#define REGISTER_WFUNC_2(FuncName, P1N, P1T, P2N, P2T) \
    RegisterWFunction(#FuncName, [this](const WFunctionParams& Params) { \
        _GET_WP(P1T, P1N); _GET_WP(P2T, P2N); \
        this->FuncName(P1N, P2N); return nullptr; \
    })

// 3개
#define REGISTER_WFUNC_3(FuncName, P1N, P1T, P2N, P2T, P3N, P3T) \
    RegisterWFunction(#FuncName, [this](const WFunctionParams& Params) { \
        _GET_WP(P1T, P1N); _GET_WP(P2T, P2N); _GET_WP(P3T, P3N); \
        this->FuncName(P1N, P2N, P3N); return nullptr; \
    })

// 4개
#define REGISTER_WFUNC_4(FuncName, P1N, P1T, P2N, P2T, P3N, P3T, P4N, P4T) \
    RegisterWFunction(#FuncName, [this](const WFunctionParams& Params) { \
        _GET_WP(P1T, P1N); _GET_WP(P2T, P2N); _GET_WP(P3T, P3N); _GET_WP(P4T, P4N); \
        this->FuncName(P1N, P2N, P3N, P4N); return nullptr; \
    })

// 5개
#define REGISTER_WFUNC_5(FuncName, P1N, P1T, P2N, P2T, P3N, P3T, P4N, P4T, P5N, P5T) \
    RegisterWFunction(#FuncName, [this](const WFunctionParams& Params) { \
        _GET_WP(P1T, P1N); _GET_WP(P2T, P2N); _GET_WP(P3T, P3N); _GET_WP(P4T, P4N); _GET_WP(P5T, P5N); \
        this->FuncName(P1N, P2N, P3N, P4N, P5N); return nullptr; \
    })


// 0개 + Ret
#define REGISTER_WFUNC_RET_0(FuncName, RT) \
    RegisterWFunction(#FuncName, [this](const WFunctionParams& Params) { \
        return MakeShared<RT>(this->FuncName()); \
    })

// 1개 + Ret
#define REGISTER_WFUNC_RET_1(FuncName, P1N, P1T, RT) \
    RegisterWFunction(#FuncName, [this](const WFunctionParams& Params) { \
        _GET_WP(P1T, P1N); \
        return MakeShared<RT>(this->FuncName(P1N)); \
    })

// 2개 + Ret
#define REGISTER_WFUNC_RET_2(FuncName, P1N, P1T, P2N, P2T, RT) \
    RegisterWFunction(#FuncName, [this](const WFunctionParams& Params) { \
        _GET_WP(P1T, P1N); _GET_WP(P2T, P2N); \
        return MakeShared<RT>(this->FuncName(P1N, P2N)); \
    })

// 3개 + Ret
#define REGISTER_WFUNC_RET_3(FuncName, P1N, P1T, P2N, P2T, P3N, P3T, RT) \
    RegisterWFunction(#FuncName, [this](const WFunctionParams& Params) { \
        _GET_WP(P1T, P1N); _GET_WP(P2T, P2N); _GET_WP(P3T, P3N); \
        return MakeShared<RT>(this->FuncName(P1N, P2N, P3N)); \
    })

// 4개 + Ret
#define REGISTER_WFUNC_RET_4(FuncName, P1N, P1T, P2N, P2T, P3N, P3T, P4N, P4T, RT) \
    RegisterWFunction(#FuncName, [this](const WFunctionParams& Params) { \
        _GET_WP(P1T, P1N); _GET_WP(P2T, P2N); _GET_WP(P3T, P3N); _GET_WP(P4T, P4N); \
        return MakeShared<RT>(this->FuncName(P1N, P2N, P3N, P4N)); \
    })

// 5개 + Ret
#define REGISTER_WFUNC_RET_5(FuncName, P1N, P1T, P2N, P2T, P3N, P3T, P4N, P4T, P5N, P5T, RT) \
    RegisterWFunction(#FuncName, [this](const WFunctionParams& Params) { \
        _GET_WP(P1T, P1N); _GET_WP(P2T, P2N); _GET_WP(P3T, P3N); _GET_WP(P4T, P4N); _GET_WP(P5T, P5N); \
        return MakeShared<RT>(this->FuncName(P1N, P2N, P3N, P4N, P5N)); \
    })