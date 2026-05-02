#include "../include/SystemManager.h"

#include "Collision3DSystem.h"
#include "Collision2DSystem.h"
#include "AudioSystem.h"
#include "AnimationSystem.h"
#include "CameraSystem.h"
#include "LightingSystem.h"
#include "ParticleSystem.h"
#include "Render3DSystem.h"
#include "TweenSystem.h"
#include "UI/UiSystem.h"

namespace GPC {
    SystemManager::SystemManager() {
        InitializeSystem<Render3DSystem>();
        InitializeSystem<LightingSystem>();
        InitializeSystem<Collision3DSystem>();
        InitializeSystem<Collision2DSystem>();
        InitializeSystem<AudioSystem>();
        InitializeSystem<CameraSystem>();
        InitializeSystem<ParticleSystem>();
        InitializeSystem<AnimationSystem>();
        InitializeSystem<TweenSystem>();
        InitializeSystem<UiSystem>();
    }
} // GPC