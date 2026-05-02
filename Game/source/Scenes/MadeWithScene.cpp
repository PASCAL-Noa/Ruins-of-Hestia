#include "Scenes/MadeWithScene.h"

#include "UI/UiAnim.h"
#include "UI/UiAnimFluent.h"
#include "UI/UiBuilder.h"
#include "UI/UiCanvas.h"

namespace GPC
{
    ErrorType SceneMadeWith::OnAssetsLoad()
    {
        SceneDefault::OnAssetsLoad();

        LoadTextureArray(
            "583x520",
            583, 520,
            {
                { "Logo", "Resources/Images/LogoEngine.png" },
            }
        );

        LoadTextureArray(
            "203x52",
            203, 52,
            {
                { "Made", "Resources/Images/Made.png" },
            }
        );

        LoadTextureArray(
            "167x44",
            167, 44,
            {
                { "With", "Resources/Images/With.png" },
            }
        );

        return ErrorType::SUCCESS;
    }

    ErrorType SceneMadeWith::OnCreate(SceneInformation& info)
    {
        SceneDefault::OnCreate(info);

        UiBuilder b(this);

        mp_rootCanva = b.At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
            .Tint(Colors::BLACK)
            .FitToScreen(true)
            .BuildCanvas();

        float LogoOffsetX = 70.0f;
        float TextOffsetX = -200.0f;
        float TextOffsetY = -25.0f;

        mp_Sprite = b.ChildOf(mp_rootCanva->GetEntityID())
                            .At(Anchors::MIDDLE_MIDDLE, { LogoOffsetX, 0.0f })
                            .Scale({ 0.5f, 0.5f })
                            .Sprite("583x520", "Logo")
                            .BuildSprite();

        mp_Text1 = b.ChildOf(mp_rootCanva->GetEntityID())
                            .At(Anchors::MIDDLE_MIDDLE, { TextOffsetX, TextOffsetY })
                            .Scale({ 1.0f, 1.0f })
                            .Sprite("203x52", "Made")
                            .BuildSprite();

        mp_Text2 = b.ChildOf(mp_rootCanva->GetEntityID())
                            .At(Anchors::MIDDLE_MIDDLE, { TextOffsetX, TextOffsetY + 50 })
                            .Scale({ 1.0f, 1.0f })
                            .Sprite("167x44", "With")
                            .BuildSprite();


        return ErrorType::SUCCESS;
    }

    ErrorType SceneMadeWith::OnStart(SceneChangeCtx& ctx)
    {
        SceneDefault::OnStart(ctx);

        m_ctx = ctx;

        mp_Sprite->SetTint(Color(1,1,1,0));
        mp_Text1->SetTint(Color(1,1,1,0));
        mp_Text2->SetTint(Color(1,1,1,0));

        auto PlayFade = [this](auto* element, float diff)
        {
            UiAnim::On(*this, element)
            .After(diff)
            .FadeIn(1.0f)
            .After(3.0f)
            .FadeOut(1.0f)
            .OnComplete([this]
            {
                m_AnimFinishedCount++;

                if (m_AnimFinishedCount == 3)
                {
                    m_IsAnimEnded = true;
                }
            })
            .Play();
        };

        PlayFade(mp_Text1, 0.0f);
        PlayFade(mp_Text2, 0.2f);
        PlayFade(mp_Sprite, 0.4f);

        return ErrorType::SUCCESS;
    }

    void SceneMadeWith::OnSceneUpdate()
    {
        SceneDefault::OnSceneUpdate();

        if (m_IsAnimEnded)
        {
            SwapToScene("Main_Menu", m_ctx);
        }

    }

    void SceneMadeWith::OnDestroy()
    {
        SceneDefault::OnDestroy();
    }
}
