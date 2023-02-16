@echo off
SET vsTools=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools
SET cwd=%cd%\..\..\android

"%vsTools%\VsDevCmd.bat" && cd %cwd% && gradlew build