#pragma once

#include <Level/UILevel.h>

class Credit : public UILevel
{
public:
	Credit();

private:
	virtual void OnInitialized() override;
	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;
};

