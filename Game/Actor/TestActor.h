#pragma once

#include <Actor/Actor.h>

// 테스트 액터
class TestActor : public Craft::Actor
{
public:
	TestActor();
	virtual void Tick(float deltaTime) override;
};