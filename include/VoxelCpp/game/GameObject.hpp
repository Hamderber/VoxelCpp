#pragma once

#include <VoxelCpp/rendering/Model.hpp>
#include <VoxelCpp/physics/RigidBody.hpp>
#include <glm/fwd.hpp>
#include <memory>
#include <unordered_map>

// This implementation is TEMPORARY !

namespace Game
{
	class GameObject
	{
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, GameObject>;

		GameObject(const GameObject &) = delete;
		GameObject &operator=(const GameObject &) = delete;
		GameObject(GameObject &&) = default;
		GameObject &operator=(GameObject &&) = default;

		static GameObject create()
		{
			static id_t currentId = 0;
			return GameObject(currentId++);
		}

		const id_t id() const { return m_ID; };

		std::shared_ptr<Rendering::Model> pModel{};
		glm::vec3 color{};
		Physics::Transform transform{};

	private:
		GameObject(id_t id) : m_ID{ id } {};

		const id_t m_ID;
	};
}