#include "Game.h"

#include <Level/MainLevel.h>
#include <Level/GameLevel.h>
#include <Level/MenuLevel.h>
#include <Level/ClearLevel.h>
#include <Level/Credit.h>
#include <Level/DiedLevel.h>

#include <Render/Renderer.h>


using namespace Craft;

Game::Game()
{
	// 두 레벨 생성 및 배열에 추가
	levelList.emplace_back(std::make_shared<MainLevel>());
	levelList.emplace_back(std::make_shared<GameLevel>());
	levelList.emplace_back(std::make_shared<MenuLevel>());
	levelList.emplace_back(std::make_shared<ClearLevel>());
	levelList.emplace_back(std::make_shared<Credit>());
	levelList.emplace_back(std::make_shared<DiedLevel>());

	// 시작 상태 설정
	state = State::Main;

	// 게임 시작시 활성화할 레벨 설정
	mainLevel = levelList[(int)state];
}

void Game::ToggleMenu()
{
	int stateIdx = static_cast<int>(state);
	// 인덱스를 1->0, 0->1로 토글하는 공식
	int nextState = 1 - stateIdx;

	if (state == State::GamePlay)
		nextState = static_cast<int>(State::Menu);
	else if (state == State::Menu)
		nextState = static_cast<int>(State::GamePlay);

	// 레벨 설정 및 상태값 업데이트
	mainLevel = levelList[nextState];
	state = static_cast<State>(nextState);

	if (state == State::GamePlay)
	{
		Renderer::Get().SetRenderMode(Renderer::RenderMode::PLAY);
	}
	else if(state == State::Menu)
	{
		Renderer::Get().SetRenderMode(Renderer::RenderMode::MENU);
	}
}

void Game::GotoLevel(State newState)
{
	int nextState = static_cast<int>(newState);
	mainLevel = levelList[nextState];

	state = newState;

	if (state == State::GamePlay)
	{
		Renderer::Get().SetRenderMode(Renderer::RenderMode::PLAY);
		Engine::Get().PlayerBackgroundMusic(BgmList[1]);
	}
	else
	{
		Renderer::Get().SetRenderMode(Renderer::RenderMode::MENU);

		if(state == State::Main)
			Engine::Get().PlayerBackgroundMusic(BgmList[0]);
		else if(state == State::Clear)
			Engine::Get().PlayerBackgroundMusic(BgmList[2]);
	}
}

void Game::ResetGame()
{
	levelList[static_cast<int>(State::GamePlay)]
		= std::make_shared<GameLevel>();
}
