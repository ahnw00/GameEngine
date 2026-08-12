#include "GameLevel.h"
#include <Actor/Player.h>
//#include <Actor/EnemySpawner.h>
#include <Render/Renderer.h>
#include <Input/Input.h>

using namespace Craft;

void GameLevel::OnInitialized()
{
	Level::OnInitialized();

	// 플레이어 액터 추가
	SpawnActor<Player>();

	// 적 생성기 액터 추가
	//SpawnActor<EnemySpawner>();
}

void GameLevel::ProcessPlayerSight()
{
	for (const auto& actor : actorList)
	{
		if (!actor) continue;

		std::shared_ptr<Player> player = Cast<Player>(actor);

		if (player)
		{
			Sight* sight = player->GetSight();

			if (!sight) continue;

			sight->Detect();
			return;
		}
	}
}
