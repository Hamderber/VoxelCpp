#include <VoxelCpp/input/KeyboardMovementController.hpp>
#include <VoxelCpp/game/GameObject.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/common.hpp>

namespace Input
{
	void KeyboardMovementController::move_in_plane_xz(GLFWwindow *pWindow, Game::GameObject &rGameObject, float dt)
	{
		glm::vec3 rotation{};
		rotation.y += static_cast<float>(glfwGetKey(pWindow, m_keyMappings.rotateRight) == GLFW_PRESS);
		rotation.y -= static_cast<float>(glfwGetKey(pWindow, m_keyMappings.rotateLeft) == GLFW_PRESS);
		rotation.x += static_cast<float>(glfwGetKey(pWindow, m_keyMappings.rotateUp) == GLFW_PRESS);
		rotation.x -= static_cast<float>(glfwGetKey(pWindow, m_keyMappings.rotateDown) == GLFW_PRESS);

		// Dot product of itself is zero
		if (glm::dot(rotation, rotation) > glm::epsilon<float>())
		{
			rGameObject.transform.eulerRotationRadians += glm::normalize(rotation) * m_rotateSpeed * dt;
		}

		// +/- ~85 degrees
		rGameObject.transform.eulerRotationRadians.x = glm::clamp(rGameObject.transform.eulerRotationRadians.x, -1.5f, 1.5f);

		float yaw = glm::mod(rGameObject.transform.eulerRotationRadians.y, glm::two_pi<float>());
		rGameObject.transform.eulerRotationRadians.y = yaw;

		const glm::vec3 WORLD_UP{ 0.f, 1.f, 0.f };
		const glm::vec3 FORWARD_DIR{ glm::sin(yaw), 0.f, glm::cos(yaw) };
		const glm::vec3 RIGHT_DIR = glm::normalize(glm::cross(WORLD_UP, FORWARD_DIR));

		glm::vec3 movement{};
		if (glfwGetKey(pWindow, m_keyMappings.moveUp) == GLFW_PRESS) movement += WORLD_UP;
		if (glfwGetKey(pWindow, m_keyMappings.moveDown) == GLFW_PRESS) movement -= WORLD_UP;
		if (glfwGetKey(pWindow, m_keyMappings.moveLeft) == GLFW_PRESS) movement -= RIGHT_DIR;
		if (glfwGetKey(pWindow, m_keyMappings.moveRight) == GLFW_PRESS) movement += RIGHT_DIR;
		if (glfwGetKey(pWindow, m_keyMappings.moveForward) == GLFW_PRESS) movement += FORWARD_DIR;
		if (glfwGetKey(pWindow, m_keyMappings.moveBackward) == GLFW_PRESS) movement -= FORWARD_DIR;
		
		if (glm::dot(movement, movement) > glm::epsilon<float>())
		{
			rGameObject.transform.translation += glm::normalize(movement) * m_moveSpeed * dt;
		}
	}
}
