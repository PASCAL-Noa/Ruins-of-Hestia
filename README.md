## Ruins of Heastia v1.0

>This release marks the culmination of 2 months of intensive development. It covers everything from the creation of the foundational building blocks of the custom C++ engine (GPC Engine) to the full implementation of the Village management and expedition gameplay loop.

<p align="center">
  <img width="60%" alt="affiche A4" src="https://github.com/user-attachments/assets/1ac52a4c-319c-4e1e-a2a5-871087338e0a" />
</p>

---

### Engine & Architecture

- ECS Architecture: Implementation of an Entity-Component-System model ensuring code modularity and maintainability.

- Vulkan Rendering: Implementation of the graphics pipeline using Vulkan. Integration of shadows and an advanced particle system fully computed on the GPU via Compute Shaders.

- Audio Engine: Development of a native and spatial sound system. Support for WAV files, channel separation (Music / SFX), looping management, and real-time volume control via settings.

- Asset Manager: Optimized loading of resources (textures, animated spritesheets, shaders, audio).

- Reactive UI Builder: Creation of a declarative API (UiBuilder) to generate complex interfaces, including a strict Hitbox system, dynamic anchoring, and data-binding (UiReactive) for real-time updates.

- Save System: Implementation of binary trees for the save system.

- Easing: Implementation of a Tweening system, useful for easily creating numerous effects using math.

- FBX Loader: The engine is capable of playing FBX animations.BX.

--- 

### Gameplay : The Village

<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/dbc68a6f-de2c-4984-8f92-ea1328120a8f" />
</p>

- Construction & Placement: Spatial grid system allowing players to add, move, or store buildings (Farms, Houses, Forge) with collision management and instance-bound callbacks.
  
<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/28140309-7b5e-4854-99c0-0dcb7c3ec3cf" />
</p>

- Shop & Inventory Management: Tracking of primary resources (Wood, Stone, Metal, Fiber, Food) alongside the storage of village equipment and gems. With these resources, you can purchase new buildings and upgrade your equipment in the forge.
  
<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/e360efb0-ec8f-4790-aed0-e685fc8ecb84" />
</p>

- Saves & Settings: Serialization of the village state (resources, placements) and user settings management (saving audio preferences).

<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/bfbcffcb-bef3-4864-ad26-71f1b7da1d39" />
</p>

--- 

### Gameplay : The Expedition

<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/14ea38db-8b46-42ae-abbf-165a5a68a05f" />
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/0a61c36a-130e-49bd-87e1-350641ac1477" />
</p>

- Procedural Generation: Creation of unique environments for every run. The map, enemy distribution, and resources are dynamically generated to ensure infinite replayability.

<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/db06da5e-a0de-4ebe-9bad-cb00a8cf2af2" />
</p>

- Resource Gathering: Level exploration allows players to collect materials and gems that will be used back at the village for crafting and development.

<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/dc83eec0-ff4b-43b9-949b-19ec39440376" />
</p>

- Combat & Mobs: This is where the stats of the equipment forged in the village truly shine.

<p align="center">
<img width="70%"  alt="image" src="https://github.com/user-attachments/assets/44b0309c-6c68-4cd5-9dcb-cd7a1352465c" />
</p>

---

### Built With

<p align="center">
  <img src="https://img.shields.io/badge/Vulkan-C41E3A?style=for-the-badge&logo=vulkan&logoColor=white" alt="Vulkan" />
  <img src="https://img.shields.io/badge/CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white" alt="CMake" />
  <img src="https://img.shields.io/badge/RtAudio-2D3436?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="RtAudio" />
  <img src="https://img.shields.io/badge/GLM-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="GLM" />
  <img src="https://img.shields.io/badge/GLFW-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="GLFW" />
  <img src="https://img.shields.io/badge/CLion-000000?style=for-the-badge&logo=clion&logoColor=white" alt="CLion" />
  <img src="https://img.shields.io/badge/Git-F05032?style=for-the-badge&logo=git&logoColor=white" alt="Git" />
</p>


