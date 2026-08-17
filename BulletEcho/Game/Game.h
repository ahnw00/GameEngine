#pragma once

#include <Engine/Engine.h>
#include <vector>

// 레벨 관리에 사용할 상태 열거형
enum class State
{
	Main = 0,
	GamePlay = 1,
	Menu = 2,
	Clear = 3,
	Credit = 4,
	Died = 5,
	Length // 이거 꿀팁까지 아닌 팁
};

// 메뉴 레벨 및 게임 레벨을 관리하는 객체
class Game : public Craft::Engine
{
public:
	Game();
	~Game() = default;

	// 메뉴/게임 레벨을 전환하는 함수
	void ToggleMenu();

	// 스타트 레벨 -> 게임 레벨
	void GotoLevel(State newState);

	void ResetGame();

	inline const std::vector<std::shared_ptr<Craft::Level>>& GetLevelList()
	{
		return levelList;
	}

private:
	// 메뉴 레벨과 게임 레벨을 관리할 배열
	std::vector<std::shared_ptr<Craft::Level>> levelList;

	// 현재 활성화된 레벨의 상태를 나타내는 변수
	State state = State::GamePlay;

	std::string BgmList[3] = { "mainbgm.wav", "bgm.wav", "clearbgm.wav" };
};

