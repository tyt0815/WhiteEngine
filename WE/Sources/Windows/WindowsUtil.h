#pragma once

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

#include <windows.h>
#include <wrl.h>

#define MAX_STRING 256

#ifndef RELEASECOM
#define RELEASECOM(x) { if(x){ x->Release(); x = 0; } }
#endif