#pragma once

#include <Level/UILevel.h>


class MainLevel : public UILevel
{
public:
	MainLevel();

private:
	virtual void OnInitialized() override;
	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;
};

