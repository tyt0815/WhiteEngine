#pragma once

class UTimer
{
public:
	UTimer();

	float GetTotalTime()const; // in seconds
	float GetDeltaTime()const; // in seconds

	void Reset(); // Call before message loop.
	void Start(); // Call when unpaused.
	void Stop();  // Call when paused.
	void Tick();  // Call every frame.

private:
	double SecondsPerCount = 0.f;
	double DeltaTime = 0.f;

	__int64 BaseTime = 0;
	__int64 PausedTime = 0;
	__int64 StopTime = 0;
	__int64 PrevTime = 0;
	__int64 TickTime = 0;

	bool bStopped = false;
};