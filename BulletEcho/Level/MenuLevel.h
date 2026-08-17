#pragma once

#include <Level/UILevel.h>


class MenuLevel : public UILevel
{
public:
	MenuLevel();

private:
	// 이벤트 함수 오버라이드
	virtual void OnInitialized() override;
	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;
};

