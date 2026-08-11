#include "Ground.h"

using namespace Craft;

Ground::Ground(const Vector2& position)
	: Actor(".", position)
{
	// 우선순위 설정
	// 바닥 액터는 다른 액터랑 겹쳤을 때 덮어씌워져야 함
	sortingOrder = 0;
}
