#include <iostream>
#include "CubeState.h"
#include "ftd2xx.h"
#pragma comment (lib, "ftd2xx.lib")

extern CubeState state;

DWORD WINAPI connect(LPVOID n);