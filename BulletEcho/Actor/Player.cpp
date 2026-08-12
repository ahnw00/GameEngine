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

Player::Player()
	: Character({ " * ", "***", " * "}, Vector2::Zero, Color::Green),
	fireMode(FireMode::OneShot)
{
	// 캐릭터 타입 설정
	SetCharacterType(Character::Type::Player);

	// 시야 세팅
	sight = std::make_unique<Sight>(this);
	sight->SetRadius(5.f);
	sight->SetDegree(60.f);

	// 그려지는 우선순위
	sortingOrder = 5;

	// 생성 위치 설정
	int x = (Engine::Get().GetWidth() / 2) - (width / 2);
	int y = (Engine::Get().GetHeight() / 2) - (height / 2);
	SetPosition(Vector2(x, y));

	// x, y 위치 저장
	xPosition = static_cast<float>(x);
	yPosition = static_cast<float>(y);

	// 연사 타이머 시간 설정
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
	//super::OnCollision(other);

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

// 오른쪽 바향: 1 | 왼쪽 방향: -1
// 이동 처리 함수(내부에서 사용)
void Player::Move(float xDir, float yDir, float deltaTime)
{
	// x 위치 업데이트
	// 이동 처리 -> 이동 방향과 빠르기를 적용해서 새로운 위치를 구하는 것
	xPosition += xDir * moveSpeed * deltaTime;
	yPosition += yDir * moveSpeed * deltaTime;

	// 화면 왼쪽 벗어나지 않도록 처리
	if (xPosition < 0)
	{
		xPosition = 0.f;
	}
	// 화면 오른쪽 벗어나지 않도록 처리
	if (xPosition + width >= Engine::Get().GetWidth())
	{
		xPosition = static_cast<float>(Engine::Get().GetWidth() - width);
	}

	// 화면 위쪽 벗어나지 않도록 처리
	if (yPosition < 0)
	{
		yPosition = 0.f;
	}
	// 화면 오른쪽 벗어나지 않도록 처리
	if (yPosition >= Engine::Get().GetHeight())
	{
		yPosition = static_cast<float>(Engine::Get().GetHeight());
	}

	// 위치 업데이트
	Vector2 newPosition = GetPosition();
	// float 값을 int로 형변환할 때 소숫점 값은 버림 처리됨!
	newPosition.x = static_cast<int>(xPosition);
	newPosition.y = static_cast<int>(yPosition);
	SetPosition(newPosition);
}

void Player::Fire()
{
	Vector2 bulletInitPosition = Vector2(
		GetPosition().x + (width / 2),
		GetPosition().y + (height / 2)
	);

	// 탄약 생성
	std::shared_ptr<Level> owner = GetOwner();
	if (owner)
	{
		owner->SpawnActor<Bullet>(bulletInitPosition, forward);
	}
}

void Player::FireInterval()
{
	// 발사 가능 여부 확인
	if (!CanShoot())
		return;

	// 발사 처리
	Fire();

	// 경과 시간 초기화
	timer.Reset();
}

Actor::Direction Player::GetForwardDirection()
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
