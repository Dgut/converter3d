#pragma once

#include "interface.h"
#include <glm/glm.hpp>

namespace Converter3D
{
	class TransformModifier : public IModifier
	{
		glm::mat4 transform;

		static void Multiply(glm::vec3* vertices, size_t num, glm::mat4& matrix);
		static void Multiply(glm::vec4* vertices, size_t num, glm::mat4& matrix);
	public:
		TransformModifier() :
			transform(1.f)
		{
		}

		virtual void Modify(IMesh* mesh) override;

		void Translate(const glm::vec3& v);
		void Rotate(float angle, const glm::vec3& v);
		void Scale(const glm::vec3& v);
	};
}
