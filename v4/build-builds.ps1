cmake -B build/ninja -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake -B build/vs -G "Visual Studio 17 2022" -A x64
