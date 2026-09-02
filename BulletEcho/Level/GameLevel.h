#pragma once

#include <Level/Level.h>
#include <Actor/Actor.h>
#include <memory>

class Player;

class GameLevel : public Craft::Level
{
	TYPE_DECLARATIONS(GameLevel, Level)

private:
	// 초기화 이벤트 함수 오버라이드
	virtual void OnInitialized() override;

public:
	virtual void ProcessPlayerSight() override;
	virtual void Draw3D() override;

	bool CanMove(
		const Craft::Vector2& nextPosition,
		const std::shared_ptr<Craft::Actor>& movingActor
	);

	inline bool CheckGameClear() const { return leftEnemy <= 0; }
	inline void EnemyKilled() { leftEnemy--; }
	inline void SetElapsedTime(float deltaTime) { elapsedTime += deltaTime; }
	inline float GetElapsedTime() const { return elapsedTime; }
	
	Player* GetPlayer() const;

private:
	void LoadMap(const std::string& filename);

	std::vector<std::string> map;

	bool isGameClear = false;

	float elapsedTime = 0.f;

	int leftEnemy = 0;

	std::weak_ptr<Player> player;
};

