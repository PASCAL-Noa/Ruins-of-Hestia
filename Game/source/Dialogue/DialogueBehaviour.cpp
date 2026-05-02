#include "Dialogue/DialogueBehaviour.h"

#include <regex>
#include <sstream>

#include "Debugger.h"
#include "RenderWindow.h"
#include "Scene.h"
#include "UI/UiBuilder.h"
#include "UI/UiCanvas.h"
#include "UI/UiSprite.h"
#include "UI/UiSystem.h"
#include "UI/UiText.h"
#include "UI/UiTheme.h"

namespace GPC
{
    void DialogueBehaviour::OnCreate(const BehaviorCreateContext* pCtx)
    {
        Behavior::OnCreate(pCtx);
        mp_Scene = pCtx->pScene;
        BuildUi(pCtx);
    }

    void DialogueBehaviour::OnUpdate(const BehaviorUpdateContext* pCtx)
    {

        Behavior::OnUpdate(pCtx);
        if (!pCtx || !pCtx->pScene || !mp_DialogBoxSprite) return;

        auto* sys = pCtx->pScene->GetSystem<UiSystem>();
        const float globalRatio = sys ? sys->GetGlobalRatio() : 1.0f;
        if (globalRatio <= 0.0f) return;

        const auto windowSize = glm::vec2(pCtx->pScene->GetRenderWindow()->GetSize());
        const float targetWidth = windowSize.x - 2.0f * kDialogueBeginPosX * globalRatio;
        const float scaleX = targetWidth > 0.0f ? targetWidth / (kDialogueBoxAtlasWidth * globalRatio) : 0.1f;
        if (m_LastScaleX != scaleX)
        {
            m_LastScaleX = scaleX;
            mp_DialogBoxSprite->SetScale({ scaleX, 0.5f });

            m_DialogueMaxLettersPerLine = m_BaseDialogueMaxLettersPerLine * globalRatio / 1.1f;

            LoadParsedDialogue(false);
        }

    }

    std::string NormalizeSpecialCharacters(const std::string& input)
    {
        std::string out;
        out.reserve(input.size());

        for (size_t i = 0; i < input.size(); )
        {
            unsigned char c = static_cast<unsigned char>(input[i]);

            if (c == 0xE2u && i + 2 < input.size())
            {
                unsigned char b1 = static_cast<unsigned char>(input[i + 1]);
                unsigned char b2 = static_cast<unsigned char>(input[i + 2]);

                if (b1 == 0x80u && b2 == 0xA6u)
                {
                    out += "...";
                    i += 3;
                    continue;
                }
            }

            out += static_cast<char>(c);
            ++i;
        }

        return out;
    }


    void DialogueBehaviour::StopDialog()
    {
        if (mp_RootCanvas)
            mp_RootCanvas->IsEnable = false;

        for (auto* c : m_HiddenOnStart)
        {
            if (c) c->IsEnable = true;
        }

        m_IsDialogueEnded = true;
    }

    void DialogueBehaviour::SetVisibility(bool active) {
        mp_RootCanvas->IsEnable = active;
        mp_DialogBoxSprite->IsEnable = active;
        mp_SpeakerSprite->IsEnable = active;
        mp_Speaker2Sprite->IsEnable = active;
        mp_SpeakerNameSprite->IsEnable = active;
        mp_SpeakerNameLabel->IsEnable = active;
        mp_DialogButton->SetEnabled(active);

        for (auto jsp : mp_LineLabels) {
            jsp->IsEnable = active;
        }
    }

    std::vector<ParsedDialogue> DialogueBehaviour::ParseDialogueFile(const std::string& path)
    {
        std::ifstream file(path);
        std::vector<ParsedDialogue> result;

        if (!file.is_open())
        {
            //GPC_ERROR << "Failed to open dialogue file: " << path << ENDL;
            result.clear();
            return result;
        }

        std::string line;
        ParsedDialogue current;
        bool isFirstLine = true;

        auto trimRight = [](std::string& s)
        {
            while (!s.empty())
            {
                const unsigned char c = static_cast<unsigned char>(s.back());
                if (c == '\r' || c == '\n' || c == '\t' || c == ' ')
                    s.pop_back();
                else
                    break;
            }
        };

        auto trimLeft = [](std::string& s)
        {
            while (!s.empty())
            {
                const unsigned char c = static_cast<unsigned char>(s.front());
                if (c == '\t' || c == ' ')
                {
                    s.erase(s.begin());
                    continue;
                }

                // Strip UTF-8 BOM if present
                if (s.size() >= 3 &&
                    static_cast<unsigned char>(s[0]) == 0xEFu &&
                    static_cast<unsigned char>(s[1]) == 0xBBu &&
                    static_cast<unsigned char>(s[2]) == 0xBFu)
                {
                    s.erase(0, 3);
                }
                break;
            }
        };

        while (std::getline(file, line))
        {
            trimRight(line);
            if (isFirstLine)
            {
                trimLeft(line);
                isFirstLine = false;
            }

            if (line.empty())
                continue;

            if (line.front() == '[' && line.back() == ']')
            {
                if (!current.Speakers.empty() && !current.Text.empty())
                {
                    result.push_back(current);
                    current = {};
                }

                std::string raw = line.substr(1, line.size() - 2);

                std::stringstream ss(raw);
                std::string token;

                current.Speakers.clear();

                while (std::getline(ss, token, '|'))
                {
                    current.Speakers.push_back(token);
                }
            }
            else
            {
                if (!current.Text.empty())
                    current.Text += " ";

                current.Text += line;
            }
        }

        if (!current.Speakers.empty() && !current.Text.empty())
        {
            result.push_back(current);
        }

        return result;
    }

    void DialogueBehaviour::ApplySpeakerVisual(const std::vector<std::string>& speakers) const
    {
        if (!mp_SpeakerSprite || !mp_Speaker2Sprite) return;

        mp_SpeakerSprite->SetTint(Colors::TRANSPARENT);
        mp_Speaker2Sprite->SetTint(Colors::TRANSPARENT);

        for (size_t i = 0; i < speakers.size(); ++i)
        {
            std::string s = speakers[i];
            std::ranges::transform(s, s.begin(), ::toupper);

            UiSprite* target = (i == 0) ? mp_SpeakerSprite : mp_Speaker2Sprite;
            if (!target) continue;

            target->SetTint(i == 0 ? Colors::WHITE : Colors::GRAY);

            if (s == "CALI")
            {
                target->SetSprite("317x642", "Cali_Face");
                target->SetScale({ 0.5f, 0.5f });
            }
            else if (s == "IRIS")
            {
                target->SetSprite("303x642", "Iris_Face");
                target->SetScale({ 0.5f, 0.55f });
            }
            else if (s == "SILENE")
            {
                target->SetSprite("212x642", "Silene_Face");
                target->SetScale({ 0.7f, 0.7f });
            }
            else if (s == "SILENE?")
            {
                target->SetTint(Colors::TRANSPARENT);
                mp_SpeakerNameLabel->SetText("SILENE");
            }
            else
            {
                target->SetTint(Colors::TRANSPARENT);
            }
        }
    }

    void DialogueBehaviour::BuildUi(const BehaviorCreateContext* pCtx)
    {
        if (!pCtx || !pCtx->pScene) return;

        UiBuilder b(pCtx->pScene);

        mp_RootCanvas = b.At(Anchors::TOP_LEFT, { 0.0f, 0.0f })
            .Tint(Colors::TRANSPARENT)
            .FitToScreen(true)
            .BuildCanvas();
        if (!mp_RootCanvas) return;

        mp_SpeakerSprite = b.ChildOf(mp_RootCanvas->GetEntityID())
                            .At(Anchors::BOTTOM_LEFT, { kDialogueMarginX + 100, -kDialogueMarginY })
                            .Scale({ 0.5f, 0.5f })
                            .Sprite("1411x418", "Bg_Dialogue")
                            .Tint(Colors::TRANSPARENT)
                            .BuildSprite();


        mp_Speaker2Sprite = b.ChildOf(mp_RootCanvas->GetEntityID())
                            .At(Anchors::BOTTOM_RIGHT, { -kDialogueMarginX - 100, -kDialogueMarginY })
                            .Scale({ 0.7f, 0.7f })
                            .Sprite("1411x418", "Bg_Dialogue")
                            .Tint(Colors::TRANSPARENT)
                            .BuildSprite();

        mp_DialogBoxSprite = b.ChildOf(mp_RootCanvas->GetEntityID())
                            .At(Anchors::BOTTOM_LEFT, { kDialogueBeginPosX, -kDialogueMarginY })
                            .Scale({ 1.0f, 0.5f })
                            .Sprite("1411x418", "Bg_Dialogue")
                            .BuildSprite();

        mp_SpeakerNameSprite = b.ChildOf(mp_RootCanvas->GetEntityID())
                            .At({0.67f, 0.65f} )
                            .Scale({ 0.5f, 0.5f })
                            .Sprite("283x156", "Bg_Dialogue_Name")
                            .BuildSprite();

        if (mp_DialogBoxSprite)
        {
            mp_DialogButton = b.BuildButton(mp_RootCanvas);
            if (mp_DialogButton)
            {
                mp_DialogButton->OnClick.ConnectLambda([this]() { NextPage(); });
            }

            mp_SpeakerNameLabel = b.ChildOf(mp_SpeakerNameSprite->GetEntityID())
                                  .At(Anchors::MIDDLE_MIDDLE, { 0.f, 0.f })
                                  .Font(UiTheme::DefaultFont, 24.0f)
                                  .Tint(UiTheme::DarkText)
                                  .Text("")
                                  .BuildText();
            if (mp_SpeakerNameLabel) mp_SpeakerNameLabel->BindReactive(m_SpeakerName);

            for (int i = 0; i < kDialogueMaxLinesPerPage; ++i)
            {
                mp_LineLabels[i] = b.ChildOf(mp_DialogBoxSprite->GetEntityID())
                                   .At(Anchors::TOP_LEFT, { 50.0f, kDialogueLineStartY + static_cast<float>(i) * kDialogueLineOffset })
                                   .Font(UiTheme::DefaultFont, 24.0f)
                                   .Tint(UiTheme::DarkText)
                                   .Text("")
                                   .BuildText();
                if (mp_LineLabels[i]) mp_LineLabels[i]->BindReactive(m_LineReactives[i]);
            }
        }

        if (mp_RootCanvas)
            mp_RootCanvas->IsEnable = false;
    }

    void DialogueBehaviour::PaginateText(uint32_t offset, const uint32_t size, bool resetPage)
    {
        m_Pages.clear();
        if (resetPage)
            m_CurrentPage = 0;

        const uint32_t capped = offset + size > m_FileLength ? m_FileLength - offset : size;

        std::vector<char> clean;
        clean.reserve(capped);
        for (uint32_t i = 0; i < capped; ++i)
        {
            const char c = m_FileBuffer[offset + i];
            if (c == '\n' || c == '\r' || c == '\t')
                continue;
            //if (c < 0)
            //    continue;
            clean.push_back(c);
        }
        clean.push_back('\0');

        std::stringstream ss(clean.data());
        std::string word;
        std::vector<std::string> words;
        while (ss >> word) words.push_back(word);

        std::array<std::string, kDialogueMaxLinesPerPage> current{};
        int line = 0;

        auto flushPage = [&]()
        {
            m_Pages.push_back(current);
            current = {};
            line = 0;
        };

        auto advanceLine = [&]()
        {
            ++line;
            if (line >= kDialogueMaxLinesPerPage)
                flushPage();
        };

        for (const auto& w : words)
        {
            if (current[line].empty())
            {
                current[line] = w;
                if (static_cast<int>(w.size()) > m_DialogueMaxLettersPerLine)
                    advanceLine();
            }
            else if (static_cast<int>(current[line].size() + 1 + w.size()) <= m_DialogueMaxLettersPerLine)
            {
                current[line] += " " + w;
            }
            else
            {
                advanceLine();
                current[line] = w;
                if (static_cast<int>(w.size()) > m_DialogueMaxLettersPerLine)
                    advanceLine();
            }
        }

        m_Pages.push_back(current);
        if (m_CurrentPage >= m_Pages.size())
            m_CurrentPage = static_cast<uint32_t>(m_Pages.size()) - 1;
        ShowCurrentPage();
    }

    void DialogueBehaviour::ShowCurrentPage()
    {
        if (m_CurrentPage >= m_Pages.size()) return;
        const auto& page = m_Pages[m_CurrentPage];
        for (int i = 0; i < kDialogueMaxLinesPerPage; ++i)
        {
            m_LineReactives[i].Set(page[i]);
        }
    }

    void DialogueBehaviour::NextPage()
    {
        if (m_CurrentPage + 1 < m_Pages.size())
        {
            ++m_CurrentPage;
            ShowCurrentPage();
            return;
        }

        ++m_CurrentDialogue;
        LoadParsedDialogue(true);
    }

    void DialogueBehaviour::OnDestroy()
    {
        Behavior::OnDestroy();

        // mp_Scene->DestroyEntity(mp_RootCanvas->GetEntityID());
        // mp_Scene->DestroyEntity(mp_DialogBoxSprite->GetEntityID());
        // mp_Scene->DestroyEntity(mp_SpeakerSprite->GetEntityID());
        // mp_Scene->DestroyEntity(mp_Speaker2Sprite->GetEntityID());
        // mp_Scene->DestroyEntity(mp_SpeakerNameLabel->GetEntityID());

        mp_DialogButton->SetDisabled(true);

        // mp_Scene->RemoveBehavior<UiButton>(mp_DialogButton->GetEntityID());
        // mp_Scene->DestroyEntity(mp_DialogButton->GetEntityID());

        mp_DialogButton    = nullptr;
        mp_RootCanvas      = nullptr;
        mp_DialogBoxSprite = nullptr;
        mp_SpeakerSprite   = nullptr;
        mp_Speaker2Sprite  = nullptr;
        mp_SpeakerNameLabel = nullptr;
        for (auto* it : mp_LineLabels)
        {
            //mp_Scene->DestroyEntity(it->GetEntityID());
            it = nullptr;
        }
    }


    void DialogueBehaviour::StartDialogue(const std::string& path, const std::vector<UiCanvas*>& hideExisting, const bool autoClose)
    {
        SetVisibility(true);
        for (auto* c : hideExisting)
        {
            if (c) c->IsEnable = false;
        }
        m_HiddenOnStart = hideExisting;

        m_IsAutoClose = autoClose;

        m_ParsedDialogues = ParseDialogueFile(path);
        //GPC_ERROR << "DialogueBehaviour::StartDialogue path=" << path << " parsed=" << m_ParsedDialogues.size() << ENDL;

        if (m_ParsedDialogues.empty())
        {
            mp_RootCanvas->IsEnable = false;
            return;
        }

        m_CurrentDialogue = 0;
        m_IsDialogueEnded = false;

        if (mp_RootCanvas)
            mp_RootCanvas->IsEnable = true;

        mp_DialogButton->SetDisabled(false);

        LoadParsedDialogue(true);
    }

    void DialogueBehaviour::LoadParsedDialogue(bool resetPage)
    {
        if (m_CurrentDialogue >= m_ParsedDialogues.size())
        {
            mp_DialogButton->SetDisabled(true);

            if (m_IsAutoClose)
                StopDialog();

            return;
        }

        const auto& d = m_ParsedDialogues[m_CurrentDialogue];
        //GPC_ERROR << "DialogueBehaviour::LoadParsedDialogue idx=" << m_CurrentDialogue << " speakers=" << d.Speakers.size() << " textLen=" << d.Text.size() << ENDL;

        if (!d.Speakers.empty())
        {
            m_SpeakerName.Set(d.Speakers[0]);
        }

        ApplySpeakerVisual(d.Speakers);

        std::string normalized  = NormalizeSpecialCharacters(d.Text);

        m_FileBuffer.assign(normalized.begin(), normalized.end());
        m_FileBuffer.push_back('\0');
        m_FileLength = static_cast<uint32_t>(normalized.size());

        PaginateText(0, m_FileLength, resetPage);

    }
}
