@echo off

set OUT_DIR="%~dp0/../../dist"
set BUILD_DIR="%~dp0/../../build"
set PLUGIN_NAME="ScaleRemapper"
set PLUGIN_DIR="%BUILD_DIR%/%PLUGIN_NAME%_artefacts"

rmdir %OUT_DIR% /s/q
mkdir %OUT_DIR%

cd /d %BUILD_DIR%

for /R "%PLUGIN_DIR%" %%f in (*.exe) do copy "%%f" %OUT_DIR%
for /R "%PLUGIN_DIR%" %%f in (*.vst3) do copy "%%f" %OUT_DIR%
for /R "%PLUGIN_DIR%" %%f in (*.dll) do copy "%%f" %OUT_DIR%
