#include "GameLevel.h"

#include <Actor/Player.h>
#include <Actor/Enemy.h>
#include <Actor/Wall.h>
#include <Actor/Bullet.h>
#include <Render/Renderer.h>
#include <Input/Input.h>
#include <Engine/Engine.h>

#include <cassert>


using namespace Craft;

void GameLevel::OnInitialized()
{
	Level::OnInitialized();

	Engine::Get().PlayerBackgroundMusic("bgm.wav");

	LoadMap("BulletEchoMap2.txt");

	//// 플레이어 액터 추가
	//SpawnActor<Player>(Vector2::Zero);

	//// 적 생성기 액터 추가
	//SpawnActor<Enemy>(Vector2::Zero);
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
		if (currentMode == Renderer::RenderMode::PLAY &&
			!actor->IsTypeOf<Player>())
			continue;

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

			Renderer::Get().SetSight(bulletPosition);
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

	// 읽은 데이터를 기반으로 로직 제작
	// 1. 화면에 액터를 그리기

	// 문자열에 저장된 값에 접근할 때 사용할 인덱스
	int index = 0;

	// 액터 생성에 사용할 위치 값
	Vector2 position;
	while (true)
	{
		// 종료 조건 - 내용을 모두 읽었는지 파악
		if (index >= fileSize)
			break;

		// 이번에 확인할 문자 값
		char mapChar = buffer[index];

		// 인덱스 증가 처리
		index++;

		// 개행 문자라면 로직은 액터 생성 로직은 건너뛰고
		// 위치 값만 설정
		if (mapChar == '\n')
		{
			++position.y;
			position.x = 0;
			continue;
		}

		switch (mapChar)
		{
			// 벽
		case '#':
			SpawnActor<Wall>(position);
			break;

			// 적
		case 'E':
			SpawnActor<Enemy>(position);
			break;

			// 플레이어
		case 'P':
			SpawnActor<Player>(position);
			break;
		}

		// x 위치 업데이트
		++position.x;
	}

	// 모두 사용한 버퍼 해제
	delete[] buffer;
	buffer = nullptr;

	// 파일 닫기
	fclose(file);
	file = nullptr;
}
