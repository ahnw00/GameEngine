#include "Box.h"

using namespace Craft;

Box::Box(const Vector2& position)
	: Actor("B", position, Color::Yellow)
{
	// 박스는 이동이 가능하기 때문에 땅과 겹칠 수 있음
	// 땅보다 높은 우선순위
	// 목표 지점(타겟)보다도 높아야해
	sortingOrder = 5;
}
