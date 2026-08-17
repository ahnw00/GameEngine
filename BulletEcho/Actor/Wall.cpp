#include "Wall.h"

#include <string>
#include <vector>

using namespace Craft;

Wall::Wall(Vector2 position, int width, int height)
	: Actor({"#"}, position, Color::Green)
{
	std::string temp = "";

	while (width--)
		temp += '#';

	std::vector<std::string> wallImage(height, temp);
	
	ChangeImage(wallImage);
}
