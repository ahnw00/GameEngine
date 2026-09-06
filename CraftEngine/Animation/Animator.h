#pragma once

#include <Core/Core.h>
#include <Math/Vector2.h>
#include <Actor/Actor.h>
#include <Utility/Timer.h>
#include <vector>
#include <string>
#include <memory>


namespace Craft
{
	class CRAFT_API Animator
	{
	public:
		enum class AnimationMode
		{
			Idle,
			Walk,
			Attack,
			Search,
			Dead
		};

	public:
		Animator(std::weak_ptr<Actor> owner);

		void Tick(float deltaTime);

        void LoadSprites(const std::string& basePath);

		void SetDirection(Craft::Actor::Direction newDirection);
		void SetAnimationMode(AnimationMode newMode);

    private:
        void LoadAnimation(
            int directionIndex, int animationIndex,
            const std::string& filePath
        );
        std::string GetAnimationFolderName(int animationIndex) const;
        int GetDirectionIndex(Actor::Direction direction) const;

	private:
		// 화면에 표시해야할 방향
		Craft::Actor::Direction direction = Craft::Actor::Direction::N;

		AnimationMode curBehavior = AnimationMode::Idle;

		// 현재 방향에 맞는 스프라이트 리스트
		// std::vector<std::string>> 이게 sprite 하나
		// std::vector<sprite> 해당 방향에서 프레임별 스프라이트 모아둔거
		// 순서: N, NE, E, SE, S, SW, W, NW
		// 순서: Idle, Walk, Attack, Search, Dead
		// 만약 현재 걷는 중이라면 8방향의 걷는 이미지들이 다 들어있어야해
		// direction, behavior, index
        using sprite = std::vector<std::string>;
        std::vector<sprite> sprites[8][5] = { };

		int spriteIndex = 0;

		std::weak_ptr<Actor> ownerActor;

		Timer timer;

		float animatorSpeed = 0.2f;
	};
}

