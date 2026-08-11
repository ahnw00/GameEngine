#include "PlayerBullet.h"

using namespace Craft;

PlayerBullet::PlayerBullet(const Vector2& position)
	: Actor("*", position, Color::White),
	yPosition(static_cast<float>(position.y))
{
}

void PlayerBullet::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	// 아래에서 위로 이동 처리
	yPosition -= moveSpeed * deltaTime;

	// 좌표 검사(화면 벗어났는지 확인)
	if (yPosition < 0.f)
	{
		// 삭제 처리
		Destroy();
		return;
	}

	// 위치 값 설정 및 갱신
	Vector2 newPosition = GetPosition();
	newPosition.y = static_cast<float>(yPosition);
	SetPosition(newPosition);
}
