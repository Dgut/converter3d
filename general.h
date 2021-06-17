#pragma once

#include "interface.h"
#include <functional>
#include <list>
#include <map>
#include <vector>
#include <glm/glm.hpp>

namespace Converter3D
{
	// helper function for casting from attribute buffer to glm::vec3
	inline const glm::vec3& attrib3(const IAttributeBuffer<Float>& attrib, size_t index)
	{
		return *reinterpret_cast<const glm::vec3*>(attrib.GetPointer() + index * 3);
	}

	template<class T>
	class AttributeBuffer : public std::vector<T>, public IAttributeBuffer<T>
	{
		size_t dimension = 0;
	public:
		AttributeBuffer(size_t dimension = 1) :
			dimension(dimension)
		{
		}

		virtual T* GetPointer() override
		{
			return std::vector<T>::data();
		}

		virtual const T* GetPointer() const override
		{
			return std::vector<T>::data();
		}

		virtual const T& Get(size_t index) const override
		{
			return std::vector<T>::operator[](index);
		}

		virtual size_t GetSize() const override
		{
			return dimension ? std::vector<T>::size() / dimension : 0;
		}

		virtual size_t GetDimension() const override
		{
			return dimension;
		}
	};

	template<class T>
	class Face : public IFace<T>
	{
		AttributeBuffer<T> indices[static_cast<size_t>(Attribute::Count)];
	public:
		Face()
		{
			for (auto& i : indices)
				i.reserve(4);
		}

		virtual const IBuffer<T>& Get(Attribute attribute) const override
		{
			return indices[static_cast<size_t>(attribute)];
		}

		virtual size_t GetSize() const override
		{
			return indices[static_cast<size_t>(Attribute::Position)].size();
		}

		std::vector<T>& operator[](size_t index)
		{
			return indices[index];
		}

		const std::vector<T>& operator[](size_t index) const
		{
			return indices[index];
		}
	};

	template<class T>
	class FaceBuffer : public std::vector<Face<T>>, public IBuffer<IFace<T>>
	{
	public:
		virtual const IFace<T>& Get(size_t index) const override
		{
			return std::vector<Face<T>>::operator[](index);
		}

		virtual size_t GetSize() const override
		{
			return std::vector<Face<T>>::size();
		}
	};

	class Mesh : public IMesh
	{
		std::shared_ptr<IAttributeBuffer<Float>> attributes[static_cast<size_t>(Attribute::Count)];
		std::shared_ptr<IBuffer<IFace<Uint>>> faces;
	public:
		virtual void SetAttribute(Attribute attribute, std::shared_ptr<IAttributeBuffer<Float>> buffer) override
		{
			attributes[static_cast<size_t>(attribute)] = buffer;
		}

		virtual const std::shared_ptr<IAttributeBuffer<Float>> GetAttribute(Attribute attribute) const override
		{
			return attributes[static_cast<size_t>(attribute)];
		}

		virtual void SetFaces(std::shared_ptr<IBuffer<IFace<Uint>>> faces) override
		{
			Mesh::faces = faces;
		}

		virtual const std::shared_ptr<IBuffer<IFace<Uint>>> GetFaces() const override
		{
			return faces;
		}

		Float Area() const;
		Float Volume() const;
		bool IsInside(const glm::vec3& p) const;

		static void ForEachTriangle(const IMesh& mesh, std::function<void(const glm::vec3&, const glm::vec3&, const glm::vec3&)> func);
	};

	class Manager : public IManager
	{
		std::map<std::string, std::shared_ptr<IImporter>> importers;
		std::map<std::string, std::shared_ptr<IExporter>> exporters;
		std::list<std::shared_ptr<IModifier>> modifiers;

		std::unique_ptr<Mesh> mesh;

		static std::string Extension(std::string path);
	public:
		virtual void RegisterImporter(std::string extension, std::shared_ptr<IImporter> importer) override
		{
			importers[extension] = importer;
		}

		virtual void RegisterExporter(std::string extension, std::shared_ptr<IExporter> exporter) override
		{
			exporters[extension] = exporter;
		}

		virtual void AddModifier(std::shared_ptr<IModifier> modifier) override
		{
			modifiers.push_back(modifier);
		}

		virtual Error Import(std::string path) override;
		virtual Error Export(std::string path) override;

		Mesh* GetMesh() const;
	};
}
