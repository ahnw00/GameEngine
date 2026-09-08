#include "ClearLevel.h"
#include <Game/Game.h>
#include <Input/Input.h>
#include <Render/Renderer.h>

#include <format>


using namespace Craft;

ClearLevel::ClearLevel()
{
	menuTitle = "CLEAR!";

	// 메뉴 아이템 생성
	itemList.emplace_back(
		std::make_unique<UIItem>(
			"-Restart",
			[]()
			{
				// 메뉴 토글 함수 호출
				Game& game = dynamic_cast<Game&>(Engine::Get());
				game.GotoLevel(State::GamePlay);
			}
		)
	);

	itemList.emplace_back(
		std::make_unique<UIItem>(
			"-Credit",
			[]()
			{
				// 메인메뉴로 나가기
				Game& game = dynamic_cast<Game&>(Engine::Get());
				game.GotoLevel(State::Credit);
			}
		)
	);
}

void ClearLevel::OnInitialized()
{
	UILevel::OnInitialized();

	Engine::Get().PlayerBackgroundMusic("dangerousFuture.wav");
}

void ClearLevel::Tick(float deltaTime)
{
	UILevel::Tick(deltaTime);


}

void ClearLevel::Draw()
{
	Vector2 screenSize = Renderer::Get().GetScreenSize();
	Vector2 center = Vector2(
		screenSize.x / 2 - 16,
		screenSize.y / 2 - 4
	);

	std::string timeCleared = "You've Saved the Matrix in " + std::format("{:.2f}", clearTime) + " sec";

	UILevel::Draw();
	Renderer::Get().Submit(nullptr, { " ", timeCleared}, center);
}
