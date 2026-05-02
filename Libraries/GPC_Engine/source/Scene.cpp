#include "Scene.h"

#include "Assets.h"
#include "Collision3DSystem.h"
#include "Collision2DSystem.h"
#include "AudioSystem.h"
#include "AnimationSystem.h"
#include "CameraSystem.h"
#include "LightingSystem.h"
#include "ParticleSystem.h"
#include "Profiler.h"
#include "Render3DSystem.h"
#include "SceneManager.h"
#include "TweenSystem.h"
#include "UI/UiAnimComponent.h"
#include "UI/UiCanvas.h"
#include "UI/UiLayout.h"
#include "UI/UiSprite.h"
#include "UI/UiSystem.h"
#include "UI/UiText.h"

namespace GPC {

    void Scene::CreateMainCanvas()
    {
        auto canvas = CreateEntityAs2D();
        auto canvasComponent = AddComponent<UiCanvas>(canvas);
        mp_MainCanvas = GetComponent<Transform2D>(canvas);
        canvasComponent->SetTint(Color(glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f }));
    }

    void Scene::Purge() {

        while (!m_ToDestroy.empty()) {
            EntityID eid = m_ToDestroy.front();
            if (m_EntityManager.IsEntityAlive(eid)) {
                KillEntity(eid);
            }
            m_ToDestroy.pop();
        }

    }

    RenderWindow* Scene::GetRenderWindow() const
    {
        return mp_Window;
    }


    void Scene::KillEntity(EntityID eid) {
        for (uint32_t i = 0; i < MAX_COMPONENTS; ++i) {
            m_ComponentBlockManager.RemoveComponent(eid, i);
        }
        m_BehaviorBlockManager.GetBlock(eid)->ClearBehaviors();
        m_EntityManager.DestroyEntity(eid);
    }

    void Scene::SetCollisionGridOffset(glm::vec2 offset) {
        m_SystemManager.GetSystem<Collision3DSystem>()->GridOffset = {
            offset.x * static_cast<float>(m_SystemManager.GetSystem<Collision3DSystem>()->CellSize.x ) * static_cast<float>(Collision3DSystem::GridSize.x),
            offset.y * static_cast<float>(m_SystemManager.GetSystem<Collision3DSystem>()->CellSize.y ) * static_cast<float>(Collision3DSystem::GridSize.y)
        };
    }

    void Scene::SetCollisionGridScale(glm::u32vec2 scale) {
        m_SystemManager.GetSystem<Collision3DSystem>()->CellSize = scale;
    }

    Scene::Scene() :
        mp_Window(nullptr),
        m_EntityManager(),
        m_ComponentBlockManager(),
        m_SystemManager(),
        m_BehaviorBlockManager()
    { }

    ErrorType Scene::Create(SceneInformation &info)
    {
        mp_Window = info.pWindow;
        mp_Manager = info.pManager;

        m_SystemManager.UpdateAllCtxScene(this);

        OnAssetsLoad();
        CreateMainCanvas();
        OnCreate(info);

        return ErrorType::SUCCESS;
    }

    void Scene::OnDestroy() {
        uint32_t count = m_EntityManager.GetLivingEntitiesCount();
        EntityID* buffer = new EntityID[count];
        memcpy(buffer, m_EntityManager.GetLivingEntities(), sizeof(EntityID) * count);

        for (uint32_t i = 0; i < count; ++i) {
            DestroyEntity(buffer[i]);
        }

        delete[] buffer;

        Purge();
    }

    EntityID Scene::CreateEmpty() {
        return m_EntityManager.CreateEntity();
    }

    EntityID Scene::CreateEntityAs3D() {
        EntityID eid = CreateEmpty();
        AddComponent<Transform3D>(eid);
        return eid;
    }

    EntityID Scene::CreateEntityAs2D() {
        EntityID eid = CreateEmpty();
        AddComponent<Transform2D>(eid);
        return eid;
    }

    EntityID Scene::CreateModel(glm::vec3 position, const std::string &texture_name, const std::string &model_name, Transform3D **pTransform, Render3DComponent **pRenderer) {
        EntityID id = CreateEntityAs3D();

        Transform3D *transform = GetComponent<Transform3D>(id);
        if (pTransform != nullptr) *pTransform = transform;

        transform->LocalTransform.SetPosition(position);
        auto renderer = AddComponent<Render3DComponent>(id);
        renderer->SetTexture("1024x1024", texture_name);
        renderer->pGeometry = GPC_MESH(model_name);
        if (pRenderer != nullptr) *pRenderer = renderer;

        return id;
    }

    EntityID Scene::CreatePhysicModel(glm::vec3 position, const std::string &texture_name, const std::string &model_name, bool is_sphere, Transform3D **pTransform, Render3DComponent **pRenderer, Collision3DComponent **pCollider) {
        EntityID id = CreateEntityAs3D();

        if (pTransform != nullptr) *pTransform = GetComponent<Transform3D>(id);

        auto renderer = AddComponent<Render3DComponent>(id);
        renderer->SetTexture("1024x1024", texture_name);
        renderer->pGeometry = GPC_MESH(model_name);
        if (pRenderer != nullptr) *pRenderer = renderer;

        auto collider = AddComponent<Collision3DComponent>(id);
        collider->IsSphere = is_sphere;
        collider->IsKinematic = false;
        collider->IsTrigger = false;
        collider->CollType = ColliderType::Props;
        if (pCollider != nullptr) *pCollider = collider;

        return id;
    }

    EntityID Scene::CreateCube(glm::vec3 position, const std::string &texture_name, Transform3D **pTransform, Render3DComponent **pRenderer) {
        return CreateModel(position, texture_name, "Cube", pTransform, pRenderer);
    }

    EntityID Scene::CreateSphere(glm::vec3 position, const std::string &texture_name, Transform3D **pTransform, Render3DComponent **pRenderer) {
        return CreateModel(position, texture_name, "Sphere", pTransform, pRenderer);
    }

    EntityID Scene::CreatePlane(glm::vec3 position, const std::string &texture_name, Transform3D **pTransform, Render3DComponent **pRenderer) {
        return CreateModel(position, texture_name, "Plane", pTransform, pRenderer);
    }

    EntityID Scene::CreateEntityAs3D(EntityID parent) {
        EntityID eid = CreateEmpty();
        AddComponent<Transform3D>(eid)->SetParent(GetComponent<Transform3D>(parent));
        return eid;
    }

    EntityID Scene::CreateEntityAs3D(Transform3D *pParent) {
        EntityID eid = CreateEmpty();
        AddComponent<Transform3D>(eid)->SetParent(pParent);
        return eid;
    }

    bool Scene::IsAlive(EntityID eid) {
        return m_EntityManager.IsEntityAlive(eid);
    }

    uint32_t Scene::GetEntityCount()
    {
        return m_EntityManager.GetLivingEntitiesCount();
    }


    static bool HasAnyUiComponent(Scene& scene, EntityID eid) {
        return scene.HasComponent<UiCanvas>(eid)
            || scene.HasComponent<UiSprite>(eid)
            || scene.HasComponent<UiText>(eid)
            || scene.HasComponent<UiLayout>(eid)
            || scene.HasComponent<UiAnimComponent>(eid);
    }

    static void EnqueueDescendantsDFS(Scene& scene, std::queue<EntityID>& queue, EntityID eid) {
        auto* tr = scene.HasComponent<Transform2D>(eid) ? scene.GetComponent<Transform2D>(eid) : nullptr;
        if (!tr) return;

        std::vector<Transform3D*> stack;
        stack.reserve(tr->GetChildCount());
        for (uint32_t i = 0; i < tr->GetChildCount(); ++i) {
            stack.push_back(tr->GetChild(i));
        }

        while (!stack.empty()) {
            Transform3D* node = stack.back();
            stack.pop_back();
            if (!node) continue;

            for (uint32_t i = 0; i < node->GetChildCount(); ++i) {
                stack.push_back(node->GetChild(i));
            }

            queue.push(node->GetEntityID());

            if (auto* uiSys = scene.GetSystem<UiSystem>()) {
                uiSys->OnEntityDestroyed(node->GetEntityID());
            }
        }
    }

    void Scene::DestroyEntity(EntityID eid) {
        if (!m_EntityManager.IsEntityAlive(eid)) return;

        if (auto* uiSys = GetSystem<UiSystem>()) {
            uiSys->OnEntityDestroyed(eid);
        }

        if (HasAnyUiComponent(*this, eid)) {
            EnqueueDescendantsDFS(*this, m_ToDestroy, eid);
        }

        m_ToDestroy.push(eid);
    }

    void Scene::DestroyEntityRecursive(EntityID eid) {
        if (!m_EntityManager.IsEntityAlive(eid)) return;

        if (auto* uiSys = GetSystem<UiSystem>()) {
            uiSys->OnEntityDestroyed(eid);
        }

        EnqueueDescendantsDFS(*this, m_ToDestroy, eid);
        m_ToDestroy.push(eid);
    }

        void Scene::ReportCollision3D(EntityID eid1, EntityID eid2, BehaviorCollision3DContext *ctx1, BehaviorCollision3DContext *ctx2) {
        m_BehaviorBlockManager.GetBlock(eid1)->Collide3D(ctx1);
        m_BehaviorBlockManager.GetBlock(eid2)->Collide3D(ctx2);
    }

    void Scene::ReportCollision2D(EntityID eid1, EntityID eid2, BehaviorCollision2DContext *ctx1, BehaviorCollision2DContext *ctx2) {
        m_BehaviorBlockManager.GetBlock(eid1)->Collide2D(ctx1);
        m_BehaviorBlockManager.GetBlock(eid2)->Collide2D(ctx2);
    }

    bool Scene::RayCast3D(RayHit& hit, Ray ray)
    {
        return m_SystemManager.GetSystem<Collision3DSystem>()->Raycast3D(hit, ray);
    }

    CameraComponent * Scene::GetActiveCamera() {
        return m_SystemManager.GetSystem<CameraSystem>()->CameraActive();
    }

    glm::vec2 Scene::WorldToScreen(glm::vec3 position) {
        glm::mat4 view;
        glm::mat4 proj;
        GetActiveCamera()->Context.GetViewProj(view, proj, static_cast<float>(mp_Window->GetSize().x) / static_cast<float>(mp_Window->GetSize().y));
        glm::vec4 screen_position = proj * view * glm::vec4(position.x, position.y, position.z, 1.0f);
        return glm::vec2(
            screen_position.x * mp_Window->GetSize().x,
            screen_position.y * mp_Window->GetSize().y
        );
    }

    RayHit Scene::GetCameraRay(glm::vec2 screen_position)
    {

        glm::mat4 view;
        glm::mat4 proj;
        auto camera = GetActiveCamera();
        float aspect = static_cast<float>(mp_Window->GetSize().x) / static_cast<float>(mp_Window->GetSize().y);
        camera->Context.GetViewProj(view, proj, aspect);
        auto transform = GetComponent<Transform3D>(camera->GetEntityID());

        proj[1][1] *= -1;
        glm::vec4 viewport = glm::vec4(0, 0, mp_Window->GetSize().x, mp_Window->GetSize().y);

        glm::vec3 ray_origin = glm::unProject(
            glm::vec3(screen_position.x, mp_Window->GetSize().y - screen_position.y, 0.0f),
            view,  // pas d'inverse ici
            proj,
            viewport
        );
        glm::vec3 ray_dir = -transform->LocalTransform.GetForward();

        Ray rayOut = Ray(ray_origin, ray_dir);
        rayOut.maxDistance = FLT_MAX;

        RayHit  hitOut;
        RayCast3D(hitOut, rayOut);

        return hitOut;

    }

    void Scene::SwapToScene(const std::string &name, SceneChangeCtx& ctx) {
        mp_Manager->SetActiveScene(name, ctx);
    }

    void Scene::OnSceneBeginUpdateScene() {

        SystemContext system_ctx{};
        system_ctx.pClock = &m_Clock;
        system_ctx.pScene = this;
        system_ctx.pWindow = mp_Window;

        // GPC_DEBUG_ONLY { GPC::Profiler::NewTask("Updating Performance", "Updating Total"); }
        mp_Window->Update();
        m_SystemManager.BeginUpdateSystem<ParticleSystem>(&system_ctx);

        mp_Window->BeginDrawing();

        // // Prépare l'update des Transforms World matrix
        // CBlockPtr<Transform3D> transform_3d_block = m_ComponentBlockManager.GetComponentBlock<Transform3D>();
        // Transform3D* transform_3d = transform_3d_block->ComponentData();
        // for (uint32_t i = 0; i < transform_3d_block->GetEntityCount(); ++i) {
        //     transform_3d[i].SetDirty();
        // }

        m_Clock.Update();

        m_SystemManager.BeginUpdateSystem<Collision3DSystem>(&system_ctx);
        m_SystemManager.BeginUpdateSystem<Collision2DSystem>(&system_ctx);
        m_SystemManager.BeginUpdateSystem<AudioSystem>(&system_ctx);
        m_SystemManager.BeginUpdateSystem<LightingSystem>(&system_ctx);
        m_SystemManager.BeginUpdateSystem<AnimationSystem>(&system_ctx);
        m_SystemManager.BeginUpdateSystem<Render3DSystem>(&system_ctx);
        m_SystemManager.BeginUpdateSystem<UiSystem>(&system_ctx);
        m_SystemManager.BeginUpdateSystem<CameraSystem>(&system_ctx);
        m_SystemManager.BeginUpdateSystem<TweenSystem>(&system_ctx);

    }

    void Scene::OnSceneUpdate() {

        m_SystemManager.UpdateSystem<AnimationSystem>();
        m_SystemManager.UpdateSystem<Collision3DSystem>();
        m_SystemManager.UpdateSystem<Collision2DSystem>();
        m_SystemManager.UpdateSystem<AudioSystem>();
        m_SystemManager.UpdateSystem<TweenSystem>();
        m_SystemManager.UpdateSystem<UiSystem>();

        m_SystemManager.LateUpdateSystem<AnimationSystem>();
        m_SystemManager.LateUpdateSystem<Collision3DSystem>();
        m_SystemManager.LateUpdateSystem<Collision2DSystem>();
        m_SystemManager.LateUpdateSystem<AudioSystem>();
        m_SystemManager.LateUpdateSystem<TweenSystem>();
        m_SystemManager.LateUpdateSystem<UiSystem>();
    }

    void Scene::UpdateShadows(const std::vector<Light*>& light, const glm::vec3 camPos, const bool isSceneVillage)
    {
        mp_Window->UpdateShadows(light, camPos, isSceneVillage);
    }

    void Scene::OnSceneBeginCalculation()
    {
        mp_Window->BeginCalculation();
    }

    void Scene::OnSceneUpdateFrameCalculation()
    {
        m_SystemManager.UpdateSystem<ParticleSystem>();
    }

    void Scene::OnSceneEndCalculation()
    {
        mp_Window->EndCalculation();
    }
    
    void Scene::OnSceneBeginDraw3D() {
        // GPC_DEBUG_ONLY { GPC::Profiler::NewTask("Updating 3D Performance", "Updating 3D"); }
        mp_Window->BeginDraw3D();
    }

    void Scene::OnSceneEndDraw3D() {
        mp_Window->EndDraw3D();
        // GPC_DEBUG_ONLY { GPC::Profiler::EndTask("Updating 3D Performance"); }
    }

    void Scene::OnSceneBeginDraw2D() {
        // GPC_DEBUG_ONLY { GPC::Profiler::NewTask("Updating 2D Performance", "Updating 2D"); }
        mp_Window->BeginDraw2D();
    }

    void Scene::OnSceneEndDraw2D() {
        mp_Window->EndDraw2D();
        // GPC_DEBUG_ONLY { GPC::Profiler::EndTask("Updating 2D Performance"); }
    }

    void Scene::OnSceneBeginDrawParticles()
    {
        mp_Window->BeginDrawParticles();
    }

    void Scene::OnSceneUpdateFrameParticles()
    {
        m_SystemManager.LateUpdateSystem<ParticleSystem>();
    }

    void Scene::OnSceneEndDrawParticles()
    {
        mp_Window->EndDrawParticles();
    }

    void Scene::OnSceneBehaviorUpdate() {
        BehaviorUpdateContext behavior_update_ctx{};
        behavior_update_ctx.pScene = this;
        behavior_update_ctx.pClock = &m_Clock;

        uint64_t entities_count = m_EntityManager.GetLivingEntitiesCount();
        EntityID* entities = m_EntityManager.GetLivingEntities();
        for (uint64_t i = 0; i < entities_count; ++i) {
            m_BehaviorBlockManager.GetBlock(entities[i])->Update(behavior_update_ctx);
        }
        for (uint64_t i = 0; i < entities_count; ++i) {
            m_BehaviorBlockManager.GetBlock(entities[i])->LateUpdate(behavior_update_ctx);
        }
    }

    void Scene::OnSceneBehaviorStart() {
        BehaviorCreateContext behavior_create_ctx{};
        behavior_create_ctx.pScene = this;
        behavior_create_ctx.pClock = &m_Clock;

        uint64_t entities_count = m_EntityManager.GetLivingEntitiesCount();
        EntityID* entities = m_EntityManager.GetLivingEntities();
        for (uint64_t i = 0; i < entities_count; ++i) {
            m_BehaviorBlockManager.GetBlock(entities[i])->Start(behavior_create_ctx);
        }
    }

    void Scene::OnSceneUpdate3D() {
        auto& Program = GPC_GRAPHIC_PROGRAM("Default Program")->Program;
        mp_Window->UseProgram(&Program);

        m_SystemManager.UpdateSystem<CameraSystem>();
        m_SystemManager.UpdateSystem<LightingSystem>();
        m_SystemManager.UpdateSystem<Render3DSystem>();

        m_SystemManager.LateUpdateSystem<CameraSystem>();
        m_SystemManager.LateUpdateSystem<LightingSystem>();
        m_SystemManager.LateUpdateSystem<Render3DSystem>();
    }

    void Scene::OnSceneUpdate2D() {
    }


    void Scene::OnSceneEndUpdateScene() {
        m_SystemManager.EndUpdateSystem<Collision3DSystem>();
        m_SystemManager.EndUpdateSystem<Collision2DSystem>();
        m_SystemManager.EndUpdateSystem<AudioSystem>();
        m_SystemManager.EndUpdateSystem<LightingSystem>();
        m_SystemManager.EndUpdateSystem<AnimationSystem>();
        m_SystemManager.EndUpdateSystem<Render3DSystem>();
        m_SystemManager.EndUpdateSystem<CameraSystem>();

        mp_Window->EndDrawing();
        // GPC_DEBUG_ONLY { GPC::Profiler::EndTask("Updating Performance"); }
    }

    void Scene::Update() {
        Purge();
        OnSceneBeginUpdateScene();

        OnSceneBeginCalculation();
        OnSceneUpdateFrameCalculation();
        OnSceneEndCalculation();

        const auto light_component_data = GetComponentBlock<LightingComponent>()->ComponentData();
        const auto light_count = GetComponentBlock<LightingComponent>()->GetEntityCount();
        const EntityID CAMERA_EID = GetActiveCamera()->GetEntityID();

        if (light_count > 0) {
            std::vector<Light*> lights(light_count);
            for (int i = 0; i < light_count; ++i)
            {
                lights[i] = light_component_data[i].pLightData;
            }

            UpdateShadows(lights, GetComponentBlock<Transform3D>()->ComponentData()[CAMERA_EID].LocalTransform.GetPosition(), m_isSceneVillage);
        }

        OnSceneBeginDraw3D();
        OnSceneUpdate3D();
        OnSceneEndDraw3D();

        OnSceneBeginDrawParticles(); // TODO See why my render pass not working
        OnSceneUpdateFrameParticles();
        OnSceneEndDrawParticles();

        OnSceneBeginDraw2D();
        OnSceneUpdate2D();
        OnSceneEndDraw2D();

        OnSceneEndUpdateScene();

        OnSceneUpdate();

        OnSceneBehaviorUpdate();
        OnSceneBehaviorStart();

        mp_Manager->UpdateCurrentScene();
        Inputs::UpdateStates();

    }

    void Scene::Destroy()
    {
        OnAssetsDestroy();
        OnDestroy();
    }

} // GPC