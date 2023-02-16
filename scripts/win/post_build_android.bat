@echo off

set RELATIVE_DIR=..\..
set OUT_DIR=%RELATIVE_DIR%\out
set BUILD_DIR=RELATIVE_DIR%\android\app\build\outputs\apk\release
set ARTIFACT=app-release.apk

robocopy %BUILD_DIR% %OUT_DIR% %ARTIFACT% /NFL /NDL /NJH /NJS /nc /ns /np