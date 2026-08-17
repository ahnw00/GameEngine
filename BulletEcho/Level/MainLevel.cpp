#include "MainLevel.h"
#include <Game/Game.h>
#include <Input/Input.h>
#include <Render/Renderer.h>


using namespace Craft;

MainLevel::MainLevel()
{
	menuTitle = "PROJECT S";

	itemList.emplace_back(
		std::make_unique<UIItem>(
			"-Start",
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
			"-Quit Game",
			[]()
			{
				// 게임 종료 호출
				Engine::Get().Quit();
			}
		)
	);
}

void MainLevel::OnInitialized()
{
	UILevel::OnInitialized();

	// 메인메뉴 브금
	Engine::Get().PlayerBackgroundMusic("mainbgm.wav");
}

void MainLevel::Tick(float deltaTime)
{
	UILevel::Tick(deltaTime);


}

void MainLevel::Draw()
{
	UILevel::Draw();

}
