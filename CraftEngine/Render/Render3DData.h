#pragma once

#include <Core/Core.h> // 외부에서 접근하게 하려고
#include <Math/Color.h>

namespace Craft
{
	struct CRAFT_API Render3DData
	{
		enum class Shape
		{
			None,
			Rectangle,
			Circle,
			Billboard
		};

		Shape shape = Shape::None;

		float width = 1.f;
		float height = 1.f;

		Color color = Color::White;
	};
}