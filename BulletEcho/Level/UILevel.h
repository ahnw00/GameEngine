#pragma once

#include <Level/Level.h>

// 메뉴 아이템 구조체
struct UIItem
{
	// 메뉴 선택 시 실행할 로직 저장을 위한 함수 포인터
	//typedef void (*OnSelected)();
	using OnSelected = void(*)();

	// 생성자
	UIItem(const std::string& text, OnSelected onSelected)
		: text(text), onSelected(onSelected)
	{
	}

	// 메뉴 텍스트
	std::string text;

	// 메뉴를 선택했을 때 실행할 로직
	OnSelected onSelected = nullptr;
};

class UILevel : public Craft::Level
{
public:
	UILevel();

protected:
	// 이벤트 함수 오버라이드
	virtual void OnInitialized() override;
	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;
	virtual void ProcessPlayerSight() override { return; };

protected:
	// 현재 활성화된 메뉴 아이템 인덱스
	int currentIdx = 0;

	// 선택된 메뉴 아이템의 색상
	Craft::Color selectedColor = Craft::Color::Green;

	// 미선택된 메뉴 아이템의 색상
	Craft::Color unselectedColor = Craft::Color::White;

	// 메뉴 아이템 배열
	std::vector<std::unique_ptr<UIItem>> itemList;

	std::string menuTitle = "";
};

