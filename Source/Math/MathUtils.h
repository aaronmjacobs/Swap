#pragma once

#include <glm/glm.hpp>

#include <cmath>

namespace MathUtils
{
	constexpr float kSmallNumber = 1.e-8f;

	inline float safeReciprocal(float value, float tolerance = kSmallNumber)
	{
		if (std::fabsf(value) <= tolerance)
		{
			return 0.0f;
		}

		return 1.0f / value;
	}

	inline glm::vec3 safeReciprocal(const glm::vec3& value, float tolerance = kSmallNumber)
	{
		return glm::vec3(safeReciprocal(value.x, tolerance), safeReciprocal(value.y, tolerance),
			safeReciprocal(value.z, tolerance));
	}
}
