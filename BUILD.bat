@echo off
setlocal

REM ===== CONFIG =====
set PROJECT_DIR=%cd%
set VCPKG_DIR=%USERPROFILE%\vcpkg
set TRIPLET=x64-windows

echo ========================================
echo Installation automatique VCPKG + Vulkan + GLM
echo ========================================

REM ===== INSTALL VCPKG IF NOT EXISTS =====
if not exist "%VCPKG_DIR%" (
    echo Clonage de vcpkg...
    git clone https://github.com/microsoft/vcpkg "%VCPKG_DIR%"
    if errorlevel 1 (
        echo ERREUR: Impossible de cloner vcpkg
        pause
        exit /b 1
    )
) else (
    echo Mise a jour vcpkg...
    cd /d "%VCPKG_DIR%"
    git pull
    cd /d "%PROJECT_DIR%"
)

cd /d "%VCPKG_DIR%"

REM ===== FORCE BOOTSTRAP =====
echo Bootstrap de vcpkg...
call bootstrap-vcpkg.bat

if errorlevel 1 (
    echo ERREUR: Bootstrap vcpkg a echoue
    pause
    exit /b 1
)

echo Activation integration globale...
vcpkg integrate install

REM ===== INSTALL DEPENDENCIES =====
echo Installation Vulkan...
vcpkg install vulkan

echo Installation GLM...
vcpkg install glm
vcpkg install freetype

echo Installation GLFW...
vcpkg install glfw3

REM ===== RETURN TO PROJECT =====
cd /d "%PROJECT_DIR%"

echo Configuration CMake...
cmake -S . -B build ^
-DCMAKE_TOOLCHAIN_FILE=%VCPKG_DIR%\scripts\buildsystems\vcpkg.cmake ^
-DVCPKG_TARGET_TRIPLET=%TRIPLET%

if errorlevel 1 (
    echo ERREUR CMake
    pause
    exit /b 1
)

echo ========================================
echo Installation terminee avec succes !
echo ========================================

pause
