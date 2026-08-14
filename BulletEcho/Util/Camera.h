#pragma once

#include <Math/Vector2.h>

class Player;

class Camera
{
public:
	Camera(const Craft::Vector2& screenSize, Player* owner);

	void FollowPlayer();

	//inline Craft::Vector2 GetPosition() const { return renderStartPosition; }


private:
	Craft::Vector2 screenSize = Craft::Vector2::Zero;

	Player* owner = nullptr;
};

