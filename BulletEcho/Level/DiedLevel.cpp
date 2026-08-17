#include "DiedLevel.h"
#include <Game/Game.h>
#include <Input/Input.h>
#include <Render/Renderer.h>


using namespace Craft;

DiedLevel::DiedLevel()
{
	menuTitle = "YOU DIED";

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
			"-Menu",
			[]()
			{
				// 메인메뉴로 나가기
				Game& game = dynamic_cast<Game&>(Engine::Get());
				game.GotoLevel(State::Menu);
			}
		)
	);
}

void DiedLevel::OnInitialized()
{
	UILevel::OnInitialized();


}

void DiedLevel::Tick(float deltaTime)
{
	UILevel::Tick(deltaTime);


}

void DiedLevel::Draw()
{
	UILevel::Draw();


}
