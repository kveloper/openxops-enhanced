@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x86 -host_arch=x64
if errorlevel 1 exit /b %errorlevel%
"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" "%~dp0repos\X2\X2.vcxproj" /p:Configuration=Release /p:Platform=Win32 /p:PreferredToolArchitecture=x64 /m /nologo /verbosity:minimal /clp:ErrorsOnly "/flp:logfile=%~dp0opengl-build.log;verbosity=normal"
exit /b %errorlevel%