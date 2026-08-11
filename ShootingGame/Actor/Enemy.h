#pragma once

#include <Actor/Actor.h>
#include <Utility/Timer.h>

class Enemy : public Craft::Actor
{
	TYPE_DECLARATIONS(Enemy, Actor)

	// 이동 방향 열거형
	enum class MoveDirection
	{
		None = -1,
		Left,
		Right
	};

public:
	Enemy(const std::string& image = "(o0o)", int yPosition = 5);

private:
	// 이벤트 함수 오버라이드
	virtual void Tick(float deltaTime) override;

	// 충돌 처리 함수 오버라이드
	virtual void OnCollision(const std::shared_ptr<Actor>& other) override;

private:
	// 이동 방향 열거형 함수
	MoveDirection direction = MoveDirection::None;

	// 좌우 이동 처리를 위한 변수
	float xPosition = 0.f;
	float moveSpeed = 5.f;

	// 발사 타이머
	Timer timer;
};

