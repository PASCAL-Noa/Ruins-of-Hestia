 - Il faut CLion ou un compilateur MinGW (peu importe la version) (Si vous voulez je peux vous donner des plugins pour le rendre tout beau)
 - Il faut lancer le BUILD.bat
 - Installer le SDK de vulkan (prenez rien des fonctions avancées genre GLM faut juste l'installer et mettre le minimum)
 - Mettre dans les options de compilation CMake (oubliez pas le "-"): 
> -DCMAKE_TOOLCHAIN_FILE=C:/Users/[USERNAME]/vcpkg/scripts/buildsystems/vcpkg.cmake