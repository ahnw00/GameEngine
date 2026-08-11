#include "Player.h"
#include <Input/Input.h>
#include <Level/GameLevel.h>
#include <Game/Game.h>

using namespace Craft;

Player::Player(const Vector2& position)
	: Actor("P", position, Color::Green)
{
	sortingOrder = 10;
}

void Player::Tick(float deltaTime)
{
	// 상위 계층의 tick 호출
	super::Tick(deltaTime);

	// ESC 종료 처리
	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		// 종료 처리
		//QuitGame();

		// 메뉴 토글
		Game& game = dynamic_cast<Game&>(Engine::Get());
		game.ToggleMenu();
		return;
	}

	// 이동 처리를 위해 GameLevel 객체 얻어오기
	// 다운 캐스팅 - 형변환 실패하면 null 반환
	std::shared_ptr<GameLevel> level = Cast<GameLevel>(GetOwner());

	// 이동 처리
	Vector2 newPosition = GetPosition();
	if (Input::Get().GetKeyDown(VK_RIGHT))
	{
		// 이동하려는 위치 값 만들기
		newPosition.x += 1;
	}
	else if (Input::Get().GetKeyDown(VK_LEFT))
	{
		// 이동하려는 위치 값 만들기
		newPosition.x -= 1;
	}
	if (Input::Get().GetKeyDown(VK_UP))
	{
		// 이동하려는 위치 값 만들기
		newPosition.y -= 1;
	}
	else if (Input::Get().GetKeyDown(VK_DOWN))
	{
		// 이동하려는 위치 값 만들기
		newPosition.y += 1;
	}

	// 이동 가능 여부 확인
	if (level && level->CanMove(GetPosition(), newPosition))
	{
		// 새로운 위치 설정
		SetPosition(newPosition);
	}
}
