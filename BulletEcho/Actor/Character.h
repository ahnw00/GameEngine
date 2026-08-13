#pragma once

#include <Actor/Actor.h>
#include <Util/Timer.h>
#include <memory>

class Sight;

class Character : public Craft::Actor, public std::enable_shared_from_this<Character>
{
public:
	enum class Type
	{
		None,
		Player,
		Enemy
	};

	TYPE_DECLARATIONS(Character, Actor)

public:
	Character(
		const std::vector<std::string>& sprite,
		Craft::Vector2 position,
		Craft::Color color
	);
	virtual ~Character();

protected:
	// 이벤트 함수 오버라이드
	virtual void Tick(float deltaTime) override;

	// 충돌 이벤트 함수 오버라이드
	virtual void OnCollision(const std::shared_ptr<Actor>& other) override;

	// 이동 처리 함수(내부에서 사용)
	virtual void Move(float xDir, float yDir, float deltaTime);

	// 탄약 발사 함수
	virtual void Fire();

	// 연속 발사 함수
	virtual void FireInterval();

	// 발사 가능 여부 확인 함수
	inline bool CanShoot() const { return timer.IsTimeOut(); }

	// 바라보는 방향 구하는 함수(8방향 중 한 방향)
	Craft::Actor::Direction GetForwardDirection();

protected:
	// 좌우 이동 처리할 때 사용할 변수
	float xPosition = 0.f;

	// 위아래 이동 처리할 때 사용할 변수
	float yPosition = 0.f;

	// 캐릭터 타입(플레이어/적)
	Type type = Type::None;

	// 이동 속도
	float moveSpeed = 30.f;

	// 발사 간격
	float fireInterval = 0.f;

	// 타이머 변수
	Timer timer;

	// 시야
	std::unique_ptr<Sight> sight;

	// 체력
	float HP = 100.f;

	// 공격력
	float attackPower = 10.f;

public:
	inline Type GetCharacterType()	const { return type; }

	inline void SetAttackPower(float power) { attackPower = power; }

	Sight* GetSight();

	void ApplyDamage(float damage);

	void Die();

protected:
	inline void SetCharacterType(Type ct) { type = ct; }
};

