#include "Character.h"

#include <Util/Sight.h>
#include <Engine/Engine.h>
#include <Level/Level.h>
#include <Actor/Bullet.h>
#include <Level/GameLevel.h>

#include <cmath>


using namespace Craft;

Character::Character(
	const std::vector<std::string>& sprite, 
	Craft::Vector2 position, 
	Craft::Color color
) : Actor(sprite, position, color)
{
	// 그려지는 우선순위
	sortingOrder = 5;
}

Character::~Character()
{ }

void Character::Tick(float deltaTime)
{
	super::Tick(deltaTime);
}

void Character::OnCollision(const std::shared_ptr<Actor>& other)
{
	super::OnCollision(other);

}

void Character::Move(float xDir, float yDir, float deltaTime)
{
	// x 위치 업데이트
	// 이동 처리 -> 이동 방향과 빠르기를 적용해서 새로운 위치를 구하는 것
	float newX = xPosition + xDir * moveSpeed * deltaTime;
	float newY = yPosition + yDir * moveSpeed * deltaTime;

	//xPosition += xDir * moveSpeed * deltaTime;
	//yPosition += yDir * moveSpeed * deltaTime;

	// 화면 왼쪽 벗어나지 않도록 처리
	if (newX < 0)
	{
		newX = 0.f;
	}
	// 화면 오른쪽 벗어나지 않도록 처리
	if (newX + width >= Engine::Get().GetWidth())
	{
		newX = static_cast<float>(Engine::Get().GetWidth() - width);
	}

	// 화면 위쪽 벗어나지 않도록 처리
	if (newY < 0)
	{
		newY = 0.f;
	}
	// 화면 오른쪽 벗어나지 않도록 처리
	if (newY >= Engine::Get().GetHeight())
	{
		newY = static_cast<float>(Engine::Get().GetHeight());
	}

	// 위치 업데이트
	Vector2 newPosition = GetPosition();
	// float 값을 int로 형변환할 때 소숫점 값은 버림 처리됨!
	newPosition.x = static_cast<int>(newX);
	newPosition.y = static_cast<int>(newY);

	// 이동 처리를 위해 GameLevel 객체 얻어오기
	// 다운 캐스팅 - 형변환 실패하면 null 반환
	std::shared_ptr<GameLevel> level = Cast<GameLevel>(GetOwner());
	if (level && level->CanMove(newPosition))
	{
		SetPosition(newPosition);
		xPosition = newX;
		yPosition = newY;
	}
}

void Character::Fire()
{
	Vector2 bulletInitPosition = Vector2(
		GetPosition().x + (width / 2),
		GetPosition().y + (height / 2)
	);

	// 탄약 생성
	std::shared_ptr<Level> owner = GetOwner();
	if (owner)
	{
		owner->SpawnActor<Bullet>(
			bulletInitPosition, 
			forward, 
			shared_from_this(),
			attackPower
		);
	}
}

void Character::FireInterval()
{
	// 발사 가능 여부 확인
	if (!CanShoot())
		return;

	// 발사 처리
	Fire();

	// 경과 시간 초기화
	timer.Reset();
}

Craft::Actor::Direction Character::GetForwardDirection()
{
	constexpr float PI = 3.14159265f;

	float angle = std::atan2(-forward.y, forward.x) * 180.f / PI;

	// E를 0도로 만들고, N을 90도로 맞춤
	angle = 90.f - angle;

	if (angle < 0.f)
		angle += 360.f;

	int idx = static_cast<int>(std::round(angle / 45.f)) % 8;

	return static_cast<Actor::Direction>(idx);
}

Sight* Character::GetSight()
{
	if(!sight)
		return nullptr;

	return sight.get();
}

void Character::ApplyDamage(float damage)
{
	HP = (HP - damage < 0.f) ? 0.f : HP - damage;

	if (HP <= 0.f)
	{
		Die();
	}
}

void Character::Die()
{
	// Todo: 죽는 모션 재생

	Destroy();
}
