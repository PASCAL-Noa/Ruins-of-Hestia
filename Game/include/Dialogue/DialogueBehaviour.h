#pragma once

#include <array>
#include <fstream>
#include <vector>

#include "Behavior.h"
#include "UI/UiReactive.h"
#include "UiConstants.h"
#include "UI/UiButton.h"

namespace GPC
{
    struct UiCanvas;
    struct UiSprite;
    struct UiText;

    struct ParsedDialogue
    {
        std::vector<std::string> Speakers;
        std::string Text;
    };

    struct DialogueBehaviour : public Behavior
    {
    public:
        INHERIT_BEHAVIOR_CONSTRUCTOR(DialogueBehaviour);

        void OnCreate(const BehaviorCreateContext* pCtx) override;
        void OnUpdate(const BehaviorUpdateContext* pCtx) override;

        void StartDialogue(const std::string& path, const std::vector<UiCanvas*>& hideExisting, bool autoClose = true);
        [[nodiscard]] bool IsDialogueEnded() const { return m_IsDialogueEnded; }

        [[nodiscard]] uint32_t GetCurrentDialogueIndex() const { return m_CurrentDialogue; }
        [[nodiscard]] uint32_t GetParsedDialoguesCount() const { return static_cast<uint32_t>(m_ParsedDialogues.size()); }

        void StopDialog();

        void SetVisibility(bool active);
        
        UiButton* mp_DialogButton = nullptr;
    private:
        std::vector<ParsedDialogue> ParseDialogueFile(const std::string& path);
        void LoadParsedDialogue(bool resetPage);
        void ApplySpeakerVisual(const std::vector<std::string>& speakers) const;

        void BuildUi(const BehaviorCreateContext* pCtx);
        void PaginateText(uint32_t offset, uint32_t size, bool resetPage);
        void ShowCurrentPage();
        void NextPage();

        void OnDestroy() override;

    private:
        UiCanvas* mp_RootCanvas = nullptr;
        UiSprite* mp_DialogBoxSprite = nullptr;
        UiSprite* mp_SpeakerSprite = nullptr;
        UiSprite* mp_Speaker2Sprite = nullptr;
        UiSprite* mp_SpeakerNameSprite = nullptr;
        UiText* mp_SpeakerNameLabel = nullptr;
        std::array<UiText*, kDialogueMaxLinesPerPage> mp_LineLabels{};

        Scene* mp_Scene = nullptr;

        UiReactive<std::string> m_SpeakerName{ "" };
        std::array<UiReactive<std::string>, kDialogueMaxLinesPerPage> m_LineReactives{
            UiReactive<std::string>{""},
            UiReactive<std::string>{""},
            UiReactive<std::string>{""},
            UiReactive<std::string>{""}
        };

        std::vector<char> m_FileBuffer;
        uint32_t m_FileLength = 0;

        std::vector<std::array<std::string, kDialogueMaxLinesPerPage>> m_Pages;
        uint32_t m_CurrentPage = 0;

        std::vector<ParsedDialogue> m_ParsedDialogues;
        uint32_t m_CurrentDialogue = 0;

        std::vector<UiCanvas*> m_HiddenOnStart;
        bool m_IsDialogueEnded = true;

        bool m_IsAutoClose = true;

        int m_DialogueMaxLettersPerLine = 70;
        int m_BaseDialogueMaxLettersPerLine = m_DialogueMaxLettersPerLine;

        float m_LastScaleX = 1.f;
    };
}
