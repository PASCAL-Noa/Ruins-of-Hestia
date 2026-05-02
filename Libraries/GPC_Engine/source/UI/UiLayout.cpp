#include "UI/UiLayout.h"

#include "Scene.h"
#include "TransformComponents.h"
#include "UI/UiCanvas.h"
#include "UI/UiSprite.h"
#include "UI/UiText.h"

namespace GPC
{
    static void ResolveRow(Scene& scene, Transform2D* parent, const UiLayout& layout);
    static void ResolveColumn(Scene& scene, Transform2D* parent, const UiLayout& layout);
    static void ResolveGrid(Scene& scene, Transform2D* parent, const UiLayout& layout);

    static glm::vec2 GetElementSize(Scene& scene, EntityID eid)
    {
        if (scene.HasComponent<UiSprite>(eid)) return scene.GetComponent<UiSprite>(eid)->GetSize();
        if (scene.HasComponent<UiText>(eid))   return scene.GetComponent<UiText>(eid)->GetSize();
        if (scene.HasComponent<UiCanvas>(eid)) return scene.GetComponent<UiCanvas>(eid)->GetSize();
        return { 0.0f, 0.0f };
    }

    void ResolveLayoutFor(Scene& scene, EntityID rootId)
    {
        auto* layout = scene.GetComponent<UiLayout>(rootId);
        if (!layout) return;

        auto* rootTransform = scene.GetComponent<Transform2D>(rootId);
        if (!rootTransform) return;

        switch (layout->Mode)
        {
            case UiLayoutMode::Row:    ResolveRow(scene, rootTransform, *layout); break;
            case UiLayoutMode::Column: ResolveColumn(scene, rootTransform, *layout); break;
            case UiLayoutMode::Grid:   ResolveGrid(scene, rootTransform, *layout); break;
            case UiLayoutMode::Stack:
            default:
                break;
        }
    }

    static glm::vec2 GetParentSize(Scene& scene, Transform2D* parent)
    {
        const EntityID parentId = parent->GetEntityID();
        if (auto* canvas = scene.GetComponent<UiCanvas>(parentId))
        {
            return canvas->GetSize();
        }
        return { 0.0f, 0.0f };
    }

    static float SumFlexGrow(Scene& scene, Transform2D* parent)
    {
        float total = 0.0f;
        for (uint32_t i = 0; i < parent->GetChildCount(); ++i)
        {
            auto* child = parent->GetChild(i);
            if (!child) continue;
            auto* cl = scene.GetComponent<UiLayout>(child->GetEntityID());
            if (cl) total += cl->FlexGrow;
        }
        return total;
    }

    static void ResolveRow(Scene& scene, Transform2D* parent, const UiLayout& layout)
    {
        const glm::vec2 parentSize = GetParentSize(scene, parent);
        const float paddingLeft  = layout.Padding.x;
        const float paddingTop   = layout.Padding.y;
        const float paddingRight = layout.Padding.z;

        const uint32_t childCount = parent->GetChildCount();
        if (childCount == 0) return;

        const float gapTotal = layout.Gap * static_cast<float>(childCount > 0 ? childCount - 1 : 0);
        const float availableWidth = parentSize.x - paddingLeft - paddingRight - gapTotal;
        const float flexTotal = SumFlexGrow(scene, parent);

        float cursorX = paddingLeft;
        for (uint32_t i = 0; i < childCount; ++i)
        {
            auto* child = parent->GetChild(i);
            if (!child) continue;

            const EntityID childId = child->GetEntityID();
            auto* cl = scene.GetComponent<UiLayout>(childId);
            const glm::vec2 childSize = GetElementSize(scene, childId);

            float width = childSize.x;
            if (cl && cl->FlexGrow > 0.0f && flexTotal > 0.0f)
            {
                width = (availableWidth * cl->FlexGrow) / flexTotal;
                if (scene.HasComponent<UiCanvas>(childId)) scene.GetComponent<UiCanvas>(childId)->SetSize({ width, childSize.y });
                else if (scene.HasComponent<UiSprite>(childId)) scene.GetComponent<UiSprite>(childId)->SetSize({ width, childSize.y });
            }

            child->LocalTransform.SetPosition({ cursorX, paddingTop, 0.0f });
            cursorX += width + layout.Gap;
        }
    }

    static void ResolveColumn(Scene& scene, Transform2D* parent, const UiLayout& layout)
    {
        const glm::vec2 parentSize = GetParentSize(scene, parent);
        const float paddingLeft   = layout.Padding.x;
        const float paddingTop    = layout.Padding.y;
        const float paddingBottom = layout.Padding.w;

        const uint32_t childCount = parent->GetChildCount();
        if (childCount == 0) return;

        const float gapTotal = layout.Gap * static_cast<float>(childCount > 0 ? childCount - 1 : 0);
        const float availableHeight = parentSize.y - paddingTop - paddingBottom - gapTotal;
        const float flexTotal = SumFlexGrow(scene, parent);

        float cursorY = paddingTop;
        for (uint32_t i = 0; i < childCount; ++i)
        {
            auto* child = parent->GetChild(i);
            if (!child) continue;

            const EntityID childId = child->GetEntityID();
            auto* cl = scene.GetComponent<UiLayout>(childId);
            const glm::vec2 childSize = GetElementSize(scene, childId);

            float height = childSize.y;
            if (cl && cl->FlexGrow > 0.0f && flexTotal > 0.0f)
            {
                height = (availableHeight * cl->FlexGrow) / flexTotal;
                if (scene.HasComponent<UiCanvas>(childId)) scene.GetComponent<UiCanvas>(childId)->SetSize({ childSize.x, height });
                else if (scene.HasComponent<UiSprite>(childId)) scene.GetComponent<UiSprite>(childId)->SetSize({ childSize.x, height });
            }

            child->LocalTransform.SetPosition({ paddingLeft, cursorY, 0.0f });
            cursorY += height + layout.Gap;
        }
    }

    static void ResolveGrid(Scene& scene, Transform2D* parent, const UiLayout& layout)
    {
        const glm::vec2 parentSize = GetParentSize(scene, parent);
        if (layout.GridCols == 0 || layout.GridRows == 0) return;

        const float paddingLeft = layout.Padding.x;
        const float paddingTop  = layout.Padding.y;

        const float cellWidth  = (parentSize.x - paddingLeft - layout.Padding.z
                                  - layout.Gap * static_cast<float>(layout.GridCols - 1))
                                 / static_cast<float>(layout.GridCols);
        const float cellHeight = (parentSize.y - paddingTop - layout.Padding.w
                                  - layout.Gap * static_cast<float>(layout.GridRows - 1))
                                 / static_cast<float>(layout.GridRows);

        const uint32_t childCount = parent->GetChildCount();
        for (uint32_t i = 0; i < childCount; ++i)
        {
            auto* child = parent->GetChild(i);
            if (!child) continue;

            const uint32_t col = i % layout.GridCols;
            const uint32_t row = i / layout.GridCols;
            if (row >= layout.GridRows) break;

            const float x = paddingLeft + static_cast<float>(col) * (cellWidth  + layout.Gap);
            const float y = paddingTop  + static_cast<float>(row) * (cellHeight + layout.Gap);

            child->LocalTransform.SetPosition({ x, y, 0.0f });
        }
    }
}
