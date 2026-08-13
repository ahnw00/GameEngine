#include "Player.h"

#include <Engine/Engine.h>
#include <Core/Core.h>
#include <Input/Input.h>
#include <Level/Level.h>
#include <Actor/Bullet.h>
//#include <Actor/PlayerBullet.h>
//#include <Actor/EnemyBullet.h>
//#include <Actor/DestroyEffect.h>
#include <Render/Renderer.h>

#include <cmath>

using namespace Craft;

Player::Player(
	Craft::Vector2 position
) : Character({ " * ", "***", " * "}, position, Color::Green),
	fireMode(FireMode::OneShot)
{
	// 캐릭터 타입 설정
	SetCharacterType(Character::Type::Player);

	// 공격력 세팅
	SetAttackPower(50.f);

	// 시야 세팅
	sight = std::make_unique<Sight>(this);
	sight->SetRadius(20.f);
	sight->SetDegree(30.f);

	// 생성 위치 설정
	int x = (Engine::Get().GetWidth() / 2) - (width / 2);
	int y = (Engine::Get().GetHeight() / 2) - (height / 2);

	//int x = position.x;
	//int y = position.y;

	SetPosition(Vector2(x, y));

	// x, y 위치 저장
	xPosition = static_cast<float>(x);
	yPosition = static_cast<float>(y);

	// 연사 타이머 시간 설정
	fireInterval = 0.1f;
	timer.SetTargetTime(fireInterval);
}

void Player::Tick(float deltaTime)
{
	super::Tick(deltaTime);
	sight->Tick(deltaTime);

	// ESC 키 종료
	if (Input::Get().GetKeyDown(VK_ESCAPE))
		QuitGame();

	float xDir = 0.f;
	if (Input::Get().GetKey('D'))
		xDir = 1.f;
	if (Input::Get().GetKey('A'))
		xDir = -1.f;

	float yDir = 0.f;
	if (Input::Get().GetKey('S'))
		yDir = 1.f;
	if (Input::Get().GetKey('W'))
		yDir = -1.f;

	// 이동 함수 호출
	Move(xDir, yDir, deltaTime);

	// 바라보는 방향 구하기
	Vector2 mousePos = Input::Get().GetMousePosition();
	forward = (mousePos - GetCenterPosition()).normalized();

	// 바라보는 방향에 따라 이미지 바꿔주기
	Actor::Direction dir = GetForwardDirection();
	image = sprites[static_cast<int>(dir)];

	// 발사 타이머 업데이트
	timer.Tick(deltaTime);

	if (fireMode == FireMode::OneShot)
	{
		// 단발 모드 처리
		if (Input::Get().GetKeyDown(VK_LBUTTON))
		{
			Fire();
		}
	}
	else if (fireMode == FireMode::Repeat)
	{
		// 연사 발사 처리
		if (Input::Get().GetKey(VK_LBUTTON))
		{
			FireInterval();
		}
	}

	// 발사 모드 전환 처리
	if (Input::Get().GetKeyDown('R'))
	{
		if (fireMode == FireMode::OneShot)
		{
			fireMode = FireMode::Repeat;
		}
		else if (fireMode == FireMode::Repeat)
		{
			fireMode = FireMode::OneShot;
		}
	}
}

void Player::OnCollision(const std::shared_ptr<Actor>& other)
{
	super::OnCollision(other);

	//// 부딪힌 액터가 적 탄약이면 처리
	//if (other->IsTypeOf<EnemyBullet>())
	//{
	//	// 플레이어 제거
	//	Destroy();

	//	// 적 탄약 제거
	//	other->Destroy();

	//	// 파괴 이펙트 생성
	//	if (GetOwner())
	//	{
	//		GetOwner()->SpawnActor<DestroyEffect>(GetPosition());

	//		// 게임 오버
	//		QuitGame();
	//	}
	//}
}
