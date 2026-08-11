#include "Vector2.h"
#include <cassert>
#include <cmath>

namespace Craft
{
	Vector2 Vector2::Zero(0, 0);
	Vector2 Vector2::One(1, 1);
	Vector2 Vector2::Right(1, 0);
	Vector2 Vector2::Up(0, 1);

	Vector2::Vector2(float x, float y) : x(x), y(y)
	{
	}

	Vector2::operator COORD() const
	{
		COORD coord = {};
		coord.X = static_cast<short>(x);
		coord.Y = static_cast<short>(y);

		return coord;
	}

	Vector2::operator COORD()
	{
		COORD coord = {};
		coord.X = static_cast<short>(x);
		coord.Y = static_cast<short>(y);

		return coord;
	}

	Vector2 Vector2::operator+(const Vector2& other) const
	{
		return Vector2(x + other.x, y + other.y);
	}

	Vector2 Vector2::operator-(const Vector2& other) const
	{
		return Vector2(x - other.x, y - other.y);
	}

	Vector2 Vector2::operator*(const Vector2& other) const
	{
		return Vector2(x * other.x, y * other.y);
	}

	Vector2 Vector2::operator*(const float multiply) const
	{
		return Vector2(x * multiply, y * multiply);
	}

	Vector2 Vector2::operator/(const Vector2& other) const
	{
		assert(other.x != 0 && other.y != 0);
		return Vector2(x / other.x, y / other.y);
	}

	Vector2& Vector2::operator=(const Vector2& other)
	{
		x = other.x;
		y = other.y;

		return *this;
	}

	Vector2& Vector2::operator+=(const Vector2& other)
	{
		x += other.x;
		y += other.y;

		return *this;
	}

	Vector2& Vector2::operator-=(const Vector2& other)
	{
		x -= other.x;
		y -= other.y;

		return *this;
	}

	bool Vector2::operator==(const Vector2& other) const
	{
		return (x == other.x) && (y == other.y);
	}

	bool Vector2::operator!=(const Vector2& other) const
	{
		//return (x != other.x) || (y != other.y);
		return !(*this == other);
	}

	Vector2 Vector2::normalized() const
	{
		// TODO: insert return statement here
		float size = std::sqrt(x * x + y * y);
		assert(size != 0);

		return Vector2(x / size, y / size);
	}
}