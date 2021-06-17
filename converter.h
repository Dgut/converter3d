#pragma once

#include "interface.h"
#include <vector>

namespace Converter3D
{
	class ObjImporter : public IImporter
	{
		static void ReadFloats(char** data, std::vector<Float>& floats, size_t number);
		static unsigned ReadIndices(char** data, int* indices, size_t number);
	public:
		virtual Error Import(IMesh* mesh, std::string path) override;
	};

	class StlExporter : public IExporter
	{
	public:
		virtual Error Export(const IMesh* mesh, std::string path) override;
	};
}
