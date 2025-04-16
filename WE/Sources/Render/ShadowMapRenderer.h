#pragma once

#include <d3d12.h>
#include <memory>
#include "Utility/Class.h"

class FShadowMapRenderer : FNoncopyable
{
public:
	FShadowMapRenderer();
	void Render(ID3D12GraphicsCommandList* CommandList) {};

private:

};