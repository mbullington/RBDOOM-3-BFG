cd ..
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 17" -A x64 ../neo
pause