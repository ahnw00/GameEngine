#include "GameLevel.h"
#include <Actor/Player.h>
#include <Actor/EnemySpawner.h>
#include <Render/Renderer.h>
#include <Input/Input.h>

using namespace Craft;

void GameLevel::OnInitialized()
{
	Level::OnInitialized();

	// 플레이어 액터 추가
	SpawnActor<Player>();

	// 적 생성기 액터 추가
	SpawnActor<EnemySpawner>();
}

void GameLevel::Draw()
{
	super::Draw();

	Vector2 mousePos = Input::Get().GetMousePosition();

	std::string temp = "(" 
		+ std::to_string(mousePos.x) 
		+ ", " 
		+ std::to_string(mousePos.y) 
		+ ")";

	Renderer::Get().Submit(
		temp,
		Vector2(20, 0)
	);
}