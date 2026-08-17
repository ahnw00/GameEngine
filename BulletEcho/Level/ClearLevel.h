#pragma once

#include <Level/UILevel.h>

class ClearLevel : public UILevel
{
public:
	ClearLevel();

private:
	// 이벤트 함수 오버라이드
	virtual void OnInitialized() override;
	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;

public:
	inline void SetClearTime(float elapsedTime) { clearTime = elapsedTime; }

private:
	float clearTime = 0.f;
};

