@echo off

set RELATIVE_DIR=..\..
set OUT_DIR=%RELATIVE_DIR%\dist
set BUILD_DIR=%RELATIVE_DIR%\android\app\build\outputs\apk\release
set ARTIFACT=app-release.apk
set FILE_NAME=scale-remapper.apk

robocopy %BUILD_DIR% %OUT_DIR% %ARTIFACT% /NFL /NDL /NJH /NJS /nc /ns /np & cd %OUT_DIR% & rename %ARTIFACT% %FILE_NAME%