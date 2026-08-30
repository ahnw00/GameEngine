#include "CollisionSystem.h"
#include <Actor/Actor.h>
#include <cassert>



namespace Craft
{
	// 전역 변수 초기화 꼭 해주자
	CollisionSystem* CollisionSystem::instance = nullptr;

	CollisionSystem::CollisionSystem()
	{
		assert(!instance && "instance should be null");
		instance = this;
	}

	CollisionSystem::~CollisionSystem()
	{
		instance = nullptr;
	}
	
	void CollisionSystem::Build(
		const Vector2& worldSize, 
		const std::vector<std::shared_ptr<Actor>>& actorList)
	{
		this->worldSize = worldSize;

		const int width = static_cast<int>(worldSize.x);
		const int height = static_cast<int>(worldSize.y);

		collisionGrid.clear();
		collisionGrid.resize(width * height);

		for (const auto& actor : actorList)
		{
			if (!actor || !actor->IsActive())
				continue;

			AddActor(actor.get(), actor->GetPosition());
		}
	}

	void CollisionSystem::AddActor(Actor* actor, const Vector2& position)
	{
		if (!actor || !actor->IsActive())
			return;

		AddToGrid(actor, position);
	}

	void CollisionSystem::RemoveActor(Actor* actor, const Vector2& position)
	{
		if (!actor) return;

		RemoveFromGrid(actor, position);
	}
	
	void CollisionSystem::ProcessCollision(
		const std::vector<std::shared_ptr<Actor>>& actorList)
	{
		//// 예외처리.
		//if (actorList.empty())
		//{
		//	return;
		//}

		//// 충돌한 액터에 이벤트를 한번에 정리해 전달하기 위한 배열.
		//std::vector<CollisionPair> collidedActorList;

		//// 레벨에 배치된 액터 수.
		//const int count = static_cast<int>(actorList.size());

		//// 모든 액터를 순회하면서 충돌 검사.
		//for (int ix = 0; ix < count; ++ix)
		//{
		//	const std::shared_ptr<Actor>& left = actorList[ix];
		//	if (!left || !left->IsActive())
		//	{
		//		continue;
		//	}

		//	for (int jx = ix + 1; jx < count; ++jx)
		//	{
		//		const std::shared_ptr<Actor>& right = actorList[jx];
		//		if (!right || !right->IsActive())
		//		{
		//			continue;
		//		}

		//		// 충돌 검사.
		//		if (Test(left, right))
		//		{
		//			// 이벤트 발행할 목록에 추가할 데이터 생성.
		//			CollisionPair pair = {};
		//			pair.actor = left;
		//			pair.other = right;

		//			// 목록에 추가.
		//			collidedActorList.emplace_back(pair);
		//		}
		//	}
		//}

		// 충돌 발생한 액터 목록 확인. 충돌한 액터가 없으면 함수 종료.
		if (collidedActorList.empty())
		{
			return;
		}

		// 충돌한 액터에 이벤트 전달.
		for (const CollisionPair& pair : collidedActorList)
		{
			// 이미 삭제되거나 비활성화된 액터는 제외.
			if (!pair.actor->IsActive() || !pair.other->IsActive())
			{
				continue;
			}

			// 충돌 이벤트 전달.
			pair.actor->OnCollision(pair.other);
			pair.other->OnCollision(pair.actor);
		}

		collidedActorList.clear();
	}

	bool CollisionSystem::Test(
		const std::shared_ptr<Actor>& left,
		const std::shared_ptr<Actor>& right)
	{
		if (!left || !right)
		{
			return false;
		}

		// AABB (Axis Aligned Bounding Box).
		// y는 크기가 1이기 때문에 x좌표만 고려.

		// left 액터의 현재/이전 위치.
		const Vector2 leftCurrent = left->GetPosition();
		const Vector2 leftPrevious = left->GetPrevPosition();

		// right 액터의 현재/이전 위치.
		const Vector2 rightCurrent = right->GetPosition();
		const Vector2 rightPrevious = right->GetPrevPosition();

		// 이전 프레임 위치와 현재 위치를 모두 포함하는 swept bounds 계산.
		
		// Left Actor의 x 최대/최소값
		const float leftXMin = (leftCurrent.x < leftPrevious.x) ? leftCurrent.x : leftPrevious.x;
		const float leftXMaxCurrent = leftCurrent.x + left->GetWidth() - 1;
		const float leftXMaxPrevious = leftPrevious.x + left->GetWidth() - 1;
		const float leftXMax = (leftXMaxCurrent > leftXMaxPrevious) ? leftXMaxCurrent : leftXMaxPrevious;

		// Left Actor의 y 최대/최소값
		const float leftYMin = (leftCurrent.y < leftPrevious.y) ? leftCurrent.y : leftPrevious.y;
		const float leftYMaxCurrent = leftCurrent.y + left->GetHeight() - 1;
		const float leftYMaxPrevious = leftPrevious.y + left->GetHeight() - 1;
		const float leftYMax = (leftYMaxCurrent > leftYMaxPrevious) ? leftYMaxCurrent : leftYMaxPrevious;

		// Right Actor의 x 최대/최소값
		const float rightXMin = (rightCurrent.x < rightPrevious.x) ? rightCurrent.x : rightPrevious.x;
		const float rightXMaxCurrent = rightCurrent.x + right->GetWidth() - 1;
		const float rightXMaxPrevious = rightPrevious.x + right->GetWidth() - 1;
		const float rightXMax = (rightXMaxCurrent > rightXMaxPrevious) ? rightXMaxCurrent : rightXMaxPrevious;

		// Right Actor의 y 최대/최소값
		const float rightYMin = (rightCurrent.y < rightPrevious.y) ? rightCurrent.y : rightPrevious.y;
		const float rightYMaxCurrent = rightCurrent.y + right->GetHeight() - 1;
		const float rightYMaxPrevious = rightPrevious.y + right->GetHeight() - 1;
		const float rightYMax = (rightYMaxCurrent > rightYMaxPrevious) ? rightYMaxCurrent : rightYMaxPrevious;

		// X좌표 기준으로 충돌이 발생할 수 없는 상황 처리.
		if (rightXMin > leftXMax)
		{
			return false;
		}

		if (rightXMax < leftXMin)
		{
			return false;
		}

		// Y좌표 기준으로 충돌이 발생할 수 없는 상황 처리.
		if (rightYMin > leftYMax)
		{
			return false;
		}

		if (rightYMax < leftYMin)
		{
			return false;
		}


		// 충돌 발생.
		return true;
	}

	// 충돌 테스트
	bool CollisionSystem::Test(Actor* actor, const Vector2& position)
	{
		if (!actor) return false;

		int startX = position.x;
		int startY = position.y;

		int endX = startX + actor->GetWidth() - 1;
		int endY = startY + actor->GetHeight() - 1;

		for (int x = startX; x <= endX; ++x)
		{
			if (x < 0 || x >= worldSize.x)
				continue;

			for (int y = startY; y <= endY; ++y)
			{
				if (y < 0 || y >= worldSize.y)
					continue;

				const int index = GetIndex(x, y);

				for (Actor* other : collisionGrid[index])
				{
					if (!other) continue;
					// 자기 자신
					if (other == actor) continue;

					// 충돌 발생
					CollisionPair pair;

					pair.actor = actor->shared_from_this();
					pair.other = other->shared_from_this();

					collidedActorList.emplace_back(std::move(pair));

					return true;
				}
			}
		}

		// 여기까지 왔으면 충돌 발생X
		return false;
	}
	
	void CollisionSystem::BuildCollisionMap()
	{
	}

	void CollisionSystem::UpdateActor(
		const Vector2& prevPos, 
		const Vector2& curPos, 
		Actor* actor)
	{
		if (!actor) return;

		if (prevPos == curPos)
			return;

		RemoveFromGrid(actor, prevPos);
		AddToGrid(actor, curPos);
	}

	CollisionSystem& CollisionSystem::Get()
	{
		assert(instance && "instance should not be null");
		return *instance;
	}
	
	void CollisionSystem::AddToGrid(Actor* actor, const Vector2& position)
	{
		if (!actor) return;

		int startX = static_cast<int>(position.x);
		int startY = static_cast<int>(position.y);

		int endX = startX + actor->GetWidth() - 1;
		int endY = startY + actor->GetHeight() - 1;

		int width = static_cast<int>(worldSize.x);
		int height = static_cast<int>(worldSize.y);

		for (int y = startY; y <= endY; ++y)
		{
			for (int x = startX; x <= endX; ++x)
			{
				if (x < 0 || x >= width || y < 0 || y >= height)
					continue;

				collisionGrid[GetIndex(x, y)].emplace_back(actor);
			}
		}
	}
	
	void CollisionSystem::RemoveFromGrid(Actor* actor, const Vector2& position)
	{
		if (!actor) return;

		const int startX = static_cast<int>(position.x);
		const int startY = static_cast<int>(position.y);

		const int endX = startX + actor->GetWidth() - 1;
		const int endY = startY + actor->GetHeight() - 1;

		const int width = static_cast<int>(worldSize.x);
		const int height = static_cast<int>(worldSize.y);

		for (int y = startY; y <= endY; ++y)
		{
			for (int x = startX; x <= endX; ++x)
			{
				if (x < 0 || x >= width || y < 0 || y >= height)
					continue;

				const int index = GetIndex(x, y);

				auto& cell = collisionGrid[index];

				//if (collisionGrid[index] == actor)
				//{
				//	collisionGrid[index] = nullptr;
				//}
				cell.erase(std::remove(cell.begin(), cell.end(), actor), cell.end());
			}
		}
	}
}