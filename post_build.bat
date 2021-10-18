@echo off
set PLUGIN_NAME=MidiScaleRemapper
set OUT_DIR=..\dist
set BUILD_DIR=build

cd /d %BUILD_DIR%
rmdir %OUT_DIR% /s/q
mkdir %OUT_DIR%
for /f "delims=" %%i in ('dir /s /b /a-d %PLUGIN_NAME%.exe') do copy /b "%%~i" "%OUT_DIR%"
for /f "delims=" %%i in ('dir /s /b /a-d %PLUGIN_NAME%.vst3') do copy /b "%%~i" "%OUT_DIR%"