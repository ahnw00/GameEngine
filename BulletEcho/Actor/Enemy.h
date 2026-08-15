#pragma once

#include <Actor/Actor.h>
#include <Actor/Character.h>
#include <memory>
#include <vector>
#include <Actor/DestroyEffect.h>

// DEBUGGING
constexpr auto MAP_WIDTH = 200;
constexpr auto MAP_HEIGHT = 100;


class Player;

class Enemy : public Character
{
	enum class Mode
	{
		None,
		Patrol, // 플레이어가 감지되지 않은 상태에선 순찰
		Trace,  // 플레이어 감지하면 공격 범위(사격 범위 or 근접 공격 범위)까지 추격
		Shoot,  // 사격
		Stab,   // 근접 공격
		Search  // 플레이어를 공격하다가 놓치면 일정 시간 동안 수색
	};

	enum class TurnDirection
	{
		None,
		Clockwise,
		CounterClockwise
	};

	struct AStarNode
	{
		AStarNode(
			const Craft::Vector2& position,
			int gCost,
			int hCost
		) : position(position), gCost(gCost), hCost(hCost)
		{}

		Craft::Vector2 position;

		int gCost = 0;
		int hCost = 0;

		inline int GetFCost() const
		{
			return gCost + hCost;
		}

		bool operator<(const AStarNode& other) const
		{
			return GetFCost() > other.GetFCost();
		}
	};

	struct Vector2Hash
	{
		size_t operator()(const Craft::Vector2& position) const
		{
			size_t x = std::hash<int>{}(position.x);
			size_t y = std::hash<int>{}(position.y);

			return x ^ (y << 1);
		}
	};

	TYPE_DECLARATIONS(Enemy, Character)

public:
	Enemy(Craft::Vector2 position);

private:
	// 이벤트 함수 오버라이드
	virtual void Tick(float deltaTime) override;

	void Patrol(const std::vector<Craft::Vector2>& patrolPoints, float deltaTime);

	void Trace(float deltaTime);

	void Shoot();

	void Stab();

	void Search(float deltaTime);

	void CalculatePathToTarget();

	void PlayEffect(const std::vector<DestroyEffect::EffectFrame>& sequence);

private:
	// A* 알고리즘
	std::vector<Craft::Vector2> FindPath(
		const Craft::Vector2& startPosition,
		const Craft::Vector2& targetPosition
	);

	void MoveAlongPath(float deltaTime);

	//bool CanMoveTo(const Craft::Vector2& checkPosition);

public:
	inline void SetTarget(Player* p) { target = p; }

private:
	// 순찰 위치들
	std::vector<Craft::Vector2> patrolPoints = 
	{
		Craft::Vector2(10, 10),
		Craft::Vector2(10, 20),
		Craft::Vector2(20, 20),
		Craft::Vector2(20, 10)
		//Craft::Vector2(10, 10)
	};
	int currentPatrolIndex = 0;

	// 플레이어 추격에 사용할 변수들
	// 플레이어까지의 경로 저장 변수
	std::vector<Craft::Vector2> path;
	// path의 인덱스
	int currentPathIndex = 0;
	// 경로 계산 성공/실패
	bool hasPath = false;

	// 순찰할때 회전 속도
	float rotateSpeed = 90.f;
	// 경로 탐색에 텀을 둠
	float pathUpdateTimer = 0.f;
	const float pathUpdateInterval = 0.2f;

	TurnDirection turnDirection = TurnDirection::None;
	float turnAngle = 0.f;
	float targetAngle = 0.f;

	float searchingTimer = 0.f;
	const float searchingDuration = 3.f;

	float searchMaxAngle = 60.f;
	float searchSpeed = 90.f; // 초당 회전 각도

	// Search할 때 기준이 될 벡터
	Craft::Vector2 searchForward = Craft::Vector2::Zero;

	// 이동해야하는 지점
	Craft::Vector2 targetPosition = Craft::Vector2::Zero;

	// 탐지한 플레이어
	Player* target = nullptr;
	Player* prevTarget = nullptr;

	// 현재 모드
	Mode mode = Mode::Patrol;

	const Craft::Vector2 directions[8] =
	{
		Craft::Vector2(0, -1),
		Craft::Vector2(1, -1),
		Craft::Vector2(1, 0),
		Craft::Vector2(1, 1),
		Craft::Vector2(0, 1),
		Craft::Vector2(-1, 1),
		Craft::Vector2(-1, 0),
		Craft::Vector2(-1, -1)
	};

	//const int MAP_WIDTH = 200;
	//const int MAP_HEIGHT = 100;
};

