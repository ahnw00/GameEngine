#pragma once

#include <Core/Core.h> // 외부에서 접근해주기 위해
#include <Math/Vector2.h>
#include <vector>
#include <memory>

namespace Craft
{
	// 전방 선언.
	class Actor;

	// 액터 간 충돌을 테스트하는 클래스.
	class CRAFT_API CollisionSystem
	{
		// 충돌 이벤트 발생을 위해 액터를 저장할 구조체.
		struct CollisionPair
		{
			std::shared_ptr<Actor> actor;
			std::shared_ptr<Actor> other;
		};

	public:
		CollisionSystem();
		~CollisionSystem();

		void Build(
			const Vector2& worldSize,
			const std::vector<std::shared_ptr<Actor>>& actorList
		);

		void AddActor(Actor* actor, const Vector2& position);
		void RemoveActor(Actor* actor, const Vector2& position);

		// 액터를 순회하면서 충돌을 확인하는 함수.
		void ProcessCollision(const std::vector<std::shared_ptr<Actor>>& actorList);

		// 두 액터가 충돌했는지 확인(테스트)하는 함수.
		bool Test(
			const std::shared_ptr<Actor>& left,
			const std::shared_ptr<Actor>& right
		);

		bool Test(Actor* actor, const Vector2& position);

		// 액터 순회하면서 콜리전 맵 구성
		// 처음 한번만 실행
		void BuildCollisionMap();

		void UpdateActor(
			const Vector2& prevPos,
			const Vector2& curPos,
			Actor* actor
		);

		static CollisionSystem& Get();

	private:
		inline int GetIndex(int x, int y) const
		{
			return (y * static_cast<int>(worldSize.x) + x);
		}

		void AddToGrid(Actor* actor, const Vector2& position);
		void RemoveFromGrid(Actor* actor, const Vector2& position);

	//private:
	//	Bounds GetBounds(const Actor& actor)
	//	{
	//		Vector2 pos = actor.GetPosition();

	//		return { pos.x, pos.y,
	//		pos.x + actor.GetWidth() - 1,
	//		pos.y + actor.GetHeight() - 1 };
	//	}

	private:
		// 전역 접근 가능하도록(싱글톤)
		static CollisionSystem* instance;

		Vector2 worldSize = Vector2::Zero;

		// 동적 콜리전 저장 맵
		std::vector<std::vector<Actor*>> collisionGrid;

		// 충돌한 액터쌍 저장 리스트
		std::vector<CollisionPair> collidedActorList;
	};
}