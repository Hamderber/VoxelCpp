#include <VoxelCpp/game/GameObject.hpp>
#include <memory>
#include <glm/fwd.hpp>

namespace Game
{
	GameObject GameObject::make_point_light(float radius, glm::vec3 color, float intensity)
	{
		GameObject go = create();
		go.color = color;
		go.transform.uniformScale = radius;
		go.m_pPointLight = std::make_unique<PointLightComponent>();
		go.m_pPointLight->lightIntensity = intensity;

		return go;
	}
}