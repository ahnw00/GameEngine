#pragma once

#include <Actor/Actor.h>

class Bullet : public Craft::Actor
{
	TYPE_DECLARATIONS(Bullet, Actor)

public:
	Bullet(const Craft::Vector2& position, const Craft::Vector2& direction);

private:
	// 이벤트 함수 오버라이드
	virtual void Tick(float deltaTime) override;

private:
	// 총알 속도
	float moveSpeed = 90.f;

	Craft::Vector2 position = Craft::Vector2::Zero;
	Craft::Vector2 direction = Craft::Vector2::Zero;

	// 총알의 위치
	float xPosition = 0.f;
	float yPosition = 0.f;
};

