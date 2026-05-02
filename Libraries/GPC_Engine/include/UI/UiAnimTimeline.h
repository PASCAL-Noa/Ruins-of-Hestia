#pragma once

#include <functional>
#include <vector>

#include <glm/glm.hpp>

#include "Color.h"
#include "Scene.h"
#include "Tweens.h"
#include "UI/UiAnimComponent.h"
#include "UI/UiElement.h"

namespace GPC::UiAnim
{
    class Timeline
    {
    public:
        template<typename UiT>
        class Entry
        {
        public:
            Entry(Timeline* owner, Scene* scene, UiT* elt, float timestamp)
                : mp_Owner(owner), mp_Scene(scene), mp_Elt(elt), m_Timestamp(timestamp)
            {
                if (mp_Elt) m_Eid = mp_Elt->GetEntityID();
            }

            Entry& FadeIn(float duration, Tweening::EasingType ease = Tweening::EasingType::EaseOutQuad)
            {
                return Push(duration, ease, [scene = mp_Scene, eid = m_Eid](float t)
                {
                    if (!scene->template HasComponent<UiT>(eid)) return;
                    auto* e = scene->template GetComponent<UiT>(eid);
                    if (e) e->SetTint(Color(glm::vec4(1.0f, 1.0f, 1.0f, t)));
                });
            }

            Entry& FadeOut(float duration, Tweening::EasingType ease = Tweening::EasingType::EaseInQuad)
            {
                return Push(duration, ease, [scene = mp_Scene, eid = m_Eid](float t)
                {
                    if (!scene->template HasComponent<UiT>(eid)) return;
                    auto* e = scene->template GetComponent<UiT>(eid);
                    if (e) e->SetTint(Color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f - t)));
                });
            }

            Entry& ScaleTo(glm::vec2 target, float duration, Tweening::EasingType ease = Tweening::EasingType::EaseOutQuad)
            {
                const glm::vec2 start = mp_Elt ? mp_Elt->GetScale() : glm::vec2(1.0f);
                return Push(duration, ease, [scene = mp_Scene, eid = m_Eid, start, target](float t)
                {
                    if (!scene->template HasComponent<UiT>(eid)) return;
                    auto* e = scene->template GetComponent<UiT>(eid);
                    if (e) e->SetScale(start + (target - start) * t);
                });
            }

            Entry& MoveTo(glm::vec2 destination, float duration, Tweening::EasingType ease = Tweening::EasingType::EaseOutCubic)
            {
                auto* t2d = mp_Scene->template GetComponent<Transform2D>(m_Eid);
                const glm::vec3 startPos = t2d ? t2d->LocalTransform.GetPosition() : glm::vec3(0.0f);
                const glm::vec3 endPos   = { destination.x, destination.y, startPos.z };

                return Push(duration, ease, [scene = mp_Scene, eid = m_Eid, startPos, endPos](float t)
                {
                    if (!scene->template HasComponent<Transform2D>(eid)) return;
                    auto* tr = scene->template GetComponent<Transform2D>(eid);
                    if (tr) tr->LocalTransform.SetPosition(startPos + (endPos - startPos) * t);
                });
            }

            Entry& TintTo(Color target, float duration, Tweening::EasingType ease = Tweening::EasingType::EaseOutQuad)
            {
                const Color start = mp_Elt ? mp_Elt->GetTint() : Colors::WHITE;
                return Push(duration, ease, [scene = mp_Scene, eid = m_Eid, start, target](float t)
                {
                    if (!scene->template HasComponent<UiT>(eid)) return;
                    auto* e = scene->template GetComponent<UiT>(eid);
                    if (e) e->SetTint(Color::Lerp(start, target, t));
                });
            }

            Entry& ScalePulse(float peak, float duration, Tweening::EasingType ease = Tweening::EasingType::EaseOutBack,
                              float startMul = 1.0f, float endMul = 1.0f)
            {
                const glm::vec2 start = mp_Elt ? mp_Elt->GetScale() : glm::vec2(1.0f);
                return Push(duration, ease, [scene = mp_Scene, eid = m_Eid, start, peak, startMul, endMul](float t)
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

        private:
            template<typename Fn>
            Entry& Push(float duration, Tweening::EasingType ease, Fn&& setter)
            {
                UiAnimSlot slot{};
                slot.Duration = duration;
                slot.Ease     = ease;
                slot.Delay    = m_Timestamp;
                slot.Setter   = std::forward<Fn>(setter);
                mp_Owner->m_Pending.push_back({ m_Eid, std::move(slot) });
                return *this;
            }

            Timeline* mp_Owner   = nullptr;
            Scene*    mp_Scene   = nullptr;
            UiT*      mp_Elt     = nullptr;
            EntityID  m_Eid      = 0;
            float     m_Timestamp = 0.0f;
        };

        template<typename UiT>
        Entry<UiT> At(float timestamp, UiT* elt, Scene& scene)
        {
            mp_Scene = &scene;
            return Entry<UiT>(this, &scene, elt, timestamp);
        }

        void Play()
        {
            if (!mp_Scene) return;
            for (auto& entry : m_Pending)
            {
                auto* comp = EnsureComponent(*mp_Scene, entry.Eid);
                if (!comp) continue;
                comp->Active.push_back(std::move(entry.Slot));
            }
            m_Pending.clear();
        }

    private:
        struct PendingEntry
        {
            EntityID     Eid;
            UiAnimSlot   Slot;
        };

        Scene* mp_Scene = nullptr;
        std::vector<PendingEntry> m_Pending;

        template<typename U> friend class Entry;
    };
}
