#pragma once

#include <string>
#include <memory>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>

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