#pragma once

#ifndef RELEASECOM
#define RELEASECOM(x) { if(x){ x->Release(); x = 0; } }
#endif