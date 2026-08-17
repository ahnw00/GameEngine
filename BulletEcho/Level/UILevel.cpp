#include "UILevel.h"
#include <Game/Game.h>
#include <Input/Input.h>
#include <Render/Renderer.h>

#include <cassert>


using namespace Craft;

UILevel::UILevel()
{
	Renderer::Get().SetRenderMode(Renderer::RenderMode::MENU);
}

void UILevel::OnInitialized()
{
	Level::OnInitialized();

	//Engine::Get().PlayerBackgroundMusic("bgm.wav");
}

void UILevel::Tick(float deltaTime)
{
	Level::Tick(deltaTime);

	// 배열의 요소 개수
	const int length = static_cast<int>(itemList.size());
	if (Input::Get().GetKeyDown('W'))
	{
		// 인덱스 돌리기 (-방향)
		currentIdx = (currentIdx - 1 + length) % length;
	}
	if (Input::Get().GetKeyDown('S'))
	{
		// 인덱스 돌리기 (+방향)
		currentIdx = (currentIdx + 1) % length;
	}

	// 엔터 입력 처리 -> 현재 선택된 메뉴의 로직 실행
	if (Input::Get().GetKeyDown(VK_RETURN) ||
		Input::Get().GetKeyDown(VK_SPACE))
	{
		if (itemList.size() == 0)
			return;

		assert(currentIdx >= 0
			&& currentIdx < (int)itemList.size()
			&& itemList[currentIdx]->onSelected
		);

		// 메뉴 아이템에 저장된 로직 실행
		itemList[currentIdx]->onSelected();
		currentIdx = 0;
	}
}

void UILevel::Draw()
{
	int tileLength = menuTitle.length();
	int itemCount = itemList.size();

	Vector2 screenSize = Renderer::Get().GetScreenSize();
	Vector2 center = Vector2(
		screenSize.x / 2 - tileLength / 2,
		screenSize.y / 2 - itemCount / 2
	);

	// 제목 그리기
	Renderer::Get().Submit(nullptr, { menuTitle }, center);

	// 메뉴 아이템 그리기
	const int count = static_cast<int>(itemList.size());
	for (int i = 0; i < count; ++i)
	{
		// 선택/미선택된 아이템 색상 처리
		Color textColor = (i == currentIdx) ? selectedColor : unselectedColor;

		// 아이템 그리기
		Renderer::Get().Submit(
			nullptr,
			{ itemList[i]->text },
			Vector2(center.x, center.y + 2 + i),
			textColor
		);
	}
}
