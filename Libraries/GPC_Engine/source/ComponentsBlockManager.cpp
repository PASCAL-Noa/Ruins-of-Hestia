#include "ComponentsBlockManager.h"

#include "AudioListener.h"
#include "AudioSystem.h"
#include "CameraSystem.h"
#include "Collision3DSystem.h"
#include "Collision2DSystem.h"
#include "LightingSystem.h"
#include "ParticleSystem.h"
#include "Render3DSystem.h"
#include "AnimationComponent.h"
#include "TransformComponents.h"
#include "Tween.h"
#include "TweenSystem.h"
#include "UI/Raycastable.h"
#include "UI/UiAnimComponent.h"
#include "UI/UiCanvas.h"
#include "UI/UiLayout.h"
#include "UI/UiSprite.h"
#include "UI/UiText.h"

namespace GPC {

    ComponentsBlockManager::ComponentsBlockManager() {
        InitializeArrays<Transform3D>();
        InitializeArrays<Render3DComponent>();
        InitializeArrays<Batching3DComponent>();
        InitializeArrays<LightingComponent>();
        InitializeArrays<CameraComponent>();
        InitializeArrays<Collision3DComponent>();
        InitializeArrays<Collision2DComponent>();
        InitializeArrays<AudioSource>();
        InitializeArrays<ParticleEmitter>();
        InitializeArrays<AudioListener>();
        InitializeArrays<AnimationComponent>();
        InitializeArrays<TweenComponent>();
        InitializeArrays<UiCanvas>();
        InitializeArrays<UiText>();
        InitializeArrays<UiSprite>();
        InitializeArrays<UiLayout>();
        InitializeArrays<Raycastable>();
        InitializeArrays<UiAnimComponent>();
    }

} // GPC