#!/bin/bash

set -e

# ===== CONFIG =====
PROJECT_DIR="$(pwd)"
VCPKG_DIR="$HOME/vcpkg"
TRIPLET="x64-linux"

echo "========================================"
echo "Installation automatique VCPKG + Vulkan + GLM"
echo "========================================"

# ===== INSTALL VCPKG IF NOT EXISTS =====
if [ ! -d "$VCPKG_DIR" ]; then
    echo "Clonage de vcpkg..."
    git clone https://github.com/microsoft/vcpkg "$VCPKG_DIR" || {
        echo "ERREUR: Impossible de cloner vcpkg"
        exit 1
    }
else
    echo "Mise a jour vcpkg..."
    cd "$VCPKG_DIR"
    git pull
    cd "$PROJECT_DIR"
fi

cd "$VCPKG_DIR"

# ===== BOOTSTRAP =====
echo "Bootstrap de vcpkg..."
./bootstrap-vcpkg.sh || {
    echo "ERREUR: Bootstrap vcpkg a echoue"
    exit 1
}

echo "Activation integration globale..."
./vcpkg integrate install

# ===== INSTALL DEPENDENCIES =====
echo "Installation Vulkan..."
./vcpkg install vulkan

echo "Installation GLM..."
./vcpkg install glm
./vcpkg install freetype

echo "Installation GLFW..."
./vcpkg install glfw3

# ===== RETURN TO PROJECT =====
cd "$PROJECT_DIR"

echo "Configuration CMake..."
cmake -S . -B build \
-DCMAKE_TOOLCHAIN_FILE="$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake" \
-DVCPKG_TARGET_TRIPLET=$TRIPLET || {
    echo "ERREUR CMake"
    exit 1
}

echo "========================================"
echo "Installation terminee avec succes !"
echo "========================================"