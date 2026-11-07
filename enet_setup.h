#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI

#define CloseWindow CloseWinWindow
#define ShowCursor ShowWinCursor
#define LoadImage LoadWinImage
#define DrawText DrawWinText
#define DrawTextEx DrawWinTextEx
#define PlaySound PlayWinSound

#include "enet/enet.h"

#undef CloseWindow
#undef ShowCursor
#undef LoadImage
#undef DrawText
#undef DrawTextEx
#undef PlaySound