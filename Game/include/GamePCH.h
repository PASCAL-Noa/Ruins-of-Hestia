#pragma once

// NOTE : tout header ajoute ici doit utiliser des include guards (#ifndef X_H / #define X_H / #endif) et NON `#pragma once` -> sinon GCC genere des erreurs de redefinition quand un .cpp re-include directement un header deja present dans le PCH (bug connu GCC + PCH + #pragma once).
// IMPORTANT : passer par GLM_Include.h pour que les defines projet (GLM_FORCE_DEPTH_ZERO_TO_ONE, GLM_CONFIG_ALIGNED_GENTYPES, etc.) soient actifs AVANT la 1ere inclusion de glm -> sinon ODR violation.

// STL
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// glm (via wrapper projet)
#include "GLM_Include.h"

// Engine core stable
#include "ECS_Defines.h"
#include "Behavior.h"
#include "Scene.h"
#include "SceneManager.h"
#include "TransformComponents.h"
#include "Render3DSystem.h"
#include "Inputs.h"

// UI engine stable
#include "UI/UiAnim.h"
#include "UI/UiAnimFluent.h"
#include "UI/UiBuilder.h"
#include "UI/UiButton.h"
#include "UI/UiCanvas.h"
#include "UI/UiSprite.h"
#include "UI/UiText.h"
