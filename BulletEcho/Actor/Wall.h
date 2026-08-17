#pragma once

#include <Actor/Actor.h>

class Wall : public Craft::Actor
{
	TYPE_DECLARATIONS(Wall, Actor)

public:
	Wall(Craft::Vector2 position, int width, int height);

	// 이거 활성화하면 벽이 보여
	// DEBUGGING
	//virtual bool IsVisibleOutsideSight() const override { return true; }

private:

};

