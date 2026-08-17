//#include <Engine/Engine.h>
#include <Game/Game.h>
#include <Level/GameLevel.h>

int main()
{
	// 엔진 객체 생성 및 실행
	//Craft::Engine engine;
	//engine.AddNewLevel<GameLevel>();
	//engine.Run();

	// 창 제목 설정
	SetConsoleTitleA("Project S");

	Game game;
	game.Run();
}
