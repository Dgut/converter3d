#pragma once

#include <memory>
#include <string>

namespace Converter3D
{
	using Float = float;
	using Uint = unsigned;
	using Error = int;

	namespace Errors
	{
		enum
		{
			Success = 0,
			CannotOpenFile = -1,
			WrongFileFormat = -2,
			WrongMeshFormat = -3,
			UnknownExtension = -4,
		};
	}

	// Possible vertex attributes. Can be easily extended.
	enum class Attribute : unsigned
	{
		Position,
		Texture,
		Normal,

		Count,
	};

	// Buffer for storing something.
	template<class T>
	class IBuffer
	{
	public:
		virtual const T& Get(size_t index) const = 0;
		virtual size_t GetSize() const = 0;
	};

	// Buffer for attributes with sequential access to elements in memory.
	template<class T>
	class IAttributeBuffer : public IBuffer<T>
	{
	public:
		virtual T* GetPointer() = 0;
		virtual const T* GetPointer() const = 0;
		virtual size_t GetDimension() const = 0;
	};

	// Contains the attributes of the vertices of the faces. The number of attributes must be the same if the attribute exists.
	template<class T>
	class IFace
	{
	public:
		virtual const IBuffer<T>& Get(Attribute attribute) const = 0;
		virtual size_t GetSize() const = 0;
	};

	// Mesh consists of a set of attributes and faces containing the indexes of these attributes.
	class IMesh
	{
	public:
		virtual void SetAttribute(Attribute attribute, std::shared_ptr<IAttributeBuffer<Float>> buffer) = 0;
		virtual const std::shared_ptr<IAttributeBuffer<Float>> GetAttribute(Attribute attribute) const = 0;

		virtual void SetFaces(std::shared_ptr<IBuffer<IFace<Uint>>> faces) = 0;
		virtual const std::shared_ptr<IBuffer<IFace<Uint>>> GetFaces() const = 0;
	};

	class IImporter
	{
	public:
		virtual Error Import(IMesh* mesh, std::string path) = 0;
	};

	class IExporter
	{
	public:
		virtual Error Export(const IMesh* mesh, std::string path) = 0;
	};

	class IModifier
	{
	public:
		virtual void Modify(IMesh* mesh) = 0;
	};

	class IManager
	{
	public:
		virtual void RegisterImporter(std::string extension, std::shared_ptr<IImporter> importer) = 0;
		virtual void RegisterExporter(std::string extension, std::shared_ptr<IExporter> exporter) = 0;

		virtual void AddModifier(std::shared_ptr<IModifier> modifier) = 0;

		virtual Error Import(std::string path) = 0;
		virtual Error Export(std::string path) = 0;
	};
}
