#include "Animator.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <cassert>


using namespace Craft;

namespace fs = std::filesystem;

Animator::Animator(std::weak_ptr<Actor> owner)
	: ownerActor(owner)
{
	timer.SetTargetTime(animatorSpeed);

	auto actor = ownerActor.lock();

	if (!actor)
		return;

	int directionIndex = static_cast<int>(direction);
	int behaviorIndex = static_cast<int>(curBehavior);

	//actor->Set3DRenderImage(sprites[directionIndex][behaviorIndex][spriteIndex]);
}

void Craft::Animator::Tick(float deltaTime)
{
	timer.Tick(deltaTime);

	if (timer.IsTimeOut())
	{
		int directionIndex = static_cast<int>(direction);
		int behaviorIndex = static_cast<int>(curBehavior);

		// 하나의 행동을 몇 개의 프레임으로 표현하는지 가져오기
		int size = sprites[directionIndex][behaviorIndex].size();
		if (size == 0)
		{
			timer.Reset();
			spriteIndex = 0;
			return;
		}

		spriteIndex = (spriteIndex + 1) % size;

		auto actor = ownerActor.lock();

		if (actor && actor->CheckRender3DData())
		{
			actor->Set3DRenderImage(
				sprites[directionIndex][behaviorIndex][spriteIndex]
			);
		}

		timer.Reset();
	}
}

void Craft::Animator::LoadSprites(const std::string& basePath)
{
	//const std::string basePath = "../Assets/Sprites/Enemy";

	for (int dir = 0; dir < 8; ++dir)
	{
		// 순서: Idle, Walk, Attack, Search, Dead
		for (int anim = 0; anim < 5; ++anim)
		{
			std::string filePath =
				basePath + "/" +
				std::to_string(dir) + "/" +
				GetAnimationFolderName(anim) +
				"/Animation.txt";

			LoadAnimation(dir, anim, filePath);
		}
	}

	// 기본 애니메이션의 첫 프레임 설정
	int directionIndex = static_cast<int>(direction);
	int behaviorIndex = static_cast<int>(curBehavior);

	if (!sprites[directionIndex][behaviorIndex].empty())
	{
		spriteIndex = 0;

		if (auto actor = ownerActor.lock())
		{
			if (actor->CheckRender3DData())
			{
				actor->Set3DRenderImage(
					sprites[directionIndex][behaviorIndex][spriteIndex]
				);
			}
		}
	}
}

void Craft::Animator::LoadAnimation(int directionIndex, int animationIndex, const std::string& filePath)
{
	std::ifstream file(filePath);

	if (!file)
	{
		assert(false && "failed to open a stage file.");
		return;
	}

	std::string line;
	//std::vector<std::string> 
	sprite currentSprite;

	while (std::getline(file, line))
	{
		// 프레임 종료
		if (line == "END")
		{
			if (!currentSprite.empty())
			{
				size_t maxWidth = 0;

				for (const std::string& row : currentSprite)
				{
					maxWidth = max(maxWidth, row.size());
				}

				for (std::string& row : currentSprite)
				{
					row.resize(maxWidth, ' ');
				}

				sprites[directionIndex][animationIndex].emplace_back(currentSprite);
			}

			currentSprite.clear();
			continue;
		}

		// 실제 스프라이트 한 줄
		currentSprite.emplace_back(line);
	}

	file.close();
}

// 순서: Idle, Walk, Attack, Search, Dead
std::string Craft::Animator::GetAnimationFolderName(int animationIndex) const
{
	//return "Idle";

	switch (animationIndex)
	{
	case 0:
		return "Idle";

	case 1:
		return "Walk";

	case 2:
		return "Attack";

	case 3:
		return "Search";

	case 4:
		return "Dead";
	}

	return "Idle";
}

int Craft::Animator::GetDirectionIndex(Actor::Direction direction) const
{
	return 0;
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
