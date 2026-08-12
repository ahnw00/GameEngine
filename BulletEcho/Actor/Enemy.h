#pragma once

#include <Actor/Actor.h>
#include <Actor/Character.h>

class Enemy : public Character
{
	TYPE_DECLARATIONS(Enemy, Character)

public:
	Enemy();

private:
	// 이벤트 함수 오버라이드
	virtual void Tick(float deltaTime) override;
};

