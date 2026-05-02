#pragma once

#include "Behavior.h"
#include "UI/UiElement.h"
#include "UI/UiSignal.h"

namespace GPC
{
    struct UiButton : public Behavior
    {
        INHERIT_BEHAVIOR_CONSTRUCTOR(UiButton);

        UiElement*  pElement   = nullptr;
        Transform2D* pTransform = nullptr;

        UiSignal<> OnClick;
        UiSignal<> OnRelease;
        UiSignal<> OnHover;
        UiSignal<> OnLeave;

        bool IsClicked  = false;
        bool IsHovered  = false;
        bool IsDisabled = false;

        void OnCreate(const BehaviorCreateContext* pCtx) override;
        void OnUpdate(const BehaviorUpdateContext* pCtx) override;

        [[nodiscard]] bool IsMouseOver() const;

        void SetDisabled(bool disabled) { IsDisabled = disabled; }

    private:
        static bool PointInRect(float mx, float my, float rx, float ry, float rw, float rh);
    };
}
