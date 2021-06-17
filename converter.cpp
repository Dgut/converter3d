#include "converter.h"
#include "general.h"
#include "modifier.h"
#include <array>
#include <glm/gtc/constants.hpp>

namespace Converter3D
{
	// tokenizer
	char* gettoken(char** str, const char* delim)
	{
		char* token = *str + strspn(*str, delim);
		*str = *str + strcspn(*str, delim);
		if (**str)
			*(*str)++ = 0;
		return token;
	}

	// file helper - automatically closes the file
	class File
	{
		FILE* stream;
	public:
		File(const char* name, const char* mode) : stream(nullptr) { fopen_s(&stream, name, mode); }
		~File() { if (stream)fclose(stream); }

		operator FILE*() const { return stream; }
	};

	void ObjImporter::ReadFloats(char** data, std::vector<Float>& floats, size_t number)
	{
		while (number--)
		{
			char* token = gettoken(data, " \t");
			floats.push_back(token ? static_cast<float>(atof(token)) : 0.f);
		}
	}

	unsigned ObjImporter::ReadIndices(char** data, int* indices, size_t number)
	{
		unsigned result = 0;

		char* token = gettoken(data, " \t");

		for (size_t i = 0; *token && i < number; i++)
		{
			char* index = gettoken(&token, "/");
			if (*index)
			{
				result |= 1 << i;
				indices[i] = atoi(index);
			}
		}

		return result;
	}

	Error ObjImporter::Import(IMesh* mesh, std::string path)
	{
		File file(path.c_str(), "rt");

		if (!file)
			return Errors::CannotOpenFile;	// not found

		std::shared_ptr<AttributeBuffer<Float>> attributes[] =
		{
			std::make_shared<AttributeBuffer<Float>>(3),
			std::make_shared<AttributeBuffer<Float>>(3),
			std::make_shared<AttributeBuffer<Float>>(3),
		};

		auto faces = std::make_shared<FaceBuffer<Uint>>();

		std::vector<char> line(16 * 1024);	// we assume that no one has written very long comments/faces
		while (fgets(line.data(), static_cast<int>(line.size()), file))
		{
			char* data = line.data();	// raw pointers for fast parsing
			char* comment = strchr(data, '#');
			if (comment)
				*comment = 0;	// deleting comments

			char* element = gettoken(&data, " \t");

			if (!strcmp(element, "v"))
				ReadFloats(&data, *attributes[static_cast<size_t>(Attribute::Position)], 3);
			else if(!strcmp(element, "vn"))
				ReadFloats(&data, *attributes[static_cast<size_t>(Attribute::Normal)], 3);
			else if (!strcmp(element, "vt"))
				ReadFloats(&data, *attributes[static_cast<size_t>(Attribute::Texture)], 3);
			else if (!strcmp(element, "f"))
			{
				unsigned face_format = 0;

				std::string index;
				Face<Uint>& face = faces->emplace_back(Face<Uint>());

				std::array<int, 3> offset = { 0, 0, 0 };

				unsigned format;

				while (format = ReadIndices(&data, offset.data(), offset.size()))
				{
					if (!face_format)
						face_format = format;
					else if(face_format != format)
						return Errors::WrongFileFormat;	// not consistent face format

					for (size_t i = 0; i < offset.size(); i++)
						if (face_format & (1 << i))
							face[i].push_back(offset[i] > 0 ? static_cast<Uint>(offset[i] - 1) : static_cast<Uint>(attributes[i]->GetSize() + offset[i]));
				}

				if (face.GetSize() < 3)
					return Errors::WrongFileFormat;	// not enough vertices
			}
		}

		for (int i = 0; i < 3; i++)
		{
			if (attributes[i]->GetSize())
				mesh->SetAttribute(static_cast<Attribute>(i), attributes[i]);
		}
		mesh->SetFaces(faces);

		auto rotation = TransformModifier();	// 3d modelling tools usually rotate obj files so we will do the same
		rotation.Rotate(glm::half_pi<float>(), glm::vec3(1.f, 0.f, 0.f));
		rotation.Modify(mesh);

		return Errors::Success;
	}

	Error StlExporter::Export(const IMesh* mesh, std::string path)
	{
		auto attribute = mesh->GetAttribute(Attribute::Position);
		auto faces = mesh->GetFaces();

		if (attribute && attribute->GetDimension() != 3)
			return Errors::WrongMeshFormat;	// unsuitable mesh format

		File file(path.c_str(), "wb");	// C i/o is faster
		if (!file)
			return Errors::CannotOpenFile;	// cannot be opened for writing

		uint32_t num_triangles = 0;
		uint16_t zero_attribute = 0;

		const size_t HeaderSize = 80;

		fseek(file, HeaderSize + sizeof(num_triangles), SEEK_SET);	// skip header and triangles number

		Mesh::ForEachTriangle(*mesh, [&](auto a, auto b, auto c)
			{
				auto normal = glm::normalize(glm::cross(c - b, a - b));

				fwrite(&normal, sizeof(float), 3, file);
				fwrite(&a, sizeof(float), 3, file);
				fwrite(&b, sizeof(float), 3, file);
				fwrite(&c, sizeof(float), 3, file);
				fwrite(&zero_attribute, sizeof(zero_attribute), 1, file);

				num_triangles++;
			});

		fseek(file, HeaderSize, SEEK_SET);	// write triangles number
		fwrite(&num_triangles, sizeof(num_triangles), 1, file);

		fclose(file);

		return Errors::Success;
	}
}
