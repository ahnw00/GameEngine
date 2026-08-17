#include "Camera.h"

#include <Actor/Player.h>
#include <Render/Renderer.h>

#include <iostream>


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

    // 카메라가 월드 밖으로 나가지 않도록 제한
    newRenderPosition.x = max(
        0.0f,
        min(
            newRenderPosition.x,
            Renderer::Get().GetWorldSize().x - screenSize.x
        )
    );

    newRenderPosition.y = max(
        0.0f,
        min(
            newRenderPosition.y,
            Renderer::Get().GetWorldSize().y - screenSize.y
        )
    );

	Renderer::Get().SetRenderStartPosition(newRenderPosition);
}
