#pragma once

// NOTE : tout header ajoute ici doit utiliser des include guards (#ifndef X_H / #define X_H / #endif) et NON `#pragma once` -> sinon GCC genere des erreurs de redefinition quand un .cpp re-include directement un header deja present dans le PCH (bug connu GCC + PCH + #pragma once).
// IMPORTANT : passer par GLM_Include.h pour que les defines projet (GLM_FORCE_DEPTH_ZERO_TO_ONE, GLM_CONFIG_ALIGNED_GENTYPES, etc.) soient actifs AVANT la 1ere inclusion de glm -> sinon ODR violation.

// STL
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// glm (via wrapper projet)
#include "GLM_Include.h"

// Framework core
#include "Defines.h"
#include "Object.h"
#include "Debugger.h"
#include "CompileTimeHash.h"
#include "Transform.h"
