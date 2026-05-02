#pragma once

#include "Behavior.h"
#include "CameraSystem.h"
#include "Render3DSystem.h"
#include "RoomMeta.h"
#include "RoomPreviewBehavior.h"
#include "Map Generation/Room.h"

namespace GPC {

    struct RoomEditorBehavior : public Behavior {
        INHERIT_BEHAVIOR_CONSTRUCTOR(RoomEditorBehavior);

        void OnCreate(const BehaviorCreateContext *pCtx) override;
        void OnUpdate(const BehaviorUpdateContext *pCtx) override;

        void GoToEditMode();

        static EntityID CreateEnnemiSpawner(glm::vec2 pos, Transform3D *pParent, Scene *pScene);
        static EntityID CreateResourceSpawner(glm::vec2 pos, Transform3D *pParent, Scene* pScene);
        static EntityID CreatePlayerSpawner(glm::vec2 pos, Transform3D *pParent, Scene *pScene);

        RoomPreviewBehavior* pPreview = nullptr;
    private:

        void CreateTile(glm::ivec2 pos, Scene* pScene);
        void CreateBorder(glm::vec2 pos, glm::vec2 scale, Render3DComponent*& outRenderer, Scene* pScene);
        void CreateCamera(Scene* pScene);
        void CreateSelector(Scene* pScene);

        void UpdateTileVisual();
        void UpdateBorderVisual();
        void UpdateMode();

        void SaveToFile(uint32_t fileIndex);
        void LoadFromFile(uint32_t fileIndex, Scene *pScene);
        void ReGenerateAllSpawnerFromMeta(Scene *pScene);

        void HandleSaveLoadShortcuts(Scene* pScene);

        void DrawAt(glm::ivec2 pos);
        void FillAt(glm::ivec2 pos);
        void EraseAt(glm::ivec2 pos);
        void ClearAt(glm::ivec2 pos);
        void AddEnnemi(glm::vec2 pos, Scene* pScene);
        void AddResource(glm::vec2 pos, Scene* pScene);
        void SetPlayerSpawn(glm::vec2 pos, Scene* pScene);

        void DestroySpawner(EntityID id, Scene* pScene);

        enum EditMode {
            BRUSH,
            FILL,
            ERASER,
            ERASE_FILL,
            ENNEMI_SPAWN,
            RESOURCE_SPAWN,
            PLAYER_SPAWN
        };

        enum class SlotMode { None, Save, Load };

        static constexpr uint32_t MAX_SLOT_INDEX = 999;

        struct DebugTile {
            Transform3D* pTileTransform = nullptr;
            Render3DComponent* pTileRenderer = nullptr;
        };

        struct DebugSpawn {
            Transform3D* pSpawnTransform = nullptr;
            Render3DComponent* pSpawnRenderer = nullptr;
        };

        struct Selector {
            Transform3D* pPointerTransform = nullptr;
            Transform3D* pSelectorTransform = nullptr;
            EntityID RayCastPlaneID = 0;
            glm::ivec2 Position{0, 0};
        };

        Transform3D* mp_Transform = nullptr;

        Transform3D* mp_CameraTransform = nullptr;
        CameraComponent* mp_CameraComponent = nullptr;

        Render3DComponent* mp_BorderUpRenderer = nullptr;
        Render3DComponent* mp_BorderDownRenderer = nullptr;
        Render3DComponent* mp_BorderLeftRenderer = nullptr;
        Render3DComponent* mp_BorderRightRenderer = nullptr;

        RoomMeta m_MetaRoom;
        std::array<std::array<DebugTile, ROOM_SIZE>, ROOM_SIZE> m_DebugGrid{};

        Selector m_Selector;
        EditMode m_CurrentMode = BRUSH;

        SlotMode m_SlotMode      = SlotMode::None;
        uint32_t m_SlotBuffer    = 0;
        bool     m_SlotHasDigit  = false;
        bool     m_SlotCancelled = false;

        std::vector<EntityID> m_LivingSpawners;
    };

} // GPC