#pragma once

#include <Actor/Actor.h>
#include <memory>

class Sight;

class Character : public Craft::Actor
{
public:
	enum class Type
	{
		None,
		Player,
		Enemy
	};

	TYPE_DECLARATIONS(Character, Actor)

public:
	Character(
		const std::vector<std::string>& sprite,
		Craft::Vector2 position,
		Craft::Color color
	);
	virtual ~Character();

protected:
	// 이벤트 함수 오버라이드
	virtual void Tick(float deltaTime) override;

	// 캐릭터 타입(플레이어/적)
	Type type = Type::None;

	// 시야
	std::unique_ptr<Sight> sight;

public:
	inline Type GetCharacterType()	const { return type; }

	Sight* GetSight();

protected:
	inline void SetCharacterType(Type ct) { type = ct; }
};

