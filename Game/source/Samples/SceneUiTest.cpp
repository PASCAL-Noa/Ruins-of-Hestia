#include "Samples/SceneUiTest.h"

#include "Random.h"
#include "UI/Raycastable.h"
#include "UI/UiAnim.h"
#include "UI/UiAnimFluent.h"
#include "UI/UiAnimTimeline.h"
#include "UI/UiBuilder.h"
#include "UI/UiButton.h"
#include "UI/UiCanvas.h"
#include "UI/UiLayout.h"
#include "UI/UiSprite.h"
#include "UI/UiSystem.h"
#include "UI/UiText.h"
#include "UI/UiTheme.h"
#include "UiConstants.h"

namespace GPC
{
    ErrorType SceneUiTest::OnAssetsLoad()
    {
        SceneDefault::OnAssetsLoad();

        GPC_ASSETS->AddTexture("Ui_Pickaxe", "Resources/Images/pickaxe.png");
        GPC_ASSETS->AddTexture("Ui_Black",   "Resources/Images/Black.png");

        GPC_ASSETS->AddTextureArray(
            "16x16_ui",
            16, 16,
            {
                GPC_TEXTURE("Ui_Pickaxe"),
                GPC_TEXTURE("Ui_Black"),
            }
        );

        return ErrorType::SUCCESS;
    }

    UiCanvas* SceneUiTest::BuildStyledPanel(UiCanvas* parent, Anchor anchor, glm::vec2 offset,
                                            glm::vec2 size, Color bg, Color border)
    {
        UiBuilder b(this);
        const EntityID pid = parent->GetEntityID();

        b.ChildOf(pid)
         .At(anchor, offset + glm::vec2{ 2.0f, 2.0f })
         .Size(size)
         .Bg(Color(0, 0, 0, 70))
         .BuildCanvas();

        b.ChildOf(pid)
         .At(anchor, offset - glm::vec2{ 1.0f, 1.0f })
         .Size(size + glm::vec2{ 2.0f, 2.0f })
         .Bg(border)
         .BuildCanvas();

        return b.ChildOf(pid)
                .At(anchor, offset)
                .Size(size)
                .Bg(bg)
                .BuildCanvas();
    }

    UiCanvas* SceneUiTest::BuildButton(UiCanvas* parent, const std::string& label)
    {
        UiBuilder b(this);

        UiCanvas* btn = b.ChildOf(parent->GetEntityID())
                         .Size({ 220.0f, 42.0f })
                         .Bg(UiTheme::ButtonPrimary)
                         .BuildCanvas();
        if (!btn) return nullptr;

        b.ChildOf(btn->GetEntityID())
         .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
         .Font(UiTheme::DefaultFont, 16.0f)
         .Tint(Colors::BLACK)
         .Text(label)
         .BuildText();

        return btn;
    }

    UiCanvas* SceneUiTest::BuildSmallButton(UiCanvas* parent, const std::string& label)
    {
        UiBuilder b(this);

        UiCanvas* btn = b.ChildOf(parent->GetEntityID())
                         .Size({ 110.0f, 36.0f })
                         .Bg(UiTheme::ButtonPrimary)
                         .FlexGrow(1.0f)
                         .BuildCanvas();
        if (!btn) return nullptr;

        b.ChildOf(btn->GetEntityID())
         .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
         .Font(UiTheme::DefaultFont, 13.0f)
         .Tint(Colors::BLACK)
         .Text(label)
         .BuildText();

        return btn;
    }

    ErrorType SceneUiTest::OnCreate(SceneInformation& info)
    {
        SceneDefault::OnCreate(info);

        CreateDebugOverlay();

        UiBuilder b(this);

        mp_Root = b.At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
                   .Size({ kBaseResolutionWidth, kBaseResolutionHeight })
                   .FitToScreen(true)
                   .Bg(Color(30, 34, 42, 255))
                   .BuildCanvas();
        if (!mp_Root) return ErrorType::SUCCESS;

        UiCanvas* header = BuildStyledPanel(
            mp_Root, Anchors::TOP_MIDDLE, { 0.0f, 8.0f },
            { 1040.0f, 66.0f },
            Color(35, 42, 58, 255),
            Color(215, 168, 60, 255));

        if (header)
        {
            b.ChildOf(header->GetEntityID())
             .At(Anchors::MIDDLE_MIDDLE, { 0.0f, -10.0f })
             .Font(UiTheme::DefaultFont, 26.0f)
             .Tint(UiTheme::AccentGold)
             .Text("UiAnim Playground")
             .BuildText();

            b.ChildOf(header->GetEntityID())
             .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 16.0f })
             .Font(UiTheme::DefaultFont, 13.0f)
             .Tint(UiTheme::BodyText)
             .Text("Interactive showcase of the UI animation system")
             .BuildText();
        }

        UiCanvas* centerZone = BuildStyledPanel(
            mp_Root, Anchors::MIDDLE_MIDDLE, { 0.0f, -60.0f },
            { 500.0f, 300.0f },
            Color(42, 48, 62, 235),
            Color(80, 180, 200, 255));

        if (centerZone)
        {
            b.ChildOf(centerZone->GetEntityID())
             .At(Anchors::TOP_MIDDLE, { 0.0f, 10.0f })
             .Font(UiTheme::DefaultFont, 13.0f)
             .Tint(UiTheme::AccentCyan)
             .Text("TARGET ZONE")
             .BuildText();

            mp_Target = b.ChildOf(centerZone->GetEntityID())
                         .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
                         .Sprite("16x16_ui", "Ui_Pickaxe")
                         .Scale(6.0f)
                         .Tint(UiTheme::AccentCyan)
                         .BuildSprite();

            mp_StateLabel = b.ChildOf(centerZone->GetEntityID())
                             .At(Anchors::BOTTOM_MIDDLE, { 0.0f, -15.0f })
                             .Font(UiTheme::DefaultFont, 16.0f)
                             .Tint(UiTheme::AccentGold)
                             .Text("IDLE")
                             .BuildText();
            if (mp_StateLabel) mp_StateLabel->BindReactive(m_StateText);

            BuildExtraTargets(centerZone);
        }

        BuildDemoButtons(mp_Root);
        BuildAdvancedButtons(mp_Root);

        UiCanvas* footer = BuildStyledPanel(
            mp_Root, Anchors::BOTTOM_MIDDLE, { 0.0f, -4.0f },
            { 1040.0f, 56.0f },
            Color(25, 30, 42, 255),
            Color(155, 55, 180, 255));

        if (footer)
        {
            mp_HelpLabel = b.ChildOf(footer->GetEntityID())
                            .At(Anchors::MIDDLE_MIDDLE, { 0.0f, -8.0f })
                            .Font(UiTheme::DefaultFont, 14.0f)
                            .Tint(UiTheme::BodyText)
                            .Text("Click a button to launch an animation")
                            .BuildText();
            if (mp_HelpLabel) mp_HelpLabel->BindReactive(m_HelpText);

            mp_StatusLabel = b.ChildOf(footer->GetEntityID())
                              .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 14.0f })
                              .Font(UiTheme::DefaultFont, 12.0f)
                              .Tint(UiTheme::DimmedText)
                              .Text("ready")
                              .BuildText();
            if (mp_StatusLabel) mp_StatusLabel->BindReactive(m_Status);
        }

        m_Status.Set("ready");
        m_HelpText.Set("Click a button to launch an animation");
        return ErrorType::SUCCESS;
    }

    void SceneUiTest::BuildExtraTargets(UiCanvas* parent)
    {
        UiBuilder b(this);

        const std::array<glm::vec2, 4> positions = {{
            { -180.0f, -70.0f },
            {  180.0f, -70.0f },
            { -180.0f,  70.0f },
            {  180.0f,  70.0f }
        }};

        const std::array<Color, 4> colors = {{
            Color(220, 60, 80, 255),
            Color(240, 170, 50, 255),
            Color(170, 90, 220, 255),
            Color(80, 200, 180, 255)
        }};

        for (int i = 0; i < 4; ++i)
        {
            mp_ExtraTargets[i] = b.ChildOf(parent->GetEntityID())
                                  .At(Anchors::MIDDLE_MIDDLE, positions[i])
                                  .Sprite("16x16_ui", "Ui_Pickaxe")
                                  .Scale(2.5f)
                                  .Tint(colors[i])
                                  .BuildSprite();
        }
    }

    void SceneUiTest::BuildDemoButtons(UiCanvas* root)
    {
        UiBuilder b(this);

        const Color greenAccent  = Color(30, 150, 100, 255);
        const Color purpleAccent = Color(155, 55, 180, 255);

        UiCanvas* leftHeader = BuildStyledPanel(
            root, Anchors::TOP_LEFT, { 24.0f, 92.0f },
            { 228.0f, 30.0f },
            greenAccent,
            Color(60, 200, 140, 255));
        if (leftHeader)
        {
            b.ChildOf(leftHeader->GetEntityID())
             .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
             .Font(UiTheme::DefaultFont, 13.0f)
             .Tint(Colors::BLACK)
             .Text("SIMPLE ANIMATIONS")
             .BuildText();
        }

        UiCanvas* leftCol = BuildStyledPanel(
            root, Anchors::TOP_LEFT, { 24.0f, 128.0f },
            { 228.0f, 440.0f },
            Color(34, 40, 54, 240),
            greenAccent);
        if (leftCol)
        {
            leftCol->SetFitToScreen(false);
            b.ChildOf(leftCol->GetEntityID());
        }
        {
            auto* layout = AddComponent<UiLayout>(leftCol->GetEntityID());
            if (layout)
            {
                layout->Mode    = UiLayoutMode::Column;
                layout->Gap     = 5.0f;
                layout->Padding = { 8.0f, 8.0f, 8.0f, 8.0f };
                layout->IsDirty = true;
                if (auto* sys = GetSystem<UiSystem>()) sys->RegisterLayoutRoot(leftCol->GetEntityID());
            }
        }

        UiCanvas* rightHeader = BuildStyledPanel(
            root, Anchors::TOP_RIGHT, { -24.0f, 92.0f },
            { 228.0f, 30.0f },
            purpleAccent,
            Color(200, 100, 220, 255));
        if (rightHeader)
        {
            b.ChildOf(rightHeader->GetEntityID())
             .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
             .Font(UiTheme::DefaultFont, 13.0f)
             .Tint(Colors::BLACK)
             .Text("COMPOSITION")
             .BuildText();
        }

        UiCanvas* rightCol = BuildStyledPanel(
            root, Anchors::TOP_RIGHT, { -24.0f, 128.0f },
            { 228.0f, 440.0f },
            Color(34, 40, 54, 240),
            purpleAccent);
        {
            auto* layout = AddComponent<UiLayout>(rightCol->GetEntityID());
            if (layout)
            {
                layout->Mode    = UiLayoutMode::Column;
                layout->Gap     = 5.0f;
                layout->Padding = { 8.0f, 8.0f, 8.0f, 8.0f };
                layout->IsDirty = true;
                if (auto* sys = GetSystem<UiSystem>()) sys->RegisterLayoutRoot(rightCol->GetEntityID());
            }
        }

        auto hook = [this](UiCanvas* btn, const std::string& name, std::function<void()> action)
        {
            if (!btn) return;
            UiBuilder bb(this);
            UiButton* ub = bb.BuildButton(btn);
            if (!ub) return;
            ub->OnClick.ConnectLambda([this, name, action = std::move(action)]()
            {
                m_Status.Set(name);
                m_HelpText.Set(name);
                action();
            });
        };

        hook(BuildButton(leftCol, "Fade In"), "FadeIn 0.5s", [this]()
        {
            UiAnim::Cancel(*this, mp_Target);
            UiAnim::FadeIn(*this, mp_Target, 0.5f);
        });

        hook(BuildButton(leftCol, "Fade Out"), "FadeOut 0.5s", [this]()
        {
            UiAnim::Cancel(*this, mp_Target);
            UiAnim::FadeOut(*this, mp_Target, 0.5f);
        });

        hook(BuildButton(leftCol, "Fade To 0.3"), "FadeTo 0.3", [this]()
        {
            UiAnim::Cancel(*this, mp_Target);
            UiAnim::FadeTo(*this, mp_Target, 0.3f, 0.4f);
        });

        hook(BuildButton(leftCol, "Scale Pulse"), "ScalePulse 1.5x", [this]()
        {
            UiAnim::Cancel(*this, mp_Target);
            UiAnim::ScalePulse(*this, mp_Target, 1.5f, 0.6f);
        });

        hook(BuildButton(leftCol, "Scale To 10x"), "ScaleTo 10x", [this]()
        {
            UiAnim::Cancel(*this, mp_Target);
            UiAnim::ScaleTo(*this, mp_Target, { 10.0f, 10.0f }, 0.5f);
        });

        hook(BuildButton(leftCol, "Move Random"), "MoveTo random", [this]()
        {
            UiAnim::Cancel(*this, mp_Target);
            const float x = Random::Float(-300.0f, 300.0f);
            const float y = Random::Float(-200.0f, 200.0f);
            UiAnim::MoveTo(*this, mp_Target, { x, y }, 0.6f);
        });

        hook(BuildButton(leftCol, "Slide From Left"), "SlideInX -400", [this]()
        {
            UiAnim::Cancel(*this, mp_Target);
            UiAnim::SlideInX(*this, mp_Target, -400.0f, 0.8f);
        });

        hook(BuildButton(leftCol, "Tint Red"), "TintTo red", [this]()
        {
            UiAnim::Cancel(*this, mp_Target);
            UiAnim::TintTo(*this, mp_Target, Colors::CRIMSON, 0.4f);
        });

        hook(BuildButton(leftCol, "Tint Cyan"), "TintTo cyan", [this]()
        {
            UiAnim::Cancel(*this, mp_Target);
            UiAnim::TintTo(*this, mp_Target, UiTheme::AccentCyan, 0.4f);
        });

        hook(BuildButton(rightCol, "Chain Fade Move Pulse"), "Chain demo", [this]()
        {
            UiAnim::Cancel(*this, mp_Target);
            UiAnim::On(*this, mp_Target)
                .FadeOut(0.3f)
                .Then().MoveTo({ 200.0f, -150.0f }, 0.5f)
                .Then().FadeIn(0.3f)
                .Then().ScalePulse(1.8f, 0.5f)
                .OnComplete([this](){ m_Status.Set("chain complete"); })
                .Play();
        });

        hook(BuildButton(rightCol, "Parallel Fade Scale Tint"), "Parallel demo", [this]()
        {
            UiAnim::Cancel(*this, mp_Target);
            UiAnim::OnParallel(*this, mp_Target)
                .FadeIn(0.6f)
                .With().ScaleTo({ 8.0f, 8.0f }, 0.6f)
                .With().TintTo(Colors::AMETHYST, 0.6f)
                .OnComplete([this](){ m_Status.Set("parallel complete"); })
                .Play();
        });

        hook(BuildButton(rightCol, "Delayed fade out"), "Delay demo", [this]()
        {
            UiAnim::Cancel(*this, mp_Target);
            UiAnim::On(*this, mp_Target)
                .After(0.5f).FadeOut(0.4f)
                .Then().FadeIn(0.4f)
                .Play();
        });

        hook(BuildButton(rightCol, "Combo crazy"), "Crazy demo", [this]()
        {
            UiAnim::Cancel(*this, mp_Target);
            UiAnim::On(*this, mp_Target)
                .MoveTo({ -300.0f, 0.0f }, 0.3f)
                .Then().MoveTo({ 300.0f, 0.0f }, 0.3f)
                .Then().MoveTo({ 0.0f, -200.0f }, 0.3f)
                .Then().MoveTo({ 0.0f, 200.0f }, 0.3f)
                .Then().MoveTo({ 0.0f, 0.0f }, 0.3f)
                .Then().ScalePulse(2.0f, 0.5f)
                .OnComplete([this](){ m_Status.Set("combo done"); })
                .Play();
        });

        hook(BuildButton(rightCol, "Cancel All"), "Cancel", [this]()
        {
            UiAnim::Cancel(*this, mp_Target);
        });

        hook(BuildButton(rightCol, "Reset Target"), "Reset", [this]()
        {
            ResetTarget();
        });

        hook(BuildButton(rightCol, "Is Animating"), "Query", [this]()
        {
            const bool animating = UiAnim::IsAnimating(*this, mp_Target);
            m_Status.Set(animating ? "YES - animating" : "NO - idle");
        });

        hook(BuildButton(rightCol, "Cycle Easing"), "Cycle easing", [this]()
        {
            struct EasingEntry { Tweening::EasingType type; const char* name; };
            static const std::array<EasingEntry, 6> kEasings = {{
                { Tweening::EasingType::Linear,         "Linear (constant)" },
                { Tweening::EasingType::EaseInExpo,     "EaseInExpo (slow start, fires)" },
                { Tweening::EasingType::EaseOutExpo,    "EaseOutExpo (fires, slow stop)" },
                { Tweening::EasingType::EaseOutBack,    "EaseOutBack (overshoots)" },
                { Tweening::EasingType::EaseOutElastic, "EaseOutElastic (springy)" },
                { Tweening::EasingType::EaseOutBounce,  "EaseOutBounce (bouncy ball)" }
            }};

            const auto& entry = kEasings[m_EasingIndex % kEasings.size()];
            m_EasingIndex = (m_EasingIndex + 1) % kEasings.size();

            UiAnim::Cancel(*this, mp_Target);
            if (auto* tr = GetComponent<Transform2D>(mp_Target->GetEntityID()))
            {
                tr->LocalTransform.SetPosition({ -320.0f, 0.0f, 0.0f });
            }

            UiAnim::MoveTo(*this, mp_Target, { 320.0f, 0.0f }, 1.5f, entry.type);

            m_Status.Set(std::string("Easing: ") + entry.name);
        });

        hook(BuildButton(rightCol, "Multi-target dance"), "Multi-target dance", [this]()
        {
            ResetExtraTargets();

            for (int i = 0; i < 4; ++i)
            {
                auto* t = mp_ExtraTargets[i];
                if (!t) continue;

                const float delay = static_cast<float>(i) * 0.15f;

                UiAnim::On(*this, t)
                    .After(delay)
                    .FadeIn(0.3f)
                    .With().ScalePulse(1.8f, 0.6f)
                    .Then().MoveTo({ 0.0f, 0.0f }, 0.5f)
                    .Then().After(0.2f).MoveTo(
                        { i == 0 || i == 2 ? -180.0f : 180.0f,
                          i < 2 ? -120.0f : 120.0f }, 0.5f)
                    .Play();
            }
        });

        hook(BuildButton(rightCol, "Reset Extras"), "Reset extras", [this]()
        {
            ResetExtraTargets();
        });
    }

    void SceneUiTest::BuildAdvancedButtons(UiCanvas* root)
    {
        UiBuilder b(this);

        const Color blueAccent = Color(40, 130, 220, 255);

        UiCanvas* header = BuildStyledPanel(
            root, Anchors::BOTTOM_MIDDLE, { 0.0f, -160.0f },
            { 900.0f, 28.0f },
            blueAccent,
            Color(70, 170, 255, 255));
        if (header)
        {
            b.ChildOf(header->GetEntityID())
             .At(Anchors::MIDDLE_MIDDLE, { 0.0f, 0.0f })
             .Font(UiTheme::DefaultFont, 13.0f)
             .Tint(Colors::BLACK)
             .Text("PLAYBACK CONTROL")
             .BuildText();
        }

        UiCanvas* bar = BuildStyledPanel(
            root, Anchors::BOTTOM_MIDDLE, { 0.0f, -110.0f },
            { 900.0f, 50.0f },
            Color(34, 40, 54, 240),
            blueAccent);
        if (!bar) return;

        {
            auto* layout = AddComponent<UiLayout>(bar->GetEntityID());
            if (layout)
            {
                layout->Mode    = UiLayoutMode::Row;
                layout->Gap     = 5.0f;
                layout->Padding = { 6.0f, 6.0f, 6.0f, 6.0f };
                layout->IsDirty = true;
                if (auto* sys = GetSystem<UiSystem>()) sys->RegisterLayoutRoot(bar->GetEntityID());
            }
        }

        auto hook = [this](UiCanvas* btn, const std::string& status, const std::string& help, std::function<void()> action)
        {
            if (!btn) return;
            UiBuilder bb(this);
            UiButton* ub = bb.BuildButton(btn);
            if (!ub) return;
            ub->OnClick.ConnectLambda([this, status, help, action = std::move(action)]()
            {
                m_Status.Set(status);
                m_HelpText.Set(help);
                action();
            });
        };

        hook(BuildSmallButton(bar, "OnStart"),
             "OnStart wait 1s",
             "Target idle 1s, then OnStart fires (turns RED), then moves up 1s. Watch status change.",
             [this]()
        {
            ResetTarget();
            UiAnim::On(*this, mp_Target)
                .After(1.0f)
                .MoveTo({ 0.0f, -120.0f }, 1.0f)
                .OnStart([this]()
                {
                    if (mp_Target) mp_Target->SetTint(Colors::CRIMSON);
                    m_HelpText.Set("OnStart fired! Target turned RED and is moving up");
                })
                .OnComplete([this]()
                {
                    if (mp_Target) mp_Target->SetTint(UiTheme::AccentCyan);
                    m_HelpText.Set("OnComplete fired. Target back to cyan.");
                })
                .Play();
        });

        hook(BuildSmallButton(bar, "OnUpdate"),
             "OnUpdate tracks t",
             "Target moves right over 2s. OnUpdate(t) tints it live from CYAN to RED based on progress",
             [this]()
        {
            ResetTarget();
            UiAnim::On(*this, mp_Target)
                .MoveTo({ 320.0f, 0.0f }, 2.0f)
                .OnUpdate([this](float t)
                {
                    if (!mp_Target) return;
                    const glm::vec4 cyan = static_cast<glm::vec4>(UiTheme::AccentCyan);
                    const glm::vec4 red  = static_cast<glm::vec4>(Colors::CRIMSON);
                    mp_Target->SetTint(Color(cyan + (red - cyan) * t));
                })
                .Play();
        });

        hook(BuildSmallButton(bar, "Launch long anim"),
             "Long anim started",
             "10s aller-retour. Now click Pause / Resume to control it.",
             [this]()
        {
            ResetTarget();
            UiAnim::On(*this, mp_Target)
                .MoveTo({ -300.0f, 0.0f }, 3.0f)
                .Then().MoveTo({ 300.0f, 0.0f }, 3.0f)
                .Then().MoveTo({ 0.0f, 0.0f }, 3.0f)
                .Play();

            int slots = 0;
            const EntityID eid = mp_Target ? mp_Target->GetEntityID() : 0;
            if (mp_Target && HasComponent<UiAnimComponent>(eid))
            {
                if (auto* c = GetComponent<UiAnimComponent>(eid))
                {
                    slots = static_cast<int>(c->Active.size());
                }
            }
            char buf[128];
            std::snprintf(buf, sizeof(buf),
                          "Launched on eid %u   pushed %d slot",
                          static_cast<uint32_t>(eid), slots);
            m_HelpText.Set(buf);
        });

        hook(BuildSmallButton(bar, "Pause"),
             "Paused",
             "Target frozen. Click Resume to continue.",
             [this]()
        {
            if (!mp_Target) return;
            const EntityID eid = mp_Target->GetEntityID();
            const bool hasComp = HasComponent<UiAnimComponent>(eid);

            bool before = false;
            int  slots  = 0;
            if (hasComp)
            {
                if (auto* c = GetComponent<UiAnimComponent>(eid))
                {
                    before = c->PlaybackActive;
                    slots  = static_cast<int>(c->Active.size());
                }
            }

            UiAnim::Pause(*this, mp_Target);

            bool after = true;
            if (hasComp)
            {
                if (auto* c = GetComponent<UiAnimComponent>(eid))
                {
                    after = c->PlaybackActive;
                }
            }

            mp_Target->SetTint(UiTheme::DimmedText);

            char buf[160];
            std::snprintf(buf, sizeof(buf),
                          "Pause  eid %u  hasUAC %s  PA %s then %s  slots %d",
                          static_cast<uint32_t>(eid),
                          hasComp ? "yes" : "NO",
                          before ? "true" : "false",
                          after  ? "true" : "false",
                          slots);
            m_HelpText.Set(buf);
        });

        hook(BuildSmallButton(bar, "Resume"),
             "Resumed",
             "Target back to CYAN. Anim continues where it was.",
             [this]()
        {
            if (!mp_Target) return;
            const EntityID eid = mp_Target->GetEntityID();

            bool before = true;
            if (auto* c = GetComponent<UiAnimComponent>(eid))
            {
                before = c->PlaybackActive;
            }

            UiAnim::Resume(*this, mp_Target);

            bool after = false;
            int  slots = 0;
            if (auto* c = GetComponent<UiAnimComponent>(eid))
            {
                after = c->PlaybackActive;
                slots = static_cast<int>(c->Active.size());
            }

            mp_Target->SetTint(UiTheme::AccentCyan);

            char buf[128];
            std::snprintf(buf, sizeof(buf),
                          "Resume  PA %s then %s  slots %d",
                          before ? "true" : "false",
                          after  ? "true" : "false",
                          slots);
            m_HelpText.Set(buf);
        });

        hook(BuildSmallButton(bar, "Rate slow"),
             "Slow-mo",
             "WithRate(0.25) : same animation but plays 4x SLOWER. MoveTo duration 1s -> takes 4s to finish",
             [this]()
        {
            ResetTarget();
            UiAnim::On(*this, mp_Target)
                .MoveTo({ 300.0f, 0.0f }, 1.0f).WithRate(0.25f)
                .Play();
        });

        hook(BuildSmallButton(bar, "Rate fast"),
             "Fast-forward",
             "WithRate(3.0) : same animation but plays 3x FASTER. MoveTo duration 3s -> takes 1s to finish",
             [this]()
        {
            ResetTarget();
            UiAnim::On(*this, mp_Target)
                .MoveTo({ 300.0f, 0.0f }, 3.0f).WithRate(3.0f)
                .Play();
        });

        hook(BuildSmallButton(bar, "Infinite Loop"),
             "Loop started tag loop_halo",
             "ScalePulse Repeat minus1 Tag loop_halo  pulses FOREVER until you click Cancel loop.",
             [this]()
        {
            ResetTarget();
            UiAnim::On(*this, mp_Target)
                .ScalePulse(1.4f, 0.6f)
                .Repeat(-1)
                .Tag("loop_halo")
                .Play();

            int slots = 0;
            const EntityID eid = mp_Target ? mp_Target->GetEntityID() : 0;
            if (mp_Target && HasComponent<UiAnimComponent>(eid))
            {
                if (auto* c = GetComponent<UiAnimComponent>(eid))
                {
                    slots = static_cast<int>(c->Active.size());
                }
            }
            char buf[160];
            std::snprintf(buf, sizeof(buf),
                          "Loop pushed on eid %u   pushed %d slot. Now click Cancel loop.",
                          static_cast<uint32_t>(eid), slots);
            m_HelpText.Set(buf);
        });

        hook(BuildSmallButton(bar, "PingPong"),
             "PingPong started",
             "MoveTo.PingPong(5) : moves right, then left, right, left... 5 additional cycles (6 total direction changes)",
             [this]()
        {
            ResetTarget();
            UiAnim::On(*this, mp_Target)
                .MoveTo({ 250.0f, 0.0f }, 0.6f)
                .PingPong(5)
                .Play();
        });

        hook(BuildSmallButton(bar, "Cancel loop"),
             "Cancel tag loop_halo",
             "Removes only the slots tagged loop_halo.",
             [this]()
        {
            if (!mp_Target) return;
            const EntityID eid = mp_Target->GetEntityID();
            const bool hasComp = HasComponent<UiAnimComponent>(eid);

            int slotsBefore = 0;
            if (hasComp)
            {
                if (auto* c = GetComponent<UiAnimComponent>(eid))
                {
                    slotsBefore = static_cast<int>(c->Active.size());
                }
            }

            const size_t removed = UiAnim::CancelTag(*this, mp_Target, "loop_halo");

            int slotsAfter = 0;
            if (hasComp)
            {
                if (auto* c = GetComponent<UiAnimComponent>(eid))
                {
                    slotsAfter = static_cast<int>(c->Active.size());
                }
            }

            mp_Target->SetScale({ 6.0f, 6.0f });

            char buf[192];
            std::snprintf(buf, sizeof(buf),
                          "CancelTag loop_halo  eid %u  hasUAC %s  removed %zu  slots %d then %d",
                          static_cast<uint32_t>(eid),
                          hasComp ? "yes" : "NO",
                          removed, slotsBefore, slotsAfter);
            m_HelpText.Set(buf);
        });

        hook(BuildSmallButton(bar, "Timeline"),
             "Timeline playing",
             "Timeline.At(t, elt).XXX : each entry is scheduled at an absolute timestamp. Watch the 4 extras pulse in sequence at 0.3/0.6/0.9/1.2s",
             [this]()
        {
            ResetTarget();
            ResetExtraTargets();

            UiAnim::Timeline tl;
            tl.At(0.0f, mp_Target, *this).FadeIn(0.3f);
            tl.At(0.3f, mp_ExtraTargets[0], *this).ScalePulse(1.5f, 0.4f);
            tl.At(0.6f, mp_ExtraTargets[1], *this).ScalePulse(1.5f, 0.4f);
            tl.At(0.9f, mp_ExtraTargets[2], *this).ScalePulse(1.5f, 0.4f);
            tl.At(1.2f, mp_ExtraTargets[3], *this).ScalePulse(1.5f, 0.4f);
            tl.At(1.6f, mp_Target, *this).MoveTo({ 0.0f, -200.0f }, 0.5f);
            tl.At(2.2f, mp_Target, *this).MoveTo({ 0.0f, 0.0f }, 0.5f);
            tl.Play();
        });
    }

    void SceneUiTest::ResetExtraTargets()
    {
        const std::array<glm::vec2, 4> positions = {{
            { -180.0f, -70.0f },
            {  180.0f, -70.0f },
            { -180.0f,  70.0f },
            {  180.0f,  70.0f }
        }};

        for (int i = 0; i < 4; ++i)
        {
            auto* t = mp_ExtraTargets[i];
            if (!t) continue;
            UiAnim::Cancel(*this, t);
            t->SetScale({ 2.5f, 2.5f });
            if (auto* tr = GetComponent<Transform2D>(t->GetEntityID()))
            {
                tr->LocalTransform.SetPosition({ positions[i].x, positions[i].y, 0.0f });
            }
        }
    }

    void SceneUiTest::ResetTarget()
    {
        if (!mp_Target) return;
        UiAnim::Cancel(*this, mp_Target);
        UiAnim::SetRate(*this, mp_Target, 1.0f);
        UiAnim::Resume(*this, mp_Target);
        mp_Target->SetTint(UiTheme::AccentCyan);
        mp_Target->SetScale({ 6.0f, 6.0f });
        if (auto* tr = GetComponent<Transform2D>(mp_Target->GetEntityID()))
        {
            tr->LocalTransform.SetPosition({ 0.0f, 0.0f, 0.0f });
        }
        m_Status.Set("reset");
    }

    void SceneUiTest::OnSceneUpdate()
    {
        SceneDefault::OnSceneUpdate();

        if (Inputs::IsKeyPress(Inputs::KeyCode::R))
        {
            ResetTarget();
        }

        if (mp_Target)
        {
            const EntityID eid = mp_Target->GetEntityID();
            int slotCount = 0;
            bool paused = false;
            if (HasComponent<UiAnimComponent>(eid))
            {
                if (auto* comp = GetComponent<UiAnimComponent>(eid))
                {
                    slotCount = static_cast<int>(comp->Active.size());
                    paused    = !comp->PlaybackActive;
                }
            }

            char buf[64];
            if (slotCount == 0)        std::snprintf(buf, sizeof(buf), "IDLE");
            else if (paused)           std::snprintf(buf, sizeof(buf), "PAUSED  [%d slot]", slotCount);
            else                       std::snprintf(buf, sizeof(buf), "ANIMATING  [%d slot]", slotCount);

            if (std::string(buf) != m_StateText.Get())
            {
                m_StateText.Set(buf);
            }
        }
    }

    void SceneUiTest::OnDestroy()
    {
    }
}
