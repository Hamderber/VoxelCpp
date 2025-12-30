#include <VoxelCpp/rendering/Camera.hpp>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <cmath>
#include <limits>
#include <cassert>

namespace Rendering
{
	void Camera::projection_orthographic_set(float left, float right, float top, float bottom, float near, float far)
	{
		m_projectionMatrix = glm::mat4{ 1.f };
		m_projectionMatrix[0][0] = 2.f / (right - left);
		m_projectionMatrix[1][1] = 2.f / (bottom - top);
		m_projectionMatrix[2][2] = 1.f / (far - near);
		m_projectionMatrix[3][0] = -(right + left) / (right - left);
		m_projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
		m_projectionMatrix[3][2] = -near / (far - near);
	}

	void Camera::projection_perspective_set(float fovY, float aspect, float near, float far)
	{
		assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f && "Perspective cannot be zero.");
		const float tanHalfFovy = tan(fovY / 2.f);
		m_projectionMatrix = glm::mat4{ 0.f };
		m_projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
		m_projectionMatrix[1][1] = -1.f / (tanHalfFovy);
		m_projectionMatrix[2][2] = far / (far - near);
		m_projectionMatrix[2][3] = 1.f;
		m_projectionMatrix[3][2] = -(far * near) / (far - near);
	}

	void Camera::set_view_direction(const glm::vec3 CAMERA_POS, const glm::vec3 FACING_DIR, const glm::vec3 UP)
	{
		const glm::vec3 w{ glm::normalize(FACING_DIR) };
		const glm::vec3 u{ glm::normalize(glm::cross(UP, w)) };
		const glm::vec3 v{ glm::cross(w, u) };

		m_viewMatrix = glm::mat4{ 1.f };
		m_viewMatrix[0][0] = u.x; m_viewMatrix[1][0] = u.y; m_viewMatrix[2][0] = u.z;
		                          m_viewMatrix[1][1] = v.y; m_viewMatrix[2][1] = v.z;
		m_viewMatrix[0][2] = w.x; m_viewMatrix[1][2] = w.y; m_viewMatrix[2][2] = w.z;
		
		m_viewMatrix[3][0] = -glm::dot(u, CAMERA_POS);
		m_viewMatrix[3][1] = -glm::dot(v, CAMERA_POS);
		m_viewMatrix[3][2] = -glm::dot(w, CAMERA_POS);

		m_inverseViewMatrix = glm::inverse(m_viewMatrix);
	}

	void Camera::set_view_target(const glm::vec3 CAMERA_POS, const glm::vec3 TARGET, const glm::vec3 UP)
	{
		set_view_direction(CAMERA_POS, TARGET - CAMERA_POS, UP);
	}

	void Camera::set_view_yxz(const glm::vec3 CAMERA_POS, const glm::vec3 EULER_RADIANS)
	{
		const float yaw = EULER_RADIANS.y;
		const float pitch = EULER_RADIANS.x;

		// Forward (+Z when yaw=0, pitch=0)
		glm::vec3 forward{
			glm::sin(yaw) * glm::cos(pitch),
			glm::sin(pitch),
			glm::cos(yaw) * glm::cos(pitch)
		};
		forward = glm::normalize(forward);

		const glm::vec3 worldUp{ 0.f, 1.f, 0.f };

		// Right = Up x Forward
		glm::vec3 right = glm::normalize(glm::cross(worldUp, forward));
		glm::vec3 up = glm::cross(forward, right);

		// Column-major (GLM)
		m_viewMatrix = glm::mat4{ 1.f };
		m_viewMatrix[0][0] = right.x;   m_viewMatrix[1][0] = right.y;   m_viewMatrix[2][0] = right.z;
		m_viewMatrix[0][1] = up.x;      m_viewMatrix[1][1] = up.y;      m_viewMatrix[2][1] = up.z;
		m_viewMatrix[0][2] = forward.x; m_viewMatrix[1][2] = forward.y; m_viewMatrix[2][2] = forward.z;

		m_viewMatrix[3][0] = -glm::dot(right, CAMERA_POS);
		m_viewMatrix[3][1] = -glm::dot(up, CAMERA_POS);
		m_viewMatrix[3][2] = -glm::dot(forward, CAMERA_POS);

		m_inverseViewMatrix = glm::inverse(m_viewMatrix);
	}
}