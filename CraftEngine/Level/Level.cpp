#include "Level.h"

namespace Craft
{
	Level::Level()
	{
	}

	Level::~Level()
	{
	}

	void Level::OnInitialized()
	{
		// 초기화 됐다고 설정
		hasInitialized = true;
	}

	void Level::BeginPlay()
	{
		// 액터 초기화 시 1번 호출되는 이벤트
		for (const std::shared_ptr<Actor>& actor : actorList)
		{
			// 검증 - 이미 BeginPlay 처리된 경우 건너뛰기
			if (actor->HasBegunPlay())
				continue;

			// BeginPlay 이벤트 호출
			actor->BeginPlay();
		}
	}

	void Level::Tick(float deltaTime)
	{
		for (const std::shared_ptr<Actor>& actor : actorList)
		{
			// 검증 - 활성화되지 않았으면 건너뛰기
			if (!actor->IsActive())
				continue;

			// Tick 이벤트 호출
			actor->Tick(deltaTime);
		}
	}

	void Level::Draw()
	{
		for (const std::shared_ptr<Actor>& actor : actorList)
		{
			// 검증 - 활성화되지 않았으면 건너뛰기
			if (!actor->IsActive())
				continue;

			// Draw 이벤트 호출
			actor->Draw();
		}
	}

	void Level::ProcessAddAndDestroyActors()
	{
		// 액터 제거 처리
		// 이터레이터 기반 루프
		for (auto it = actorList.begin(); it != actorList.end(); )
		{
			// 제거 요청된 액터인지 확인
			auto actor = *it;
			if (actor->HasExpired())
			{
				it = actorList.erase(it);
				continue;
			}

			// 다음 순번을 처리하기 위해 증가 처리
			++it;
		}

		// 추가 처리
		// 추가 요청된 목록이 없으면 종료
		if (addResquestedActorList.empty())
			return;

		for (const auto& actor : addResquestedActorList)
		{
			actorList.emplace_back(actor);
		}

		// 추가 처리된 목록 정리
		addResquestedActorList.clear();
	}
	void Level::SavePrevActorStates()
	{
		// 액터 순회하면 이전 상태 저장 처리
		for (auto actor : actorList)
		{
			// 비활성화 상태면 무시
			if (!actor->IsActive())
				continue;

			// 상태 저장
			actor->SavePrevState();
		}
	}
}