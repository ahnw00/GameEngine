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
#include <Util/Camera.h>
#include <Game/Game.h>
#include <Level/GameLevel.h>

#include <cmath>

using namespace Craft;

Player::Player(
	Craft::Vector2 position
) : Character({ "   ", " * ", "   "}, position, Color::Green),
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

	camera = std::make_unique<Camera>(Renderer::Get().GetScreenSize(), this);

	int x = position.x;
	int y = position.y;

	SetPosition(Vector2(x, y));

	// x, y 위치 저장
	xPosition = static_cast<float>(x);
	yPosition = static_cast<float>(y);

	// 연사 타이머 시간 설정
	fireInterval = 0.1f;
	timer.SetTargetTime(fireInterval);
}

Player::~Player() = default;

void Player::BeginPlay()
{
	//camera = std::make_unique<Camera>(Renderer::Get().)
}

void Player::Tick(float deltaTime)
{
	super::Tick(deltaTime);
	sight->Tick(deltaTime);
	camera->FollowPlayer();
	
	std::shared_ptr<GameLevel> gameLevel = Cast<GameLevel>(GetOwner());
	if (gameLevel)
	{
		gameLevel->SetElapsedTime(deltaTime);
	}

	// ESC 키 종료
	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		// 메뉴 토글
		Game& game = dynamic_cast<Game&>(Engine::Get());
		game.ToggleMenu();
		return;

		//QuitGame();
	}

	// 2d일 때 움직임
	//float xDir = 0.f;
	//if (Input::Get().GetKey('D'))
	//	xDir = 1.f;
	//if (Input::Get().GetKey('A'))
	//	xDir = -1.f;

	//float yDir = 0.f;
	//if (Input::Get().GetKey('S'))
	//	yDir = 1.f;
	//if (Input::Get().GetKey('W'))
	//	yDir = -1.f;

	Vector2 movingDir = Vector2::Zero;
	Vector2 right(-forward.y, forward.x);

	if (Input::Get().GetKey('W'))
		movingDir += forward;
	if (Input::Get().GetKey('A'))
		movingDir -= right;
	if (Input::Get().GetKey('S'))
		movingDir -= forward;
	if (Input::Get().GetKey('D'))
		movingDir += right;

	// 이동 함수 호출
	//bool isMoving = Move(xDir, yDir, deltaTime);

	bool isMoving = false;
	if (movingDir.x != 0.f || movingDir.y != 0.f)
	{
		movingDir = movingDir.normalized();

		isMoving = Move(movingDir.x, movingDir.y, deltaTime);
	}

	if (isMoving)
	{
		footstepTimer += deltaTime;

		if (footstepTimer >= footstepInterval)
		{
			footstepTimer = 0.f;
			Engine::Get().PlayerOneShot("step.wav");
		}
	}
	else
	{
		footstepTimer = 0.35f;
	}

	// Todo: 2d일 때랑 3d일 때 회전 다르게 처리
	// 바라보는 방향 구하기
	Vector2 mousePos = Input::Get().GetMousePosition();
	//forward = (mousePos - GetCenterPosition()).normalized();

	Vector2 prevMousePos = Input::Get().GetPrevMousePosition();
	
	float mouseDeltaX = Input::Get().GetMouseDelta().x;

	if (mouseDeltaX != 0.f)
	{
		float rotation = rotateSpeed * mouseDeltaX;

		// Degree -> Radian 변환
		float radStep = rotation * (3.14159265f / 180.f);

		// 회전 행렬을 적용하여 forward 벡터 회전
		float cosVal = std::cos(radStep);
		float sinVal = std::sin(radStep);

		float newX = forward.x * cosVal - forward.y * sinVal;
		float newY = forward.x * sinVal + forward.y * cosVal;

		if (!(newX == 0 && newY == 0))
		{
			forward = Vector2(newX, newY).normalized();
		}
	}

	// 바라보는 방향에 따라 이미지 바꿔주기
	//Actor::Direction dir = GetForwardDirection();
	//image = sprites[static_cast<int>(dir)];

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
