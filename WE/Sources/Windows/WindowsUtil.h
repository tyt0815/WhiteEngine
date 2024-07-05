#pragma once

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#include <windows.h>
#include <wrl.h>

#define MAX_STRING 256

#ifndef RELEASECOM
#define RELEASECOM(x) { if(x){ x->Release(); x = 0; } }
#endif