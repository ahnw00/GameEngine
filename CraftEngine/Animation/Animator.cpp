#include "Animator.h"
#include <iostream>

using namespace Craft;

Animator::Animator(std::weak_ptr<Actor> owner)
	: ownerActor(owner)
{
	timer.SetTargetTime(animatorSpeed);

	auto actor = ownerActor.lock();

	if (!actor)
		return;

	int directionIndex = static_cast<int>(direction);
	int behaviorIndex = static_cast<int>(curBehavior);

	actor->Set3DRenderImage(
		sprites[directionIndex][behaviorIndex][spriteIndex]
	);
}

void Craft::Animator::Tick(float deltaTime)
{
	timer.Tick(deltaTime);
	std::cout
		<< "Animator Tick"
		<< " / deltaTime: " << deltaTime
		<< " / timeout: " << timer.IsTimeOut()
		<< std::endl;

	if (timer.IsTimeOut())
	{
		int directionIndex = static_cast<int>(direction);
		int behaviorIndex = static_cast<int>(curBehavior);

		int size = sprites[directionIndex][behaviorIndex].size();
		if (size == 0)
			return;

		spriteIndex = (spriteIndex + 1) % size;

		std::cout << "SpriteIndex: " << spriteIndex << "\n";
		std::cout << "Timeout: " << timer.IsTimeOut() << std::endl;
		std::cout
			<< "Behavior: " << behaviorIndex
			<< " / Sprite Index: " << spriteIndex
			<< " / Size: " << size
			<< std::endl;

		std::shared_ptr<Actor> actor = ownerActor.lock();
		if (!actor)
			return;

		if (!actor->CheckRender3DData())
			return;

		actor->Set3DRenderImage(sprites[directionIndex][behaviorIndex][spriteIndex]);

		timer.Reset();
	}
}

void Animator::SetDirection(Actor::Direction newDirection)
{
	if (direction == newDirection)
		return;

	direction = newDirection;
}

void Craft::Animator::SetAnimationMode(AnimationMode newMode)
{
	if (curBehavior == newMode)
		return;

	curBehavior = newMode;
}
