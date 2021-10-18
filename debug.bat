@echo off
rmdir build /s/q
mkdir build
cd /d build
cmake -A x64 ../ && cmake --build . --config Debug