#pragma once

#include <array>

#include "Color.h"
#include "Scenes/SceneDefault.h"
#include "UI/UiElement.h"
#include "UI/UiReactive.h"

namespace GPC
{
    struct UiCanvas;
    struct UiSprite;
    struct UiText;

    class SceneUiTest : public SceneDefault
    {
    public:
        ErrorType OnAssetsLoad() override;
        ErrorType OnCreate(SceneInformation& info) override;
        void OnSceneUpdate() override;
        void OnDestroy() override;

    private:
        void BuildDemoButtons(UiCanvas* root);
        void BuildAdvancedButtons(UiCanvas* root);
        void BuildExtraTargets(UiCanvas* root);
        UiCanvas* BuildButton(UiCanvas* parent, const std::string& label);
        UiCanvas* BuildSmallButton(UiCanvas* parent, const std::string& label);
        UiCanvas* BuildStyledPanel(UiCanvas* parent, Anchor anchor, glm::vec2 offset,
                                   glm::vec2 size, Color bg, Color border);
        void ResetTarget();
        void ResetExtraTargets();

        UiReactive<std::string> m_Status{ "ready" };

        UiCanvas* mp_Root          = nullptr;
        UiSprite* mp_Target        = nullptr;
        UiText*   mp_StatusLabel   = nullptr;
        UiText*   mp_InstructionLabel = nullptr;
        UiText*   mp_HelpLabel     = nullptr;
        UiText*   mp_StateLabel    = nullptr;

        UiReactive<std::string> m_HelpText { "" };
        UiReactive<std::string> m_StateText{ "IDLE" };

        std::array<UiSprite*, 4> mp_ExtraTargets{ nullptr, nullptr, nullptr, nullptr };
        uint32_t                 m_EasingIndex = 0;
    };
}
