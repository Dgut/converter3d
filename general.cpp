#include "general.h"

namespace Converter3D
{
	Float Mesh::Area() const
	{
		Float area = 0.f;
		ForEachTriangle(*this, [&area](auto a, auto b, auto c) { area += glm::length(glm::cross(c - b, a - b)); });
		return area / 2.f;
	}

	Float Mesh::Volume() const
	{
		Float volume = 0.f;
		ForEachTriangle(*this, [&volume](auto a, auto b, auto c) { volume += glm::dot(a, (glm::cross(b, c))); });
		return volume / 6.f;
	}

	bool Mesh::IsInside(const glm::vec3& p) const
	{
		Float nearest = std::numeric_limits<Float>::max();
		bool inside = false;

		const glm::vec3 dir{ 0, 0, 1 };
		const float epsilon = 1e-8f;

		// Möller-Trumbore algorithm
		ForEachTriangle(*this, [&](auto a, auto b, auto c)
			{
				glm::vec3 ab = b - a;
				glm::vec3 ac = c - a;
				glm::vec3 pvec = glm::cross(dir, ac);
				float det = glm::dot(ab, pvec);

				if (fabs(det) < epsilon)
					return;

				float inv_det = 1 / det;

				glm::vec3 tvec = p - a;
				float u = glm::dot(tvec, pvec) * inv_det;
				if (u < 0 || u > 1)
					return;

				glm::vec3 qvec = glm::cross(tvec, ab);
				float v = glm::dot(dir, qvec) * inv_det;
				if (v < 0 || u + v > 1)
					return;

				float t = glm::dot(ac, qvec) * inv_det;

				if (t >= 0 && t < nearest)
				{
					nearest = t;
					inside = det < epsilon;
				}
			});

		return inside;
	}

	void Mesh::ForEachTriangle(const IMesh& mesh, std::function<void(const glm::vec3&, const glm::vec3&, const glm::vec3&)> func)
	{
		auto attribute = mesh.GetAttribute(Attribute::Position);
		auto faces = mesh.GetFaces();

		if (!attribute || attribute->GetDimension() != 3)
			return;

		const auto faces_num = faces->GetSize();
		for (size_t f = 0; f < faces_num; f++)
		{
			auto& face = faces->Get(f);

			auto& a = attrib3(*attribute, face.Get(Attribute::Position).Get(0));

			const auto face_size = face.GetSize();
			for (size_t i = 2; i < face_size; i++)
			{
				auto& b = attrib3(*attribute, face.Get(Attribute::Position).Get(i - 1));
				auto& c = attrib3(*attribute, face.Get(Attribute::Position).Get(i));

				func(a, b, c);
			}
		}
	}

	std::string Manager::Extension(std::string path)
	{
		const size_t dot = path.rfind('.');
		if (dot == std::string::npos)
			return "";
		return path.substr(dot + 1);
	}

	Error Manager::Import(std::string path)
	{
		std::string extension = Extension(path);

		auto importer = importers.find(extension);
		if (importer == importers.end())
			return Errors::UnknownExtension;

		mesh = std::make_unique<Mesh>();

		auto error = importer->second->Import(mesh.get(), path);
		if (error != Errors::Success)
			return error;

		for (auto& modifier : modifiers)
			modifier->Modify(mesh.get());

		return Errors::Success;
	}

	Error Manager::Export(std::string path)
	{
		std::string extension = Extension(path);

		auto exporter = exporters.find(extension);
		if (exporter == exporters.end())
			return Errors::UnknownExtension;

		if (!mesh)
			return Errors::WrongMeshFormat;

		return exporter->second->Export(mesh.get(), path);
	}

	Mesh* Manager::GetMesh() const
	{
		return mesh.get();
	}
}
