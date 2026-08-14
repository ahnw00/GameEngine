#pragma once

//#include <Actor/Actor.h>
#include <Actor/Character.h>
#include <Util/Sight.h>

#include <memory>

//" * "
//"***"
//" * "

class Camera;

class Player : public Character
{
	// 발사 모드(단발/연사)
	enum class FireMode
	{
		None = -1,
		OneShot,
		Repeat
	};

	// RTTI 등록
	TYPE_DECLARATIONS(Player, Character)

public:
	Player(Craft::Vector2 position);
	~Player();

private:
	// 이벤트 함수 오버라이드
	virtual void BeginPlay() override;

	// 이벤트 함수 오버라이드
	virtual void Tick(float deltaTime) override;

	// 충돌 이벤트 함수 오버라이드
	virtual void OnCollision(const std::shared_ptr<Actor>& other) override;

private:
	// 발사 모드 변수
	FireMode fireMode = FireMode::None;

	std::unique_ptr<Camera> camera;

	float footstepTimer = 0.35f;
	float footstepInterval = 0.4f;

	// 8방향 스프라이트
	// 순서: N, NE, E, SE, S, SW, W, NW
	std::vector<std::string> sprites[8] = {
		{" | "
		," @ "
		,"   "},

		{"  /"
		," @ "
		,"   "},

		{"   "
		," @-"
		,"   "},

		{"   "
		," @ "
		,"  \\"},

		{"   "
		," @ "
		," | "},

		{"   "
		," @ "
		,"/  "},

		{"   "
		,"-@ "
		,"   "},

		{"\\  "
		," @ "
		,"   "}
	};
};
