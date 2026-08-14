#pragma once

#include <Actor/Actor.h>

class Wall : public Craft::Actor
{
	TYPE_DECLARATIONS(Wall, Actor)

public:
	Wall(Craft::Vector2 position);

	// 이거 활성화하면 벽도 시야 밖으로 가도 보여
	// DEBUGGING
	//virtual bool IsVisibleOutsideSight() const override { return true; }

private:

};

