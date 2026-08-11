#pragma once

#include <Core/Core.h>
#include <Windows.h>

namespace Craft
{
	// 색상을 Enum으로 정의
	// 일반 enum 아니고 강타입 enum
	enum class CRAFT_API Color : WORD
	{
		Red = FOREGROUND_RED,
		Green = FOREGROUND_GREEN,
		Blue = FOREGROUND_BLUE,
		Yellow = Red | Green, // | = + 비트 연산자
		Cyan = Green | Blue,
		Purple = Red | Blue,
		White = Red | Green | Blue,
		BrightWhite = White | FOREGROUND_INTENSITY
	};
}