#ifndef GPC_SCENE_H
#define GPC_SCENE_H

#include "ECS_Defines.h"
#include "EntityManager.h"
#include "SystemManager.h"
#include "ComponentsBlockManager.h"
#include "BehaviorBlockManager.h"
#include "Render3DSystem.h"
#include "TransformComponents.h"
#include "RenderWindow.h"
#include "UI/UiSystem.h"

namespace GPC {
    class SceneManager;
    struct CameraComponent;

    struct SceneInformation {
        RenderWindow* pWindow = nullptr;
        SceneManager* pManager = nullptr;
    };

    struct SceneChangeCtx {
        void* pData         = nullptr;
        uint64_t DataSize   = 0;
    };

    GPC_INHERIT_OBJECT(Scene)
    {
    protected:
        Transform2D*    mp_MainCanvas;
        RenderWindow*   mp_Window;
        SceneManager*   mp_Manager;
        EntityManager   m_EntityManager;
        ComponentsBlockManager m_ComponentBlockManager;
        SystemManager   m_SystemManager;
        BehaviorBlockManager m_BehaviorBlockManager;
        Clock           m_Clock;

        std::queue<EntityID> m_ToDestroy;

        void CreateMainCanvas();

        void KillEntity(EntityID eid);
        void SetCollisionGridOffset(glm::vec2 offset);
        void SetCollisionGridScale(glm::u32vec2 scale);


        //TODO Le rendre mieux car une vielle variable dans le code sérieux c'est naze
        bool m_isSceneVillage = true;

    public:
        void Purge();

        RenderWindow* GetRenderWindow() const;

        Scene();
        ~Scene() override = default;

        ErrorType Create(SceneInformation& info);
        void Destroy();

        virtual ErrorType OnAssetsLoad() = 0;
        virtual void OnAssetsDestroy() = 0;

        virtual ErrorType OnCreate(SceneInformation& info) = 0;
        virtual void OnDestroy();

        virtual ErrorType OnStart(SceneChangeCtx& ctx) { return ErrorType::SUCCESS; };
        virtual ErrorType OnEnd() { return ErrorType::SUCCESS; };

        EntityID CreateEmpty();
        EntityID CreateEntityAs3D();
        EntityID CreateEntityAs2D();

        EntityID CreateModel(
            glm::vec3 position,
            const std::string& texture_name,
            const std::string& model_name,
            Transform3D** pTransform = nullptr,
            Render3DComponent** pRenderer = nullptr
            );
        EntityID CreatePhysicModel(
            glm::vec3 position,
            const std::string& texture_name,
            const std::string& model_name,
            bool is_sphere,
            Transform3D** pTransform = nullptr,
            Render3DComponent** pRenderer = nullptr,
            Collision3DComponent** pCollider = nullptr
            );
        EntityID CreateCube(glm::vec3 position, const std::string& texture_name, Transform3D** pTransform = nullptr, Render3DComponent** pRenderer = nullptr);
        EntityID CreateSphere(glm::vec3 position, const std::string& texture_name, Transform3D** pTransform = nullptr, Render3DComponent** pRenderer = nullptr);
        EntityID CreatePlane(glm::vec3 position, const std::string& texture_name, Transform3D** pTransform = nullptr, Render3DComponent** pRenderer = nullptr);

        EntityID CreateEntityAs3D(EntityID parent);
        EntityID CreateEntityAs3D(Transform3D* pParent);

        bool IsAlive(EntityID eid);
        uint32_t GetEntityCount();
        void DestroyEntity(EntityID eid);
        void DestroyEntityRecursive(EntityID eid);

        template<typename Component_t>
        requires(std::is_base_of_v<Component, Component_t>)
        Component_t* AddComponent(EntityID eid);

        template<typename Component_t>
        requires(std::is_base_of_v<Component, Component_t>)
        bool HasComponent(EntityID eid);

        template<typename Component_t>
        requires(std::is_base_of_v<Component, Component_t>)
        void RemoveComponent(EntityID eid);

        template<typename Component_t>
        requires(std::is_base_of_v<Component, Component_t>)
        Component_t* GetComponent(EntityID eid);

        template<typename Component_t>
        requires(std::is_base_of_v<Component, Component_t>)
        CBlockPtr<Component_t> GetComponentBlock();

        template<typename System_t>
        requires(std::is_base_of_v<System, System_t>)
        System_t* GetSystem()
        {
            auto sp = m_SystemManager.GetSystem<System_t>();
            return sp.get();
        }

        template<typename Behavior_t>
        requires(std::is_base_of_v<Behavior, Behavior_t>)
        Behavior_t* AddBehavior(EntityID eid);

        template<typename Behavior_t>
        requires(std::is_base_of_v<Behavior, Behavior_t>)
        void RemoveBehavior(EntityID eid);

        template<typename Behavior_t>
        requires(std::is_base_of_v<Behavior, Behavior_t>)
        Behavior_t* GetBehavior(EntityID eid);

        void ReportCollision3D(EntityID eid1, EntityID eid2, BehaviorCollision3DContext* ctx1, BehaviorCollision3DContext* ctx2);
        void ReportCollision2D(EntityID eid1, EntityID eid2, BehaviorCollision2DContext* ctx1, BehaviorCollision2DContext* ctx2);
        bool RayCast3D(RayHit& hit, Ray ray);

        CameraComponent* GetActiveCamera();
        glm::vec2 WorldToScreen(glm::vec3 position);
        RayHit GetCameraRay(glm::vec2 screen_position);

        void SwapToScene(const std::string &name, SceneChangeCtx &ctx);

        void EnableUI() { m_SystemManager.GetSystem<UiSystem>()->EnableUI(); }
        void DisableUI() { m_SystemManager.GetSystem<UiSystem>()->DisableUI(); }

        virtual void OnSceneBeginUpdateScene();

        virtual void OnSceneBeginDraw3D();
        virtual void OnSceneUpdate3D();
        virtual void OnSceneEndDraw3D();

        virtual void OnSceneBeginDraw2D();
        virtual void OnSceneUpdate2D();
        virtual void OnSceneEndDraw2D();

        virtual void OnSceneBeginCalculation();
        virtual void OnSceneUpdateFrameCalculation();
        virtual void OnSceneEndCalculation();

        void UpdateShadows(const std::vector<Light*>& light, glm::vec3 camPos, bool isSceneVillage);

        virtual void OnSceneBeginDrawParticles();
        virtual void OnSceneUpdateFrameParticles();
        virtual void OnSceneEndDrawParticles();

        virtual void OnSceneEndUpdateScene();

        virtual void OnSceneUpdate();

        virtual void OnSceneBehaviorUpdate();
        virtual void OnSceneBehaviorStart();

        void Update();
    };

} // GPC

#include "Scene.hpp"

#endif // GPC_SCENE_H
