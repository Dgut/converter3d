#include "general.h"
#include "converter.h"
#include "modifier.h"
#include <chrono>
#include <iostream>
#include <optional>

using namespace Converter3D;

static int g_argc;
static char** g_argv;
static int g_arg;

bool Command(const char* command, int arguments, const char* usage, std::function<void(char** argv)> parse)
{
    if (strcmp(command, g_argv[g_arg]))
        return false;

    if (g_arg++ + arguments < g_argc)
    {
        parse(g_argv + g_arg);
        g_arg += arguments;

        return true;
    }

    std::cout << "Usage: '" << usage << "'" << std::endl;
    exit(-1);
}

int main(int argc, char** argv)
{
    if (argc <= 1)
    {
        std::cout << R"(
    input file:     -i <path>
    output file:    -o <path>
    translate:      -t <x> <y> <z>
    rotate:         -r <deg> <x> <y> <z>
    scale:          -s <x> <y> <z>
    measure time:   -m
    mesh area:      -a
    mesh volume:    -v
    test point:     -p <x> <y> <z>
)";
        return 0;
    }

    g_argc = argc;
    g_argv = argv;

    Manager manager;
    manager.RegisterImporter("obj", std::make_shared<ObjImporter>());
    manager.RegisterExporter("stl", std::make_shared<StlExporter>());

    std::string ipath, opath;
    std::shared_ptr<TransformModifier> modifier;

    bool measure = false;
    bool area = false;
    bool volume = false;
    std::optional<glm::vec3> point;

    for(g_arg = 1; g_arg < argc;)
    {
        if (!Command("-i", 1, "-i <path>", [&](auto argv)
            {
                ipath = argv[0];
            }))
        if (!Command("-o", 1, "-o <path>", [&](auto argv)
            {
                opath = argv[0];
            }))
        if (!Command("-t", 3, "-t <x> <y> <z>", [&](auto argv)
            {
                if (!modifier)
                    modifier = std::make_shared<TransformModifier>();
                modifier->Translate(glm::vec3(atof(argv[0]), atof(argv[1]), atof(argv[2])));
            }))
        if (!Command("-r", 4, "-r <deg> <x> <y> <z>", [&](auto argv)
            {
                if (!modifier)
                    modifier = std::make_shared<TransformModifier>();
                modifier->Rotate((float)glm::radians(atof(argv[0])), glm::vec3(atof(argv[1]), atof(argv[2]), atof(argv[3])));
            }))
        if (!Command("-s", 3, "-r <x> <y> <z>", [&](auto argv)
            {
                if (!modifier)
                    modifier = std::make_shared<TransformModifier>();
                modifier->Scale(glm::vec3(atof(argv[0]), atof(argv[1]), atof(argv[2])));
            }))
        if (!Command("-m", 0, "", [&](auto argv)
            {
                measure = true;
            }))
        if (!Command("-a", 0, "", [&](auto argv)
            {
                area = true;
            }))
        if (!Command("-v", 0, "", [&](auto argv)
            {
                volume = true;
            }))
        if (!Command("-p", 3, "-p <x> <y> <z>", [&](auto argv)
            {
                point = glm::vec3(atof(argv[0]), atof(argv[1]), atof(argv[2]));
            }))
        {
            std::cout << "Unknown command: " << g_argv[g_arg] << std::endl;
            return -1;
        }
    }

    if (ipath.empty())
    {
        std::cout << "The input path is empty. Use '-i <path>'." << std::endl;
        return -1;
    }

    if (modifier)
        manager.AddModifier(modifier);

    auto start = std::chrono::high_resolution_clock::now();

    Error error;

    error = manager.Import(ipath);
    if(error != Errors::Success)
        std::cout << "Import error: " << error << std::endl;

    auto afterImport = std::chrono::high_resolution_clock::now();

    if (error == Errors::Success && !opath.empty())
    {
        error = manager.Export(opath);
        if (error != Errors::Success)
            std::cout << "Export error: " << error << std::endl;
    }

    auto afterExport = std::chrono::high_resolution_clock::now();

    if (area)
        std::cout << "Area: " << manager.GetMesh()->Area() << std::endl;

    if(volume)
        std::cout << "Volume: " << manager.GetMesh()->Volume() << std::endl;

    if(point)
        std::cout << "Point is " << (manager.GetMesh()->IsInside(point.value()) ? "inside" : "outside") << std::endl;

    auto afterMath = std::chrono::high_resolution_clock::now();

    if (measure)
    {
        std::cout << "Import: " << std::chrono::duration_cast<std::chrono::milliseconds>(afterImport - start).count() << "ms" << std::endl;
        std::cout << "Export: " << std::chrono::duration_cast<std::chrono::milliseconds>(afterExport - afterImport).count() << "ms" << std::endl;
        std::cout << "Math: " << std::chrono::duration_cast<std::chrono::milliseconds>(afterMath - afterExport).count() << "ms" << std::endl;
    }

    return 0;
}
