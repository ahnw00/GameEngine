#pragma once

#include <Actor/Character.h>

class Player;

class Sight
{

public:
	Sight(Character* owner);

public:
	// 이벤트 함수 오버라이드
	void Tick(float deltaTime);

public:
	// 플레이어 시야 계산
	void CalculatePlayerSight();

	// 적의 시야(플레이어 감지)
	Player* DetectPlayer();

	void CalculateSight();

	inline void SetRadius(float r) { radius = r; }
	inline void SetDegree(float d) { degree = d; }

private:
	// 부채꼴 모양의 시야 범위
	float radius = 0.f;
	float degree = 0.f;

	// 탐지 시작 위치
	Craft::Vector2 startPoint = Craft::Vector2::Zero;
	// 탐지 방향
	Craft::Vector2 forward = Craft::Vector2::Zero;

	// 탐지 주체
	Character* owner;

	// 탐지 주체의 타입
	Character::Type ownerType = Character::Type::None;

	// 적이면 플레이어 감지시 타겟 설정
	void SetTarget(Player* detectedPlayer);
};

