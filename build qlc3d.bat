mkdir build-release
cd build-release

cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++ -static" .. -G "MinGW Makefiles"


mingw32-make.exe -j10