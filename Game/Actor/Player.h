#pragma once

#include <Actor/Actor.h>

class Player : public Craft::Actor
{
	TYPE_DECLARATIONS(Player, Actor)

public:
	Player(const Craft::Vector2& position);

private:
	virtual void Tick(float deltaTime) override;
};

