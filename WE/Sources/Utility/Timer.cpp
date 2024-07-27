#include "Timer.h"
#include <Windows.h>

UTimer::UTimer()
{
	__int64 CountsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&CountsPerSec);
	SecondsPerCount = 1.0 / (double)CountsPerSec;
}

float UTimer::GetTotalTime() const
{
	// If we are stopped, do not count the time that has passed since we stopped.
	// Moreover, if we previously already had a pause, the distance 
	// StopTime - BaseTime includes paused time, which we do not want to count.
	// To correct this, we can subtract the paused time from StopTime:  
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  BaseTime       StopTime        startTime     StopTime    TickTime

	if (bStopped)
	{
		return (float)(((StopTime - PausedTime) - BaseTime) * SecondsPerCount);
	}

	// The distance TickTime - BaseTime includes paused time,
	// which we do not want to count.  To correct this, we can subtract 
	// the paused time from TickTime:  
	//
	//  (TickTime - PausedTime) - BaseTime 
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------> time
	//  BaseTime       StopTime        startTime     TickTime

	else
	{
		return (float)(((TickTime - PausedTime) - BaseTime) * SecondsPerCount);
	}
}

float UTimer::GetDeltaTime() const
{
	return (float)DeltaTime;
}

void UTimer::Reset()
{
	__int64 CurrentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&CurrentTime);

	BaseTime = CurrentTime;
	PrevTime = CurrentTime;
	StopTime = 0;
	bStopped = false;
}

void UTimer::Start()
{
	__int64 CurrentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&CurrentTime);


	// Accumulate the time elapsed between stop and start pairs.
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  BaseTime       StopTime        startTime     

	if (bStopped)
	{
		PausedTime += (CurrentTime - StopTime);

		PrevTime = CurrentTime;
		StopTime = 0;
		bStopped = false;
	}
}

void UTimer::Stop()
{
	if (!bStopped)
	{
		__int64 CurrentTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&CurrentTime);

		StopTime = CurrentTime;
		bStopped = true;
	}
}

void UTimer::Tick()
{
	if (bStopped)
	{
		DeltaTime = 0.0;
		return;
	}

	__int64 CurrentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&CurrentTime);
	TickTime = CurrentTime;

	// Time difference between this frame and the previous.
	DeltaTime = (TickTime - PrevTime) * SecondsPerCount;

	// Prepare for next frame.
	PrevTime = TickTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then DeltaTime can be negative.
	if (DeltaTime < 0.0)
	{
		DeltaTime = 0.0;
	}
}
