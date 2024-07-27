#pragma once

#include "Object.h"

class WViewCamera : public WObject
{
public:
	WViewCamera() = default;

	XMMATRIX GetViewMatrix();
};