#pragma once

#include <VoxelCpp/rendering/Model.hpp>
#include <VoxelCpp/physics/RigidBody.hpp>
#include <glm/fwd.hpp>
#include <memory>
#include <unordered_map>

// This implementation is TEMPORARY !

namespace Game
{
	// TODO: Change this to use a single draw command and have the indexed cumulative light go be iterated in the shader
	// rather than multiple draw calls on a per-light basis.
	struct PointLightComponent
	{
		float lightIntensity = 1.f;
	};

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

		// TODO: Don't do this. Make an actual point light object because doing it this way is stupid
		std::unique_ptr<PointLightComponent> m_pPointLight = nullptr;
		static GameObject make_point_light(float radius, glm::vec3 color, float intensity);
	
	private:
		GameObject(id_t id) : m_ID{ id } {};

		const id_t m_ID;
	};
}