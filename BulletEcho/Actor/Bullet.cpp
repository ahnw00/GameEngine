#include "Bullet.h"

#include <Engine/Engine.h>
#include <Render/Renderer.h>

using namespace Craft;

Bullet::Bullet(const Vector2& position, const Vector2& direction)
	: Actor({ "*" }, position, Color::Yellow),
	direction(direction), position(position)
{}

void Bullet::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	Vector2 newPosition = GetPosition() + (direction * moveSpeed * deltaTime);

	int xLimit = Engine::Get().GetWidth();
	int yLimit = Engine::Get().GetHeight();

	if (newPosition.x < 0.f || newPosition.x >= xLimit)
		Destroy();
	if (newPosition.y < 0.f || newPosition.y >= yLimit)
		Destroy();

	SetPosition(newPosition);

	//std::string temp = "bullet pos: " + 
	//	std::to_string(newPosition.x) + 
	//	", " + 
	//	std::to_string(newPosition.y);

	//Renderer::Get().Submit(
	//	temp,
	//	Vector2(20, 1)
	//);
}
