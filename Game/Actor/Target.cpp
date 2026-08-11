#include "Target.h"

using namespace Craft;

Target::Target(const Vector2& position)
	: Actor("T", position, Color::Red)
{
	// 우선순위 바닥보다 크고 박스나 플레이어보단 작아
	sortingOrder = 2;
}
