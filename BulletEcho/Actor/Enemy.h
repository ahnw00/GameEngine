#pragma once

#include <Actor/Actor.h>
#include <Actor/Character.h>
#include <memory>


class Player;

class Enemy : public Character
{
	enum class Mode
	{
		None,
		Patrol, // 플레이어가 감지되지 않은 상태에선 순찰
		Trace,  // 플레이어 감지하면 공격 범위(사격 범위 or 근접 공격 범위)까지 추격
		Shoot,  // 사격
		Stab,   // 근접 공격
		Search  // 플레이어를 공격하다가 놓치면 일정 시간 동안 수색
	};

	TYPE_DECLARATIONS(Enemy, Character)

public:
	Enemy(Craft::Vector2 position);

private:
	// 이벤트 함수 오버라이드
	virtual void Tick(float deltaTime) override;

	void Patrol();

	void Trace();

	void Shoot();

	void Stab();

	void Search();

private:
	// 이동 가능 위치인지 확인하여 가능한 위치 반환
	Craft::Vector2 CheckMovable(Craft::Vector2 point);

public:
	inline void SetTarget(Player* p) { target = p; }

private:
	// 순찰 위치들
	Craft::Vector2 patrolPoints[5] = {};

	// 이동해야하는 지점
	Craft::Vector2 targetPoint = Craft::Vector2::Zero;

	// 탐지한 플레이어
	Player* target = nullptr;

	// 현재 모드
	Mode mode = Mode::None;
};

