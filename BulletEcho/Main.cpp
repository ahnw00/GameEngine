//#include <Engine/Engine.h>
#include <Game/Game.h>
#include <Level/GameLevel.h>
#include <Utility/Timer.h>

int main()
{
	// 엔진 객체 생성 및 실행
	//Craft::Engine engine;
	//engine.AddNewLevel<GameLevel>();
	//engine.Run();

	// 창 제목 설정
	SetConsoleTitleA("Project S");
	// 현재 콘솔 창의 핸들을 가져옵니다.
	HWND hwnd = GetConsoleWindow();
	if (hwnd != NULL)
	{
		// 창을 최대화 상태로 변경합니다. (SW_MAXIMIZE = 3)
		ShowWindow(hwnd, SW_MAXIMIZE);
	}

	Game game;
	game.Run();
}
