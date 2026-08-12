#include "Character.h"
#include <Util/Sight.h>

Character::Character(
	const std::vector<std::string>& sprite, 
	Craft::Vector2 position, 
	Craft::Color color
) : Actor(sprite, position, color)
{ }

Character::~Character()
{
}

void Character::Tick(float deltaTime)
{
	super::Tick(deltaTime);
}

Sight* Character::GetSight()
{
	if(!sight)
		return nullptr;

	return sight.get();
}
