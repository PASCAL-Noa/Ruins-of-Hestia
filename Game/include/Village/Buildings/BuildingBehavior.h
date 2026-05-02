#pragma once
#include <functional>

#include "Behavior.h"
#include "UI/UiCanvas.h"
#include "Village/Building.h"

namespace GPC
{
    class VillageManager;

    struct BuildingBehavior : public Behavior
    {

        INHERIT_BEHAVIOR_CONSTRUCTOR(BuildingBehavior);
        Building*                           Building_               = nullptr;
        VillageManager*                     VillageManager_         = nullptr;
        glm::vec2                           Position                = glm::vec2(0.0f, 0.0f);
        bool                                IsInteracting           = false;
        bool                                IsActive                = false;

        std::function<void()>               OnRecapInteract         = nullptr;

        Render3DComponent*                  Render3DComponent_       = nullptr;
        Collision3DComponent*               Collision3DComponent_    = nullptr;
        std::string                         CurrentBasePath;

        virtual void OnInteract();
        virtual void OnNextCycle()  = 0;
        void Focus(glm::vec2 offset);

        virtual void CloseOpened();

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

    protected:
        void CreateBaseInteractionUI(const BehaviorCreateContext *pCtx, UiCanvas **canvas);

        Scene*                              LocalScene;

    };

} // GPC