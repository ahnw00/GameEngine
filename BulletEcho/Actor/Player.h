#pragma once

#include <Actor/Actor.h>
#include <Util/Timer.h>

//" * "
//"***"
//" * "

class Player : public Craft::Actor
{
	// 발사 모드(단발/연사)
	enum class FireMode
	{
		None = -1,
		OneShot,
		Repeat
	};

	// RTTI 등록
	TYPE_DECLARATIONS(Player, Actor)

public:
	Player();

private:
	// 이벤트 함수 오버라이드
	virtual void Tick(float deltaTime) override;

	// 충돌 이벤트 함수 오버라이드
	virtual void OnCollision(const std::shared_ptr<Actor>& other) override;

	// 이동 처리 함수(내부에서 사용)
	void Move(float xDir, float yDir, float deltaTime);

	// 탄약 발사 함수
	void Fire();

	// 연속 발사 함수
	void FireInterval();

	// 발사 가능 여부 확인 함수
	inline bool CanShoot() const { return timer.IsTimeOut(); }

	// 바라보는 방향 구하는 함수
	Craft::Actor::Direction GetForwardDirection();

private:
	// 좌우 이동 처리할 때 사용할 변수
	float xPosition = 0.f;

	// 위아래 이동 처리할 때 사용할 변수
	float yPosition = 0.f;

	// 이동 속도 변수
	float moveSpeed = 10.f;

	// 발사 모드 변수
	FireMode fireMode = FireMode::None;

	// 타이머 변수
	Timer timer;

	// 연사 시 발사 간격(단위: 초)
	float fireInterval = 0.1f;

	// 8방향 스프라이트
	// 순서: N, NE, E, SE, S, SW, W, NW
	std::vector<std::string> sprites[8] = {
		{" * "
		," * "
		,"   "},
		
		{"  *"
		," * "
		,"   "},

		{"   "
		," **"
		,"   "},
	
		{"   "
		," * "
		,"  *"},

		{"   "
		," * "
		," * "},

		{"   "
		," * "
		,"*  "},

		{"   "
		,"** "
		,"   "},

		{"*  "
		," * "
		,"   "},

	};
};
