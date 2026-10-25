$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
& python (Join-Path $root 'tools\fetch-image-libs.py')
if ($LASTEXITCODE) { throw 'Dependency source download or verification failed.' }
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$cmake = Join-Path $vs 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$prefix = Join-Path $root 'lib\modern'

function Build-Library($name, $source, $options) {
    $build = Join-Path $root ".tools\build\$name"
    & $cmake -S (Join-Path $root ".tools\sources\$source") -B $build -G 'Visual Studio 18 2026' -A Win32 `
        "-DCMAKE_INSTALL_PREFIX=$prefix" '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded' '-DCMAKE_SUPPRESS_REGENERATION=ON' @options
    if ($LASTEXITCODE) { throw "Configuration failed: $name" }
    & $cmake --build $build --config Release -- /m:1 /p:PreferredToolArchitecture=x64
    if ($LASTEXITCODE) { throw "Build failed: $name" }
    & $cmake --install $build --config Release
    if ($LASTEXITCODE) { throw "Install failed: $name" }
}

Build-Library 'zlib' 'zlib\zlib-1.3.2' @('-DZLIB_BUILD_SHARED=OFF', '-DZLIB_BUILD_TESTING=OFF')
Build-Library 'jpeg' 'jpeg\libjpeg-turbo-3.2.0' @('-DENABLE_SHARED=OFF', '-DWITH_CRT_DLL=OFF', '-DWITH_SIMD=OFF', '-DWITH_TURBOJPEG=OFF', '-DWITH_TESTS=OFF', '-DWITH_TOOLS=OFF')
Build-Library 'png' 'png\libpng-1.6.59' @('-DPNG_SHARED=OFF', '-DPNG_TESTS=OFF', '-DPNG_TOOLS=OFF', '-DPNG_HARDWARE_OPTIMIZATIONS=OFF', "-DZLIB_ROOT=$prefix", "-DZLIB_LIBRARY=$prefix\lib\zs.lib", "-DZLIB_INCLUDE_DIR=$prefix\include")
$licenses = Join-Path $prefix 'licenses'
New-Item -ItemType Directory -Force -Path $licenses | Out-Null
Copy-Item -LiteralPath (Join-Path $root '.tools\sources\jpeg\libjpeg-turbo-3.2.0\LICENSE.md') -Destination (Join-Path $licenses 'libjpeg-turbo-LICENSE.md')
Copy-Item -LiteralPath (Join-Path $root '.tools\sources\jpeg\libjpeg-turbo-3.2.0\README.ijg') -Destination (Join-Path $licenses 'libjpeg-README.ijg')
Copy-Item -LiteralPath (Join-Path $root '.tools\sources\png\libpng-1.6.59\LICENSE') -Destination (Join-Path $licenses 'libpng-LICENSE')
Copy-Item -LiteralPath (Join-Path $root '.tools\sources\zlib\zlib-1.3.2\LICENSE') -Destination (Join-Path $licenses 'zlib-LICENSE')
