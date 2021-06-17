#include "modifier.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>

namespace Converter3D
{
	void TransformModifier::Multiply(glm::vec3* vertices, size_t num, glm::mat4& matrix)
	{
		for (size_t i = 0; i < num; i++)
			vertices[i] = glm::vec3(matrix * glm::vec4(vertices[i], 1.f));
	}

	void TransformModifier::Multiply(glm::vec4* vertices, size_t num, glm::mat4& matrix)
	{
		for (size_t i = 0; i < num; i++)
			vertices[i] = matrix * vertices[i];
	}

	void TransformModifier::Modify(IMesh* mesh)
	{
		std::shared_ptr<IAttributeBuffer<Float>> attributes[] =
		{
			mesh->GetAttribute(Attribute::Position),
			mesh->GetAttribute(Attribute::Normal),
		};

		if (attributes[0])
			switch (attributes[0]->GetDimension())
			{
			case 3:
				Multiply(reinterpret_cast<glm::vec3*>(attributes[0]->GetPointer()), attributes[0]->GetSize(), transform);
				break;

			case 4:
				Multiply(reinterpret_cast<glm::vec4*>(attributes[0]->GetPointer()), attributes[0]->GetSize(), transform);
				break;
			}

		if (attributes[1] && attributes[1]->GetDimension() == 3)
		{
			auto normal = glm::inverseTranspose(transform);
			Multiply(reinterpret_cast<glm::vec3*>(attributes[1]->GetPointer()), attributes[1]->GetSize(), normal);
		}
	}

	void TransformModifier::Translate(const glm::vec3& v)
	{
		transform = glm::translate(v) * transform;
	}

	void TransformModifier::Rotate(float angle, const glm::vec3& v)
	{
		transform = glm::rotate(angle, v) * transform;
	}

	void TransformModifier::Scale(const glm::vec3& v)
	{
		transform = glm::scale(v) * transform;
	}
}
