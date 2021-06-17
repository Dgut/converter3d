Requires the GLM library for this project: https://github.com/g-truc/glm

Originally developed on Windows using MSVS 2019 and C++17, it will most likely compile on any other OS.

Available commands:
    input file:     -i <path>
    output file:    -o <path>
    translate:      -t <x> <y> <z>
    rotate:         -r <deg> <x> <y> <z>
    scale:          -s <x> <y> <z>
    measure time:   -m
    mesh area:      -a
    mesh volume:    -v
    test point:     -p <x> <y> <z>

All transformation commands are executed before mathematical commands.

The test.zip archive contains a huge file for the converter performance test.
Use the -m command and the release version of the application to evaluate performance.
