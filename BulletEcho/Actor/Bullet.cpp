#include "Bullet.h"

#include <Engine/Engine.h>
#include <Render/Renderer.h>
#include <Actor/Wall.h>
#include <Actor/Character.h>
#include <Actor/Enemy.h>
#include <Actor/DestroyEffect.h>
#include <Level/GameLevel.h>


using namespace Craft;

Bullet::Bullet(
	const Vector2& position,
	const Vector2& direction,
	std::shared_ptr<Character> shooter,
	float damage
) : Actor({ "*" }, position, Color::Yellow),
	direction(direction), position(position), shooter(shooter), damage(damage)
{}

void Bullet::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	Vector2 newPosition = GetPosition() + (direction * moveSpeed * deltaTime);

	int xLimit = Engine::Get().GetWidth();
	int yLimit = Engine::Get().GetHeight();

	if (newPosition.x < 0.f || newPosition.x >= xLimit)
		Destroy();
	if (newPosition.y < 0.f || newPosition.y >= yLimit)
		Destroy();

	SetPosition(newPosition);
}

void Bullet::OnCollision(const std::shared_ptr<Actor>& other)
{
	super::OnCollision(other);

	// 본인이 쏜 총알이면 충돌 무시
	if (shooter && shooter == Cast<Character>(other))
		return;
	// 총알끼리 부딪히는거 무시
	if (Cast<Bullet>(other))
		return;

	// 충돌시 총알 제거
	Destroy();
	// 파괴 이펙트 생성
	if (GetOwner())
	{
		GetOwner()->SpawnActor<DestroyEffect>(GetPosition());
	}

	// 캐릭터(적/플레이어)와 충돌했을 경우
	if (other->IsTypeOf<Character>())
	{
		std::shared_ptr<Character> damagedCharacter = Cast<Character>(other);
		damagedCharacter->ApplyDamage(damage);
	}
}
