#include "Camera.h"

#include <Actor/Player.h>
#include <Render/Renderer.h>


using namespace Craft;

//Camera::Camera(const Vector2& screenSize, Player* owner)
//	: screenSize(screenSize), owner(owner)
//{
//}

Camera::Camera(const Vector2& screenSize, Player* owner)
	: screenSize(screenSize), owner(owner)
{
}

void Camera::FollowPlayer()
{
	if (!owner) return;

	Vector2 newRenderPosition =
		Vector2(
			owner->GetCenterPosition().x - screenSize.x / 2,
			owner->GetCenterPosition().y - screenSize.y / 2
		);

	Renderer::Get().SetRenderStartPosition(newRenderPosition);
}
