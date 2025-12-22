#pragma once

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <cmath>

namespace Physics
{
	/*
		Don't forget that vulkan's positive y axis points down
	*/

	struct Transform2D
	{
		glm::vec2 translation{};
		glm::vec2 scale{1,1};
		float rotationRadians = 0;

		// TODO: Bake these values on member changes to avoid recalculating them per frame
		glm::mat2 matrix() const
		{
			const float SIN = glm::sin(rotationRadians);
			const float COS = glm::cos(rotationRadians);

			glm::mat2 rotatedMat{{ COS, SIN }, { -SIN, COS }};
			glm::mat2 scaledMat{{ scale.x, 0 }, { 0, scale.y }};
			
			return rotatedMat * scaledMat;
		};
	};

	class RigidBody
	{

	};
}