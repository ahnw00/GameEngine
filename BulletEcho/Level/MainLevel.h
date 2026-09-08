#pragma once

#include <Level/UILevel.h>
#include <Utility/Timer.h>


class MainLevel : public UILevel
{
public:
	MainLevel();

private:
	virtual void OnInitialized() override;
	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;

private:
	void MakeCone();
	void MakeMatrix();

private:
	std::vector<std::string> background;

	int timerIndex = 0;

	const int timerLength = 4;

	float timers[4] = { 0.8f, 1.f, 1.5f, 7.f };

	float matrixTimer = 0.f;
	float matrixInterval = 1.f;

	Craft::Timer timer;
};

