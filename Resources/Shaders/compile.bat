@ECHO OFF

set GLSLC=%VULKAN_SDK%/Bin/glslc.exe

for %%f in (*.vert) do (
    "%GLSLC%" "%%f" -o "%%f.spv"
)

for %%f in (*.comp) do (
    "%GLSLC%" "%%f" -o "%%f.spv"
)

for %%f in (*.frag) do (
    "%GLSLC%" "%%f" -o "%%f.spv"
)

for %%f in (*.geom) do (
    "%GLSLC%" "%%f" -o "%%f.spv"
)

pause