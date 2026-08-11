#include "Actor.h"
#include <Engine/Engine.h>
#include <Render/Renderer.h>

namespace Craft
{
	Actor::Actor(
		const std::vector<std::string>& image,
		const Vector2& position,
		Color color) 
		: image(image), position(position), color(color),
		width(static_cast<int>(image[0].length())),
		height(static_cast<int>(image.size()))
	{ }

	Actor::~Actor()
	{
	}

	void Actor::BeginPlay()
	{
		// 이벤트 처리했다고 설정
		hasBegunPlay = true;
	}

	void Actor::Tick(float deltaTime)
	{
	}

	void Actor::Draw()
	{
		// 비활성 상태이면 종료
		if (!IsActive())
			return;

		// 렌더러에 필요한 데이터 제출
		Renderer::Get().Submit(image, position, color, sortingOrder);
	}

	void Actor::OnCollision(const std::shared_ptr<Actor>& other)
	{
	}

	void Actor::Destroy()
	{
		// 삭제 예약
		hasExpired = true;
	}

	void Actor::QuitGame()
	{
		// 엔진 종료 요청
		Engine::Get().Quit();
	}

	void Actor::SetPosition(const Vector2& newPosition)
	{
		// 기존 값과 동일하면 종료
		if (position == newPosition)
			return;

		position = newPosition;
	}
}