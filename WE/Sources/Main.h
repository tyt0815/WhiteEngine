#pragma once
#include "Common/Class.h"

class UTimer;

using namespace std;

class FMain
{
	SINGLETON(FMain)
public:
	int Main();

private:

	virtual void MakeWorld() {}

	UTimer* MainTimer = nullptr;

};