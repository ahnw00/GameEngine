#include "Enemy.h"

#include <Actor/Player.h>
#include <Util/Sight.h>
#include <Engine/Engine.h>
#include <Render/Renderer.h>
#include <Level/GameLevel.h>

#include <queue>
#include <cmath>


using namespace Craft;

Enemy::Enemy(
	Vector2 position,
	const std::vector<Vector2>& patrolPoints
) : Character({ " * ", "***", " * " }, position, Color::Grey),
	patrolPoints(patrolPoints)
{
	// 캐릭터 타입 설정
	SetCharacterType(Character::Type::Enemy);

	// 공격력 세팅
	SetAttackPower(20.f);

	moveSpeed = 10.f;

	// 시야 세팅
	sight = std::make_unique<Sight>(this);
	sight->SetRadius(20.f);
	sight->SetDegree(30.f);

	SetPosition(position);

	// x, y 위치 저장
	xPosition = position.x;
	yPosition = position.y;

	// 연사 타이머 시간 설정
	fireInterval = 0.5f;
	timer.SetTargetTime(fireInterval);

	// 임시 방향
	forward = Vector2(0, 1);
}

void Enemy::Tick(float deltaTime)
{
	super::Tick(deltaTime);
	sight->Tick(deltaTime);
	timer.Tick(deltaTime);

	if (target != nullptr && prevTarget == nullptr)
	{
		// 이 블록은 타겟을 발견한 딱 1프레임만 실행

		// 1. 느낌표(!) 이펙트 등 재생
		const std::vector<DestroyEffect::EffectFrame> sequence =
		{
			{ {"!"}, 0.5f, Color::Yellow }
		};
		PlayEffect(sequence);
	}

	if (target)
	{
		float distance = (GetCenterPosition() - target->GetCenterPosition()).size();

		if (distance >= 1.5f && distance < 15.f)
			mode = Mode::Shoot;
		else if (distance < 1.5f)
			mode = Mode::Stab;
		else
			mode = Mode::Trace;
	}
	else if ((mode == Mode::Shoot || mode == Mode::Trace))
	{
		if (!target)
		{
			const std::vector<DestroyEffect::EffectFrame> sequence =
			{
				{ {"?"}, 0.5f, Color::Yellow }
			};
			PlayEffect(sequence);

			mode = Mode::Search;
			searchForward = forward;
		}
	}
	else if (mode == Mode::Search)
	{
		if (searchingTimer > searchingDuration)
		{
			mode = Mode::Patrol;

			// 추격이 끝났으니 추격용 변수 초기화
			path.clear();
			currentPathIndex = 0;
			hasPath = false;

			searchingTimer = 0.f;
		}
	}

	if (target)
	{
		targetPosition = target->GetCenterPosition();
		forward = (targetPosition - GetCenterPosition()).normalized();
	}	
	
	switch (mode)
	{
	case Enemy::Mode::None:
		break;
	case Enemy::Mode::Patrol:
		Patrol(patrolPoints, deltaTime);
		break;
	case Enemy::Mode::Trace:
		Trace(deltaTime);
		break;
	case Enemy::Mode::Shoot:
		Shoot();
		break;
	case Enemy::Mode::Stab:
		Stab();
		break;
	case Enemy::Mode::Search:
		Search(deltaTime);
		break;
	default:
		break;
	}

	prevTarget = target;
}

void Enemy::Patrol(const std::vector<Vector2>& patrolPoints, float deltaTime)
{
	if (patrolPoints.size() < 2)
		return;

	const int pointCount = static_cast<int>(patrolPoints.size());

	// ----------------------------------------------------
	// 1. 회전 상태 (제자리에서 다음 목표점을 향해 회전)
	// ----------------------------------------------------
	if (turnDirection != TurnDirection::None)
	{
		// 프레임당 회전할 각도 (Degree 기준)
		float stepAngle = rotateSpeed * deltaTime;
		turnAngle += stepAngle;

		// 목표 각도를 초과하지 않도록 보정
		float actualStep = stepAngle;
		if (turnAngle >= targetAngle)
		{
			actualStep = stepAngle - (turnAngle - targetAngle);
		}

		// Degree -> Radian 변환
		float radStep = actualStep * (3.14159265f / 180.f);

		// 콘솔 좌표계(+Y가 아래) 기준: 
		// 반시계 회전(좌회전)을 하려면 각도를 빼주어야 함
		if (turnDirection == TurnDirection::CounterClockwise)
		{
			radStep = -radStep;
		}

		// 회전 행렬을 적용하여 forward 벡터 회전
		float cosVal = std::cos(radStep);
		float sinVal = std::sin(radStep);

		float newX = forward.x * cosVal - forward.y * sinVal;
		float newY = forward.x * sinVal + forward.y * cosVal;

		forward = Craft::Vector2(newX, newY).normalized();

		// 회전이 끝났는지 체크
		if (turnAngle >= targetAngle)
		{
			turnDirection = TurnDirection::None;
			turnAngle = 0.f;

			// 미세한 부동소수점 오차를 잡기 위해 다음 목표를 향하도록 forward 강제 고정
			Craft::Vector2 nextTarget = patrolPoints[currentPatrolIndex];
			forward = (nextTarget - GetPosition()).normalized();
		}

		return; // 회전 중에는 이동하지 않음
	}

	// ----------------------------------------------------
	// 2. 이동 상태 (목표 패트롤 지점을 향해 이동)
	// ----------------------------------------------------
	Craft::Vector2 targetPt = patrolPoints[currentPatrolIndex];
	Craft::Vector2 currentPos = GetPosition();
	Craft::Vector2 dir = targetPt - currentPos;
	float distance = dir.size();

	// 목표 지점에 도달했을 경우 (오차 허용 범위 0.1f)
	if (distance <= 0.1f)
	{
		SetPosition(targetPt); // 위치 정밀하게 보정

		// 이전, 현재, 다음 패트롤 지점 구하기
		int prevIdx = (currentPatrolIndex - 1 + pointCount) % pointCount;
		int currIdx = currentPatrolIndex;
		int nextIdx = (currentPatrolIndex + 1) % pointCount;

		Craft::Vector2 pPrev = patrolPoints[prevIdx];
		Craft::Vector2 pCurr = patrolPoints[currIdx];
		Craft::Vector2 pNext = patrolPoints[nextIdx];

		// 진입 벡터(u)와 진출 벡터(v) 계산
		Craft::Vector2 u = (pCurr - pPrev).normalized();
		Craft::Vector2 v = (pNext - pCurr).normalized();

		// 내적(Dot Product)을 이용해 사이 각도(Degree) 계산
		float dot = u.x * v.x + u.y * v.y;
		dot = std::fmax(-1.f, std::fmin(1.f, dot)); // acos 오류 방지 클램핑
		targetAngle = std::acos(dot) * (180.f / 3.14159265f);

		// 외적(Cross Product)을 이용해 회전 방향 결정
		float cross = u.x * v.y - u.y * v.x;

		// 콘솔 좌표계(+Y가 아래) 기준 판단
		if (cross > 0.001f) // 우회전
		{
			turnDirection = TurnDirection::Clockwise;
		}
		else if (cross < -0.001f) // 좌회전
		{
			turnDirection = TurnDirection::CounterClockwise;
		}
		else
		{
			// 일직선이거나 180도 반대 방향일 경우
			if (dot < 0.f)
			{
				targetAngle = 180.f;
				turnDirection = TurnDirection::Clockwise;
			}
			else
			{
				targetAngle = 0.f;
				turnDirection = TurnDirection::None; // 회전 불필요
			}
		}

		// 다음 패트롤 지점으로 인덱스 갱신
		currentPatrolIndex = nextIdx;

		// 회전해야 할 각도가 거의 없다면 회전 생략
		if (targetAngle < 0.1f)
		{
			turnDirection = TurnDirection::None;
			forward = v;
		}
		else
		{
			turnAngle = 0.f;
			forward = u; // 회전 시작 전 바라보는 방향을 진입 방향으로 맞춰줌
		}
	}
	else
	{
		// 아직 도달하지 않았다면 목표를 향해 이동
		Craft::Vector2 moveDir = dir.normalized();
		forward = moveDir; // 이동 방향으로 시선 고정

		Move(moveDir.x, moveDir.y, deltaTime);
	}
}

void Enemy::Trace(float deltaTime)
{
	if (!target)
		return;

	pathUpdateTimer += deltaTime;

	Vector2 playerPosition = target->GetPosition();

	if (!hasPath || pathUpdateTimer >= pathUpdateInterval)
	{
		pathUpdateTimer = 0.f;

		std::shared_ptr<GameLevel> level = Cast<GameLevel>(GetOwner());

		if (!level)
			return;

		//// Player 주변의 목표 위치 탐색
		//for (const Vector2& direction : directions)
		//{
		//	Vector2 candidate = playerPosition + direction;

		//	if (level->CanMove(candidate, shared_from_this()))
		//	{
		//		targetPosition = candidate;
		//		break;
		//	}
		//}

		CalculatePathToTarget();
	}

	MoveAlongPath(deltaTime);
}

void Enemy::Shoot()
{
	FireInterval();
}

void Enemy::Stab()
{

}

void Enemy::Search(float deltaTime)
{
	searchingTimer += deltaTime;

	float angle = std::sin(searchingTimer * 3.f) * searchMaxAngle;

	float radian = angle * 3.14159265f / 180.f;

	float cosValue = std::cos(radian);
	float sinValue = std::sin(radian);

	forward = Vector2(
		searchForward.x * cosValue - searchForward.y * sinValue,
		searchForward.x * sinValue + searchForward.y * cosValue
	).normalized();
}

void Enemy::CalculatePathToTarget()
{
	std::vector<Vector2> newPath = FindPath(GetPosition(), targetPosition);

	if (newPath.empty())
	{
		path.clear();
		currentPathIndex = 0;
		hasPath = false;
		return;
	}

	path = std::move(newPath);
	currentPathIndex = 0;
	hasPath = true;
}

void Enemy::PlayEffect(const std::vector<DestroyEffect::EffectFrame>& sequence)
{
	// 파괴 이펙트 생성
	if (GetOwner())
	{
		GetOwner()->SpawnActor<DestroyEffect>(
			GetPosition(),
			sequence
		);
	}
}

std::vector<Vector2> Enemy::FindPath(
	const Vector2& startPosition, 
	const Vector2& targetPosition)
{
	std::vector<Vector2> path;

	std::shared_ptr<GameLevel> level = Cast<GameLevel>(GetOwner());
	if (!level)
		return {};

	std::priority_queue<AStarNode> openList;

	bool closedList[MAP_HEIGHT][MAP_WIDTH] = {};
	int gCosts[MAP_HEIGHT][MAP_WIDTH];
	Craft::Vector2 parents[MAP_HEIGHT][MAP_WIDTH];

	// gCost 초기화
	for (int y = 0; y < MAP_HEIGHT; ++y)
	{
		for (int x = 0; x < MAP_WIDTH; ++x)
		{
			gCosts[y][x] = INT_MAX;
			parents[y][x] = Vector2(-1, -1);
		}
	}

	// 시작점
	int startX = static_cast<int>(startPosition.x);
	int startY = static_cast<int>(startPosition.y);

	int targetX = static_cast<int>(targetPosition.x);
	int targetY = static_cast<int>(targetPosition.y);

	// 시작점 범위 검사
	if (startX < 0 || startX >= MAP_WIDTH ||
		startY < 0 || startY >= MAP_HEIGHT)
	{
		return {};
	}

	// 목표점 범위 검사
	if (targetX < 0 || targetX >= MAP_WIDTH ||
		targetY < 0 || targetY >= MAP_HEIGHT)
	{
		return {};
	}

	gCosts[startY][startX] = 0;

	// 시작점 H Cost
	int dx = std::abs(targetX - startX);
	int dy = std::abs(targetY - startY);

	int diagonal = min(dx, dy);
	int straight = max(dx, dy) - diagonal;

	int hCost = diagonal * 14 + straight * 10;

	openList.emplace(startPosition, 0, hCost);

	while (!openList.empty())
	{
		AStarNode currentNode = openList.top();
		openList.pop();

		int currentX = static_cast<int>(currentNode.position.x);
		int currentY = static_cast<int>(currentNode.position.y);

		// 이미 처리한 위치면 무시
		if (closedList[currentY][currentX])
			continue;

		closedList[currentY][currentX] = true;

		// 목표 도착
		if (currentNode.position == targetPosition)
		{
			Vector2 currentPosition = targetPosition;

			while (currentPosition != startPosition)
			{
				path.push_back(currentPosition);

				int x = static_cast<int>(currentPosition.x);
				int y = static_cast<int>(currentPosition.y);

				currentPosition = parents[y][x];
			}

			// 시작점 -> 목표점 순서로 뒤집기
			std::reverse(path.begin(), path.end());

			return path;
		}

		// 8 방향 탐색
		for (const auto& dir : directions)
		{
			Vector2 nextPosition = currentNode.position + dir;

			int nextX = static_cast<int>(nextPosition.x);
			int nextY = static_cast<int>(nextPosition.y);

			// 맵 범위 검사
			if (nextX < 0 || nextX >= MAP_WIDTH ||
				nextY < 0 || nextY >= MAP_HEIGHT)
				continue;

			// 이미 처리한 위치
			if (closedList[nextY][nextX])
				continue;

			// 이동하려는 위치에 액터가 존재하면 이동 불가
			if (!level->CanMove(nextPosition, shared_from_this()))
				continue;

			// 직선 10, 대각선 14
			int moveCost = (dir.x == 0 || dir.y == 0) ? 10 : 14;

			// 시작점에서 nextPosition까지의 실제 비용
			int newGCost = currentNode.gCost + moveCost;

			// 기존 경로보다 좋은 경로가 아니면 무시
			if (newGCost >= gCosts[nextY][nextX])
				continue;

			// G Cost 갱신
			gCosts[nextY][nextX] = newGCost;

			// 부모 위치 저장
			parents[nextY][nextX] = currentNode.position;

			// H Cost
			int dx = std::abs(static_cast<int>(targetPosition.x - nextPosition.x));
			int dy = std::abs(static_cast<int>(targetPosition.y - nextPosition.y));

			// 8방향 이동에 맞는 Octile Distance
			int diagonal = min(dx, dy);
			int straight = max(dx, dy) - diagonal;

			int newHCost = diagonal * 14 + straight * 10;

			openList.emplace(nextPosition, newGCost, newHCost);
		}
	}

	// 경로를 찾지 못함
	return {};
}

void Enemy::MoveAlongPath(float deltaTime)
{
	if (!hasPath)
		return;

	if (currentPathIndex >= static_cast<int>(path.size()))
	{
		hasPath = false;
		return;
	}

	Vector2 currentPosition = GetPosition();
	Vector2 nextPosition = path[currentPathIndex];

	float xDir = nextPosition.x - currentPosition.x;
	float yDir = nextPosition.y - currentPosition.y;

	if (xDir != 0.f)
		xDir = xDir > 0.f ? 1.f : -1.f;

	if (yDir != 0.f)
		yDir = yDir > 0.f ? 1.f : -1.f;

	if (!Move(xDir, yDir, deltaTime))
	{
		hasPath = false;
		path.clear();
		currentPathIndex = 0;
		return;
	}

	// 목표 지점에 도착했는지 확인
	if (GetPosition() == nextPosition)
	{
		++currentPathIndex;
	}

	if (currentPathIndex >= static_cast<int>(path.size()))
	{
		hasPath = false;
	}
}
