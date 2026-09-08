//#include <Engine/Engine.h>
#include <Game/Game.h>
#include <Level/GameLevel.h>
#include <Utility/Timer.h>

#include <Windows.h>

void SetConsoleFullScreen()
{
    HWND consoleWindow = GetConsoleWindow();

    LONG style = GetWindowLong(consoleWindow, GWL_STYLE);

    // 타이틀바, 테두리 제거
    style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZEBOX | WS_SYSMENU);

    SetWindowLong(consoleWindow, GWL_STYLE, style);

    // 모니터 전체 영역 가져오기
    HMONITOR monitor = MonitorFromWindow(consoleWindow, MONITOR_DEFAULTTONEAREST);

    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(monitor, &monitorInfo);

    RECT rect = monitorInfo.rcMonitor;

    SetWindowPos(
        consoleWindow,
        HWND_TOP,
        rect.left,
        rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top,
        SWP_FRAMECHANGED | SWP_SHOWWINDOW
    );
}

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
