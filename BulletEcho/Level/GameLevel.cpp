#include "GameLevel.h"

#include <Actor/Player.h>
#include <Actor/Enemy.h>
#include <Render/Renderer.h>
#include <Input/Input.h>


using namespace Craft;

void GameLevel::OnInitialized()
{
	Level::OnInitialized();

	// 플레이어 액터 추가
	SpawnActor<Player>(Vector2::Zero);

	// 적 생성기 액터 추가
	SpawnActor<Enemy>(Vector2::Zero);
}

void GameLevel::ProcessPlayerSight()
{
	for (const auto& actor : actorList)
	{
		if (!actor) continue;

		if (std::shared_ptr<Player> player = Cast<Player>(actor))
		{
			Sight* sight = player->GetSight();

			if (!sight) continue;

			sight->CalculatePlayerSight();
			//return;
		}
		else if (std::shared_ptr<Enemy> enemy = Cast<Enemy>(actor))
		{
			Sight* sight = enemy->GetSight();

			if (!sight) continue;

			sight->CalculateSight();
			//return;
		}
	}
}

bool GameLevel::CanMove(const Craft::Vector2& nextPosition)
{
	// 게임 클리어인 경우 처리 안함
	//if (isGameClear)
	//	return false;

	// 이동하려는 위치에 어떤 액터가 있는지를 확인할 때 타입을 활용
	std::shared_ptr<Actor> player;
	for (const std::shared_ptr<Actor>& actor : actorList)
	{
		// 현재 액터가 플레이어 타입인지 확인(커스텀 RTTI 활용)
		if (actor->IsTypeOf<Player>())
		{
			player = actor;
			break;
		}
	}

	float playerMinX = nextPosition.x;
	float playerMinY = nextPosition.y;
	float playerMaxX = playerMinX + player->GetWidth() - 1;
	float playerMaxY = playerMinY + player->GetHeight() - 1;

	// 레벨을 순회하면서 플레이어가 아닌 타입을 actorList에 저장
	for (const std::shared_ptr<Actor>& actor : actorList)
	{
		// 현재 액터가 플레이어 타입인지 확인(커스텀 RTTI 활용)
		if (actor->IsTypeOf<Player>())
			continue;
		
		float actorMinX = actor->GetPosition().x;
		float actorMinY = actor->GetPosition().y;
		float actorMaxX = actorMinX + actor->GetWidth() - 1;
		float actorMaxY = actorMinY + actor->GetHeight() - 1;

		if (playerMaxX < actorMinX || actorMaxX < playerMinX)
			continue;
		if (playerMaxY < actorMinY || actorMaxY < playerMinY)
			continue;

		return false;
	}

	return true;
}