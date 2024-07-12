#include "Timer.h"

#include <windows.h>

UTimer::UTimer() :
	DeltaTime(-1.0), BaseTime(0),
    PausedTime(0), StopTime(0), PrevTime(0), CurrTime(0), bStopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	SecondsPerCount = 1.0 / (double)countsPerSec;
}

double UTimer::GetTotalTime() const
{
	// If we are stopped, do not count the time that has passed since we stopped.
	// Moreover, if we previously already had a pause, the distance 
	// StopTime - mBaseTime includes paused time, which we do not want to count.
	// To correct this, we can subtract the paused time from StopTime:  
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  BaseTime       StopTime        StartTime     StopTime    CurrTime

	if (bStopped)
	{
		return (((StopTime - PausedTime) - BaseTime) * SecondsPerCount);
	}

	// The distance CurrTime - BaseTime includes paused time,
	// which we do not want to count.  To correct this, we can subtract 
	// the paused time from CurrTime:  
	//
	//  (CurrTime - PausedTime) - BaseTime 
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------> time
	//  BaseTime       StopTime        StartTime     CurrTime

	else
	{
		return (((CurrTime - PausedTime) - BaseTime) * SecondsPerCount);
	}
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
	__int64 StartTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&StartTime);


	// Accumulate the time elapsed between stop and start pairs.
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  BaseTime       StopTime        StartTime     

	if (bStopped)
	{
		PausedTime += (StartTime - StopTime);

		PrevTime = StartTime;
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
	CurrTime = CurrentTime;

	// Time difference between this frame and the previous.
	DeltaTime = (CurrTime - PrevTime) * SecondsPerCount;

	// Prepare for next frame.
	PrevTime = CurrTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.
	if (DeltaTime < 0.0)
	{
		DeltaTime = 0.0;
	}
}
