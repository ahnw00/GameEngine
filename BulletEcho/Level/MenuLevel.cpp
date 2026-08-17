#include "MenuLevel.h"
#include <Game/Game.h>
#include <Input/Input.h>
#include <Render/Renderer.h>


using namespace Craft;

MenuLevel::MenuLevel()
{
	//SetLevelType(LevelType::Menu);

	menuTitle = "PAUSE";

	// 메뉴 아이템 생성
	itemList.emplace_back(
		std::make_unique<UIItem>(
			"-Resume",
			[]()
			{
				// 메뉴 토글 함수 호출
				Game& game = dynamic_cast<Game&>(Engine::Get());
				game.ToggleMenu();
			}
		)
	);

	itemList.emplace_back(
		std::make_unique<UIItem>(
			"-Menu",
			[]()
			{
				// 메인메뉴로 나가기
				Game& game = dynamic_cast<Game&>(Engine::Get());
				game.GotoLevel(State::Main);
			}
		)
	);
}

void MenuLevel::OnInitialized()
{
	UILevel::OnInitialized();
}

void MenuLevel::Tick(float deltaTime)
{
	UILevel::Tick(deltaTime);

	// 입력 처리(위/아래 방향키, 엔터, ESC 키)
	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		Game& game = dynamic_cast<Game&>(Engine::Get());
		game.ToggleMenu();

		// 인덱스 초기화
		currentIdx = 0;
	}
}

void MenuLevel::Draw()
{
	UILevel::Draw();

}