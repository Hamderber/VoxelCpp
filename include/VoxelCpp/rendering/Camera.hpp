#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/fwd.hpp>

namespace Rendering
{
	class Camera
	{
	public:
		void projection_orthographic_set(float left, float right, float top, float bottom, float near, float far);
		void projection_perspective_set(float fovY, float aspect, float near, float far);

		void set_view_direction(const glm::vec3 CAMERA_POS, const glm::vec3 FACING_DIR, const glm::vec3 UP = glm::vec3{ 0, -1.0f, 0 });
		void set_view_target(const glm::vec3 CAMERA_POS, const glm::vec3 TARGET, const glm::vec3 UP = glm::vec3{ 0, -1.0f, 0 });
		void set_view_yxz(const glm::vec3 CAMERA_POS, const glm::vec3 EULER_RADIANS);

		const glm::mat4 &projection_get() const { return m_projectionMatrix; };
		const glm::mat4 &view_get() const { return m_viewMatrix; };

	private:
		glm::mat4 m_projectionMatrix{ 1 };
		glm::mat4 m_viewMatrix{ 1 };
	};
}