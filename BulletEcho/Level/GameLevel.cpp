#include "GameLevel.h"

#include <Actor/Player.h>
#include <Actor/Enemy.h>
#include <Actor/Wall.h>
#include <Actor/Bullet.h>
#include <Render/Renderer.h>
#include <Input/Input.h>
#include <Engine/Engine.h>
#include <Utility/Utility.h>

#include <cassert>
#include <iostream>
#include <vector>


using namespace Craft;

void GameLevel::OnInitialized()
{
	Level::OnInitialized();

	//SetLevelType(LevelType::GamePlay);

	Engine::Get().PlayerBackgroundMusic("bgm.wav");

	LoadMap("RandomMap.txt");

	//// 플레이어 액터 추가
	//SpawnActor<Player>(Vector2(50, 50));

	//// 적 생성기 액터 추가
	//SpawnActor<Enemy>(Vector2(20, 20));
}

void GameLevel::ProcessPlayerSight()
{
	for (const auto& actor : actorList)
	{
		if (!actor) continue;

		Renderer::RenderMode currentMode = Renderer::Get().GetRenderMode();

		bool b = actor->IsTypeOf<Player>();
		// 플레이 모드면 적의 시야 렌더 안해줘
		// DEBUGGING
		//if (currentMode == Renderer::RenderMode::PLAY &&
		//	!actor->IsTypeOf<Player>())
		//	continue;

		// 디버그 모드: 적과 플레이어 시야 모두 렌더
		// 플레이 모드: 플레이어의 시야만 렌더
		if (std::shared_ptr<Character> character = Cast<Character>(actor))
		{
			// 캐릭터의 시야 가져오기
			Sight* sight = character->GetSight();
			if (!sight) continue;

			// 시야 렌더해주기
			sight->CalculateSight();
		}
		else if (std::shared_ptr<Bullet> bullet = Cast<Bullet>(actor))
		{
			Vector2 bulletPosition = bullet->GetCenterPosition();

			Renderer::Get().SetSight(bulletPosition, Renderer::SightState::None);
		}
	}
}

bool GameLevel::CanMove(
	const Craft::Vector2& nextPosition,
	const std::shared_ptr<Actor>& movingActor)
{
	// 게임 클리어인 경우 처리 안함
	//if (isGameClear)
	//	return false;

	if (!movingActor)
		return false;

	int startX = static_cast<int>(nextPosition.x);
	int startY = static_cast<int>(nextPosition.y);

	int endX = startX + movingActor->GetWidth() - 1;
	int endY = startY + movingActor->GetHeight() - 1;

	for (int x = startX; x <= endX; ++x)
	{
		for (int y = startY; y <= endY; ++y)
		{
			const auto& actors = 
				Renderer::Get().GetActorsAt(Vector2(
					static_cast<int>(x), static_cast<int>(y)));

			for (const auto& actor : actors)
			{
				// 자기 자신이면 무시
				if (actor == movingActor.get())
					continue;

				const float actorMinX = actor->GetPosition().x;
				const float actorMinY = actor->GetPosition().y;

				const float actorMaxX =
					actorMinX + actor->GetWidth() - 1;

				const float actorMaxY =
					actorMinY + actor->GetHeight() - 1;

				const float movingMinX = nextPosition.x;
				const float movingMinY = nextPosition.y;

				const float movingMaxX =
					movingMinX + movingActor->GetWidth() - 1;

				const float movingMaxY =
					movingMinY + movingActor->GetHeight() - 1;

				if (movingMaxX < actorMinX || actorMaxX < movingMinX)
					continue;

				if (movingMaxY < actorMinY || actorMaxY < movingMinY)
					continue;

				return false;
			}
		}
	}

	return true;
}

void GameLevel::LoadMap(const std::string& filename)
{
	// 최종 경로 조립
	std::string path = std::string("../Assets/") + filename;

	// 파일 열기(C-style)
	FILE* file = nullptr;
	fopen_s(&file, path.c_str(), "rt");
	if (!file)
	{
		assert(false && "failed to open a stage file.");
		return;
	}

	// 파일의 내용을 저장할 버퍼(데이터 저장공간) 확인
	// 파일 길이 확인 -> 파일 위치를 제일 뒤로 이동 시킨 다음 해당 위치 값 읽기
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file); // ftell 현재 위치를 반환

	// 파일 제일 끝위치를 구한 다음에는 다시 처음으로 되돌리기
	rewind(file); //fseek(file, 0, SEEK_SET) 이랑 똑같

	// 앞에서 구한 위치를 사용해서 버퍼 생성
	char* buffer = new char[fileSize] {};

	// 데이터 읽기(파일 읽기)
	size_t readSize = fread(buffer, sizeof(char), fileSize, file);

	assert(readSize > 0 && "No data in the stage file");

	std::vector<std::string> map;
	std::string currentRow;

	for (long i = 0; i < fileSize; ++i)
	{
		char mapChar = buffer[i];

		if (mapChar == '\r')
			continue;

		if (mapChar == '\n')
		{
			map.emplace_back(currentRow);
			currentRow.clear();
		}
		else
		{
			currentRow += mapChar;
		}
	}

	if (!currentRow.empty())
	{
		map.emplace_back(currentRow);
	}

	// 모두 사용한 버퍼 해제
	delete[] buffer;
	buffer = nullptr;

	// 파일 닫기
	fclose(file);
	file = nullptr;

	if (map.empty())
		return;

	Vector2 mapSize = Vector2(map[0].size(), map.size());
	Renderer::Get().SetWorldSize(mapSize);
	Engine::Get().SetWorldSize(mapSize.x, mapSize.y);

	// 맵에 생성할 수 있는 적은 최대 9명
	std::vector<Vector2> patrolPointsArray[9];

	// 읽은 데이터를 기반으로 로직 제작
	for (int y = 0; y < static_cast<int>(map.size()); ++y)
	{
		for (int x = 0; x < static_cast<int>(map[y].size()); ++x)
		{
			//switch (map[y][x])
			//{
			//case 'E':
			//	SpawnActor<Enemy>(Vector2(
			//		static_cast<float>(x),
			//		static_cast<float>(y)
			//	));
			//	break;

			//case 'P':
			//	SpawnActor<Player>(Vector2(
			//		static_cast<float>(x),
			//		static_cast<float>(y)
			//	));
			//	break;
			//}

			if (map[y][x] == 'P')
			{
				SpawnActor<Player>(Vector2(
					static_cast<float>(x),
					static_cast<float>(y)
				));
			}
			else if(map[y][x] >= '1' && map[y][x] <= '9')
			{
				int enemyIndex = map[y][x] - '1';

				patrolPointsArray[enemyIndex].emplace_back(Vector2(
					static_cast<float>(x),
					static_cast<float>(y)
				));
			}
		}
	}

	for (const auto& patrolPoints : patrolPointsArray)
	{
		if (patrolPoints.size() == 0)
			break;

		int randomIndex = Util::RandomRange(0, patrolPoints.size() - 1);
		SpawnActor<Enemy>(patrolPoints[randomIndex], patrolPoints);
		leftEnemy++;
	}

	const int mapHeight = static_cast<int>(map.size());
	std::vector<std::vector<bool>> visited(mapHeight);

	for (int y = 0; y < mapHeight; ++y)
	{
		visited[y].resize(map[y].size(), false);
	}

	for (int y = 0; y < mapHeight; ++y)
	{
		const int mapWidth = static_cast<int>(map[y].size());

		for (int x = 0; x < mapWidth; ++x)
		{
			// 벽이 아니거나 이미 처리한 타일이면 넘어감
			if (map[y][x] != '#' || visited[y][x])
				continue;

			// ------------------------------------------
			// 3-1. 가로 길이 찾기
			// ------------------------------------------

			int width = 0;

			while (x + width < mapWidth &&
				map[y][x + width] == '#' &&
				!visited[y][x + width])
			{
				++width;
			}

			// ------------------------------------------
			// 3-2. 세로 방향으로 같은 폭이 계속되는지 확인
			// ------------------------------------------

			int height = 1;

			while (y + height < mapHeight)
			{
				// 다음 줄이 현재 폭보다 짧으면 종료
				if (static_cast<int>(map[y + height].size()) < x + width)
					break;

				bool canExtend = true;

				for (int checkX = x;
					checkX < x + width;
					++checkX)
				{
					if (map[y + height][checkX] != '#' ||
						visited[y + height][checkX])
					{
						canExtend = false;
						break;
					}
				}

				if (!canExtend)
					break;

				++height;
			}

			// ------------------------------------------
			// 3-3. 사용한 #들을 visited 처리
			// ------------------------------------------

			for (int wallY = y;
				wallY < y + height;
				++wallY)
			{
				for (int wallX = x;
					wallX < x + width;
					++wallX)
				{
					visited[wallY][wallX] = true;
				}
			}

			// ------------------------------------------
			// 3-4. 하나의 Wall 생성
			// ------------------------------------------

			SpawnActor<Wall>(
				Vector2(
					static_cast<float>(x),
					static_cast<float>(y)
				),
				width,
				height
			);
		}
	}
}
