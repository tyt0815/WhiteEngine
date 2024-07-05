#pragma once

#define SINGLETON(type) public: static type* GetInstance()\
	{\
		static type mgr;\
		return &mgr;\
	}\
private: type(){}\
private: ~type(){}
