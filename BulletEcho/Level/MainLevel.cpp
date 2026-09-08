#include "MainLevel.h"
#include <Game/Game.h>
#include <Input/Input.h>
#include <Render/Renderer.h>
#include <Utility/Utility.h>

#include <cmath>
#include <string>


using namespace Craft;



MainLevel::MainLevel()
{
	menuTitle = "PROJECT S";

	itemList.emplace_back(
		std::make_unique<UIItem>(
			"-Start",
			[]()
			{
				// 메뉴 토글 함수 호출
				Game& game = dynamic_cast<Game&>(Engine::Get());
				game.GotoLevel(State::GamePlay);
			}
		)
	);

	itemList.emplace_back(
		std::make_unique<UIItem>(
			"-Quit Game",
			[]()
			{
				// 게임 종료 호출
				Engine::Get().Quit();
			}
		)
	);

    //MakeCone();

    timer.SetTargetTime(timers[0]);
}

void MainLevel::OnInitialized()
{
	UILevel::OnInitialized();

	// 메인메뉴 브금
	Engine::Get().PlayerBackgroundMusic("cyberwave.wav");
}

void MainLevel::Tick(float deltaTime)
{
	UILevel::Tick(deltaTime);

    timer.Tick(deltaTime);

    if (timer.IsTimeOut())
    {
        timerIndex = (timerIndex + 1) % timerLength;
        timer.SetTargetTime(timers[timerIndex]);

        if (timerIndex % 2 == 0)
        {
            //Engine::Get().PlayerOneShot("lightbulb.wav");
            Engine::Get().PlayerOneShot("keyboard.wav");
        }
        if (timerIndex == 1)
        {
            timers[timerIndex] = Util::RandomRange(1.f, 2.f);
        }
    }

    if (matrixTimer < matrixInterval)
    {
        matrixTimer += deltaTime;
    }
    else
    {
        MakeMatrix();
        matrixTimer = 0.f;
    }
}

void MainLevel::Draw()
{
    //if (timerIndex % 2 == 0)
    //    return;

    // 배경
    Renderer::Get().Submit(
        nullptr,
        background,
        Vector2(0, 0),
        Color::Green,
        0
    );

	UILevel::Draw();
}

void MainLevel::MakeCone()
{
    const int width = Engine::Get().GetWidth();
    const int height = Engine::Get().GetHeight();

    const int centerY = height / 2;

    background.resize(height);

    for (int y = 0; y < height; ++y)
    {
        // 중앙에서 얼마나 떨어져 있는지
        int distanceFromCenter = std::abs(y - centerY);

        // 화면 오른쪽 끝에서 허용되는 최대 높이
        int maxDistance = centerY;

        // 현재 y가 삼각형 내부에 들어오는 x 시작 위치
        int startX = 0;

        if (maxDistance > 0)
        {
            float ratio =
                static_cast<float>(distanceFromCenter) /
                static_cast<float>(maxDistance);

            startX = static_cast<int>(ratio * width);
        }

        // 삼각형 바깥
        if (startX >= width)
        {
            background[y] = "";
            continue;
        }

        // startX ~ 화면 끝까지 채우기
        background[y] = std::string(width - startX, static_cast<char>(219));
    }
}

void MainLevel::MakeMatrix()
{
    const int width = Engine::Get().GetWidth();
    const int height = Engine::Get().GetHeight();

    background.resize(height);


    for (int j = 0; j < height; ++j)
    {
        std::string str = "";

        for (int i = 0; i < width; ++i)
        {
            int randNum = Util::RandomRange(0, 20);
            if (randNum > 9)
                str += " ";
            else
                str += std::to_string(randNum);
        }
        
        background[j] = str;
    }
}
