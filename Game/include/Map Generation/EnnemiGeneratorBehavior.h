#pragma once

#include "AnimationComponent.h"
#include "Behavior.h"
#include "MapGenerator.h"
#include "Render3DSystem.h"
#include "RandomAudio.h"
#include "UI/UiCanvas.h"

namespace GPC {
    struct BossBarHandler;
    struct LivingEntityBehavior;
    struct EnnemiBehavior;

    struct EnnemiGeneratorBehavior : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(EnnemiGeneratorBehavior);

        GENERATION_ID GenerationID{0};
        Transform3D* pPlayerTransform               = nullptr;
        uint32_t Difficulty                         = 1;
        FbxMeshData* pMeshData                      = nullptr;
        UiCanvas* pMainCanvas                       = nullptr;
        RandomAudio* pEnemiWalk = nullptr;
        float m_EnemiWalkTimer = 0.0f;
        
        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnStart(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        struct EnnemiComponents {
            bool IsAlive = true;
            glm::ivec2 Room;
            EntityID EID = 0;
            EntityID HealthBar = 0;
            Transform3D* pTransform = nullptr;
            Render3DComponent* pRenderer = nullptr;
            Collision3DComponent* pCollider = nullptr;
            EnnemiBehavior* pBehavior = nullptr;
            LivingEntityBehavior* pLiving = nullptr;
            Transform3D* pHealthBarTransform = nullptr;
            Render3DComponent* pHealthBarRenderer = nullptr;
            // AnimationComponent* pAnimation = nullptr;
        };

        EnnemiComponents* GetClosestEnnemi(float& out_distance, glm::vec2& out_direction);
        EnnemiComponents* GetBoss();

        void KillEnnemi(EnnemiComponents* pEnnemi, Scene* pScene);
        void KillEnnemiByEID(EntityID eid, glm::ivec2 room, Scene* pScene);

        void DestroyAllEnnemies(Scene* pScene);

    private:

        void UpdateVisibilityBasedOnDistance();

        void CreateEnnemi(glm::vec2 position, glm::ivec2 room, Scene *pScene);
        void CreateBoss(glm::vec2 position, glm::ivec2 room, Scene *pScene);

        static constexpr float RESCALE_ENNEMI = 0.001f;
        static constexpr float RESCALE_BOSS = 0.002f;

        EnnemiComponents* mp_Boss = nullptr;
        BossBarHandler* mp_BossHealthBar = nullptr;

        Transform3D* mp_Transform = nullptr;
        glm::ivec2 m_CurrentRoom = { 100, 100 };

        static constexpr uint32_t MAX_ENNEMI_BY_ROOM = 50;
        struct Ennemies {
            std::array<EnnemiComponents, MAX_ENNEMI_BY_ROOM> Array;
            uint32_t Count = 0.0f;
        };

        MAP_ARRAY2D<Ennemies> m_SpawnedByRoom;
    };

} // GPC