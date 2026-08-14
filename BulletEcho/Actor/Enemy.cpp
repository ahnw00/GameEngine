#include "Enemy.h"

#include <Actor/Player.h>
#include <Util/Sight.h>
#include <Engine/Engine.h>
#include <Render/Renderer.h>
#include <Level/GameLevel.h>


using namespace Craft;

Enemy::Enemy(
	Craft::Vector2 position
) : Character({ " * ", "***", " * " }, position, Color::Purple)
{
	// 캐릭터 타입 설정
	SetCharacterType(Character::Type::Enemy);

	// 공격력 세팅
	SetAttackPower(10.f);

	// 시야 세팅
	sight = std::make_unique<Sight>(this);
	sight->SetRadius(20.f);
	sight->SetDegree(30.f);

	// 생성 위치 설정
	// DEBUGGING
	//int x = (Engine::Get().GetWidth() / 2) - (width / 2);
	//int y = (height / 2) + 1;

	int x = position.x;
	int y = position.y;

	SetPosition(Vector2(x, y));

	// x, y 위치 저장
	xPosition = static_cast<float>(x);
	yPosition = static_cast<float>(y);

	// 연사 타이머 시간 설정
	fireInterval = 1.f;
	timer.SetTargetTime(fireInterval);

	// 임시 방향
	forward = Vector2(0, 1);
}

void Enemy::Tick(float deltaTime)
{
	super::Tick(deltaTime);
	sight->Tick(deltaTime);
	timer.Tick(deltaTime);

	if (target)
	{
		targetPoint = target->GetCenterPosition();
		forward = (targetPoint - GetCenterPosition()).normalized();

		//std::string temp = 
		//std::to_string(target->GetCenterPosition().x) + 
		//", " + 
		//std::to_string(target->GetCenterPosition().y);
		//
		//Renderer::Get().Submit(
		//	nullptr,
		//	{ temp },
		//	Vector2(20, 1)
		//);

		timer.IsTimeOut();
		FireInterval();
	}
}

void Enemy::Patrol()
{

}

void Enemy::Trace()
{

}

void Enemy::Shoot()
{

}

void Enemy::Stab()
{

}

void Enemy::Search()
{

}

Vector2 Enemy::CheckMovable(Vector2 point)
{
	return Vector2::Zero;
}