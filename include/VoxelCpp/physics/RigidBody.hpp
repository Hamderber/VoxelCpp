#pragma once

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace Physics
{
	/*
		Don't forget that vulkan's positive y axis points down
	*/

	// TODO: Bake these values on member changes to avoid recalculating them per frame
	struct Transform
	{
		glm::vec3 translation{};
        float uniformScale{ 1.f };
		glm::vec3 eulerRotationRadians{};

		/// <summary>
		/// Order: Translate * Ry * Rx * Rz * scale. Tait-Bryan Euler Angles order Y(1) X(2) Z(3). Optimized to directly apply to
        /// the applicable mat4 members. To interperet as extrinsic, read YXZ from right to left and intrinsic as left to right.
		/// </summary>
		/// <returns>glm::mat4 transformation matrix</returns>
        glm::mat4 matrix() const
        {
            const float c3 = glm::cos(eulerRotationRadians.z);
            const float s3 = glm::sin(eulerRotationRadians.z);
            const float c2 = glm::cos(eulerRotationRadians.x);
            const float s2 = glm::sin(eulerRotationRadians.x);
            const float c1 = glm::cos(eulerRotationRadians.y);
            const float s1 = glm::sin(eulerRotationRadians.y);
            glm::mat4 result{
                {
                    uniformScale * (c1 * c3 + s1 * s2 * s3),
                    uniformScale * (c2 * s3),
                    uniformScale * (c1 * s2 * s3 - c3 * s1),
                    0.0f,
                },
                {
                    uniformScale * (c3 * s1 * s2 - c1 * s3),
                    uniformScale * (c2 * c3),
                    uniformScale * (c1 * c3 * s2 + s1 * s3),
                    0.0f,
                },
                {
                    uniformScale * (c2 * s1),
                    uniformScale * (-s2),
                    uniformScale * (c1 * c2),
                    0.0f,
                },
                {translation.x, translation.y, translation.z, 1.0f}};

            return result;
        }
	};

	class RigidBody
	{

	};
}