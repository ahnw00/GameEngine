#pragma once

#include <Actor/Actor.h>

class Sight : public Craft::Actor
{
	TYPE_DECLARATIONS(Sight, Actor)

public:
	Sight();

private:
	// 이벤트 함수 오버라이드
	virtual void Tick(float deltaTime) override;

private:
	// 부채꼴 모양의 시야 범위
	float radius = 0.f;
	float degree = 0.f;
};

