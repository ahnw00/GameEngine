#pragma once

#include <Level/UILevel.h>

class DiedLevel : public UILevel
{
public:
	DiedLevel();

private:
	virtual void OnInitialized() override;
	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;
};

