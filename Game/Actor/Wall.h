#pragma once

#include <Actor/Actor.h> // 얘는 Includes에 포함된 헤더

// 소코반 게임에서 벽을 담당
class Wall : public Craft::Actor // 엔진 코드를 Craft namespace로 감쳐둔걸 가져와서 쓴 거
{
	TYPE_DECLARATIONS(Wall, Actor)

public:
	Wall(const Craft::Vector2& position);
};

