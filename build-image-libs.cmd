@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x86 -host_arch=x64
if errorlevel 1 exit /b %errorlevel%
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-image-libs.ps1"
exit /b %errorlevel%