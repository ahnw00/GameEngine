#include "Credit.h"
#include <Game/Game.h>
#include <Input/Input.h>
#include <Render/Renderer.h>


using namespace Craft;

Credit::Credit()
{

}

void Credit::OnInitialized()
{
	UILevel::OnInitialized();

	// Todo: Credit bgm
}

void Credit::Tick(float deltaTime)
{
	UILevel::Tick(deltaTime);

	// 입력 처리(위/아래 방향키, 엔터, ESC 키)
	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		Game& game = dynamic_cast<Game&>(Engine::Get());
		game.GotoLevel(State::Main);

		// 인덱스 초기화
		currentIdx = 0;
	}
}

void Credit::Draw()
{
	UILevel::Draw();

	Vector2 screenSize = Renderer::Get().GetScreenSize();
	Vector2 center = Vector2(
		screenSize.x / 2 - 10,
		screenSize.y / 2
	);

	Renderer::Get().Submit(nullptr, { "Thank you for Playing" }, center);
	Renderer::Get().Submit(
		nullptr, 
		{ "made by ahnw00" }, 
		Vector2(center.x, center.y + 2)
	);
}
