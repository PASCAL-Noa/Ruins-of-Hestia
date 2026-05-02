#pragma once
#include "Behavior.h"
#include "Render3DSystem.h"
#include "Village/Building.h"
#include "Village/BuildingManager.h"
#include "Village/Map/Map.h"
#include "TransformComponents.h"
#include "Village/BuildingData.h"

namespace GPC
{
    class Map;
    class VillageManager;

    class Editor : public Behavior
    {
    public:
        INHERIT_BEHAVIOR_CONSTRUCTOR(Editor);

        void                        OnCreate(const BehaviorCreateContext *pCtx) override;
        void                        OnUpdate(const BehaviorUpdateContext *pCtx) override;
        void                        OnDestroy() override;

        void                        SetMap(Map* pMap);
        void                        SetCursor(Transform3D* transform) { mp_CursorTransform = transform; };
        void                        SetVillageManager(VillageManager* pVillageManager);
        void                        SetPlannerActive(bool active);
        void                        SetFullUIActive(bool active);
        void                        SetCompactUIActive(bool active);
        void                        SetCursorAt(glm::vec3 pos);
        glm::ivec2                  GetCursor();
        Map*                        GetMap() { return m_pMap; };

        void                        SetPlacementMode(bool mode);
        bool                        GetPlacementMode() const { return m_RemoveMode; };
    private:
        struct GhostState
        {
            BuildingType            Type;
            std::string             MeshPath;
            glm::ivec2              Size;
            glm::vec2               Offset;
            float                   Scale;
            float                   SpriteDepthFactor;
            float                   SpriteLateralFactor;
            bool                    HasStock;
        };

        void                        CreateUI();
        void                        ClampCursorPos();

        [[nodiscard]] GhostState    GetCurrentGhostState() const;
        void                        UpdateGhostGeometry(const GhostState& state);
        void                        UpdateGhostPosition(const GhostState& state) const;
        void                        UpdateGhostTint(const GhostState& state) const;

        void                        SelectorInputs(const BehaviorUpdateContext *pCtx);
        void                        UpdateGhostVisuals();
        void                        PlaceBuilding();
        void                        StoreBuilding();

        Transform3D*                m_pTransform;
        Render3DComponent*          m_pRender3D;

        EntityID                    m_GhostEntity = 0;
        Transform3D*                m_pGhostTransform = nullptr;
        Render3DComponent*          m_pGhostRender = nullptr;

        Transform3D*                mp_CursorTransform;
        glm::vec3                   m_SelectorGridPos;
        bool                        m_RemoveMode;
        bool                        m_LastRemoveMode;
        int                         m_CurrentTypeIdx;
        uint32_t                    m_BuildingCount = 0;

        Scene*                      m_pScene = nullptr;
        Map*                        m_pMap;
        VillageManager*             m_pVillageManager;
        UiCanvas*                   m_EditorCanvas;
        std::vector<UiSprite*>      m_EditorSprites;
        std::vector<UiText*>        m_EditorTexts;
        UiText*                     m_EditorPlacementText;
        UiSprite*                   m_SaveButtonSprite;

        bool                        m_IsActive = false;

        BuildingType                m_LastTypeIdx = BuildingType::Count;
        bool                        m_LastStockState = false;

        void                        CycleToNextAvailable();
        UiSystem*                   mp_UiSystem;

        [[nodiscard]] glm::ivec2    GetCursorGridPosition() const;
        [[nodiscard]] bool          IsCursorHoveringBuilding(const glm::ivec2& cursorPos, const BuildingData& building) const;
    };
}