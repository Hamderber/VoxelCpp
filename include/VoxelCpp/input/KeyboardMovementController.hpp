#pragma once

#include <VoxelCpp/rendering/Window.hpp>

namespace Game { class GameObject; }

namespace Input
{
	// TEMP
	class KeyboardMovementController
	{
	public:
        struct KeyMappings
        {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int rotateLeft = GLFW_KEY_LEFT;
            int rotateRight = GLFW_KEY_RIGHT;
            int rotateUp = GLFW_KEY_UP;
            int rotateDown = GLFW_KEY_DOWN;
        };

        void move_in_plane_xz(GLFWwindow *pWindow, Game::GameObject &rGameObject, float dt);

    private:
        KeyMappings m_keyMappings{};
        float m_moveSpeed = 3.f;
        float m_rotateSpeed = 1.5f;
	};
}