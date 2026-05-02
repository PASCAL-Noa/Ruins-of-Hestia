#pragma once

#include <functional>
#include <vector>

#include <glm/glm.hpp>

#include "Color.h"
#include "Scene.h"
#include "Tweens.h"
#include "UI/UiAnim.h"
#include "UI/UiAnimComponent.h"

namespace GPC::UiAnim
{
    enum class ChainMode : uint8_t
    {
        Sequential = 0,
        Parallel   = 1
    };

    template<typename UiT>
    class Builder
    {
    public:
        Builder(Scene* pScene, UiT* elt, ChainMode mode)
            : mp_Scene(pScene), mp_Elt(elt), m_Mode(mode)
        {
            if (mp_Elt) m_EntityID = mp_Elt->GetEntityID();
        }

        Builder& After(float delay)
        {
            m_PendingDelay += delay;
            return *this;
        }

        Builder& Then()
        {
            m_Mode = ChainMode::Sequential;
            return *this;
        }

        Builder& With()
        {
            m_Mode = ChainMode::Parallel;
            return *this;
        }

        Builder& FadeIn(float duration, Tweening::EasingType ease = Tweening::EasingType::EaseOutQuad)
        {
            return AddStep(duration, ease, [scene = mp_Scene, eid = m_EntityID](float t)
            {
                if (!scene->template HasComponent<UiT>(eid)) return;
                auto* e = scene->template GetComponent<UiT>(eid);
                if (e) e->SetTint(Color(glm::vec4(1.0f, 1.0f, 1.0f, t)));
            });
        }

        Builder& FadeOut(float duration, Tweening::EasingType ease = Tweening::EasingType::EaseInQuad)
        {
            return AddStep(duration, ease, [scene = mp_Scene, eid = m_EntityID](float t)
            {
                if (!scene->template HasComponent<UiT>(eid)) return;
                auto* e = scene->template GetComponent<UiT>(eid);
                if (e) e->SetTint(Color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f - t)));
            });
        }

        Builder& FadeTo(float targetAlpha, float duration, Tweening::EasingType ease = Tweening::EasingType::EaseOutQuad)
        {
            const float start = mp_Elt ? static_cast<glm::vec4>(mp_Elt->GetTint()).a : 1.0f;
            return AddStep(duration, ease, [scene = mp_Scene, eid = m_EntityID, start, targetAlpha](float t)
            {
                if (!scene->template HasComponent<UiT>(eid)) return;
                auto* e = scene->template GetComponent<UiT>(eid);
                if (e)
                {
                    const float a = start + (targetAlpha - start) * t;
                    e->SetTint(Color(glm::vec4(1.0f, 1.0f, 1.0f, a)));
                }
            });
        }

        Builder& ScaleTo(glm::vec2 target, float duration, Tweening::EasingType ease = Tweening::EasingType::EaseOutQuad)
        {
            const glm::vec2 start = mp_Elt ? mp_Elt->GetScale() : glm::vec2(1.0f);
            return AddStep(duration, ease, [scene = mp_Scene, eid = m_EntityID, start, target](float t)
            {
                if (!scene->template HasComponent<UiT>(eid)) return;
                auto* e = scene->template GetComponent<UiT>(eid);
                if (e) e->SetScale(start + (target - start) * t);
            });
        }

        Builder& ScalePulse(float peak, float duration, Tweening::EasingType ease = Tweening::EasingType::EaseOutBack,
                            float startMul = 1.0f, float endMul = 1.0f)
        {
            const glm::vec2 start = mp_Elt ? mp_Elt->GetScale() : glm::vec2(1.0f);
            return AddStep(duration, ease, [scene = mp_Scene, eid = m_EntityID, start, peak, startMul, endMul](float t)
            {
                if (!scene->template HasComponent<UiT>(eid)) return;
                auto* e = scene->template GetComponent<UiT>(eid);
                if (!e) return;
                const float factor = (t < 0.5f)
                    ? (startMul + (peak - startMul) * (t * 2.0f))
                    : (peak + (endMul - peak) * ((t - 0.5f) * 2.0f));
                e->SetScale(start * factor);
            });
        }

        Builder& MoveTo(glm::vec2 destination, float duration, Tweening::EasingType ease = Tweening::EasingType::EaseOutCubic)
        {
            auto* t2d = mp_Scene->template GetComponent<Transform2D>(m_EntityID);
            const glm::vec3 startPos = t2d ? t2d->LocalTransform.GetPosition() : glm::vec3(0.0f);
            const glm::vec3 endPos   = { destination.x, destination.y, startPos.z };

            return AddStep(duration, ease, [scene = mp_Scene, eid = m_EntityID, startPos, endPos](float t)
            {
                if (!scene->template HasComponent<Transform2D>(eid)) return;
                auto* tr = scene->template GetComponent<Transform2D>(eid);
                if (tr) tr->LocalTransform.SetPosition(startPos + (endPos - startPos) * t);
            });
        }

        Builder& TintTo(Color target, float duration, Tweening::EasingType ease = Tweening::EasingType::EaseOutQuad)
        {
            const Color start = mp_Elt ? mp_Elt->GetTint() : Colors::WHITE;
            return AddStep(duration, ease, [scene = mp_Scene, eid = m_EntityID, start, target](float t)
            {
                if (!scene->template HasComponent<UiT>(eid)) return;
                auto* e = scene->template GetComponent<UiT>(eid);
                if (e) e->SetTint(Color::Lerp(start, target, t));
            });
        }

        Builder& OnComplete(std::function<void()> cb)
        {
            if (!m_PendingSlots.empty()) m_PendingSlots.back().OnComplete = std::move(cb);
            return *this;
        }

        Builder& OnStart(std::function<void()> cb)
        {
            if (!m_PendingSlots.empty()) m_PendingSlots.back().OnStart = std::move(cb);
            return *this;
        }

        Builder& OnUpdate(std::function<void(float)> cb)
        {
            if (!m_PendingSlots.empty()) m_PendingSlots.back().OnUpdate = std::move(cb);
            return *this;
        }

        Builder& Tag(const char* tag)
        {
            if (!m_PendingSlots.empty()) m_PendingSlots.back().TagHash = UiAnimHashTag(tag);
            return *this;
        }

        Builder& WithRate(float rate)
        {
            if (!m_PendingSlots.empty()) m_PendingSlots.back().PlaybackRate = rate;
            return *this;
        }

        Builder& Repeat(int32_t count = -1)
        {
            if (!m_PendingSlots.empty())
            {
                m_PendingSlots.back().Loop        = UiAnimLoop::Restart;
                m_PendingSlots.back().RepeatCount = count;
            }
            return *this;
        }

        Builder& PingPong(int32_t count = -1)
        {
            if (!m_PendingSlots.empty())
            {
                m_PendingSlots.back().Loop        = UiAnimLoop::PingPong;
                m_PendingSlots.back().RepeatCount = count;
            }
            return *this;
        }

        void Play()
        {
            if (!mp_Scene) return;
            auto* comp = EnsureComponent(*mp_Scene, m_EntityID);
            if (!comp) return;

            for (auto& slot : m_PendingSlots)
            {
                comp->Active.push_back(std::move(slot));
            }
            m_PendingSlots.clear();
        }

    private:
        template<typename Fn>
        Builder& AddStep(float duration, Tweening::EasingType ease, Fn&& setter)
        {
            UiAnimSlot slot{};
            slot.Duration = duration;
            slot.Ease     = ease;
            slot.Setter   = std::forward<Fn>(setter);

            const float baseTime = (m_Mode == ChainMode::Sequential)
                                 ? m_AccumulatedEnd
                                 : m_LastStart;
            slot.Delay    = baseTime + m_PendingDelay;

            m_LastStart      = slot.Delay;
            const float myEnd = slot.Delay + duration;
            if (myEnd > m_AccumulatedEnd) m_AccumulatedEnd = myEnd;

            m_PendingDelay = 0.0f;
            m_PendingSlots.push_back(std::move(slot));
            return *this;
        }

        Scene*                    mp_Scene          = nullptr;
        UiT*                      mp_Elt            = nullptr;
        EntityID                  m_EntityID        = 0;
        ChainMode                 m_Mode            = ChainMode::Sequential;
        float                     m_AccumulatedEnd  = 0.0f;
        float                     m_LastStart       = 0.0f;
        float                     m_PendingDelay    = 0.0f;
        std::vector<UiAnimSlot>   m_PendingSlots;
    };

    template<typename UiT>
    inline Builder<UiT> On(Scene& scene, UiT* elt)
    {
        return Builder<UiT>(&scene, elt, ChainMode::Sequential);
    }

    template<typename UiT>
    inline Builder<UiT> OnParallel(Scene& scene, UiT* elt)
    {
        return Builder<UiT>(&scene, elt, ChainMode::Parallel);
    }
}
