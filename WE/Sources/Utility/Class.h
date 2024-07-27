#pragma once

#define SINGLETON(type) public: static type* GetInstance()\
	{\
		static type mgr;\
		return &mgr;\
	}\
private: type();\
private: ~type();

class FNoncopyable
{
protected:
	// ensure the class cannot be constructed directly
	FNoncopyable() {}
	// the class should not be used polymorphically
	~FNoncopyable() {}
private:
	FNoncopyable(const FNoncopyable&);
	FNoncopyable& operator=(const FNoncopyable&) { return *this; }
};