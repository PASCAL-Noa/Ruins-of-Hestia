#pragma once

#include <glm/glm.hpp>

#include "Color.h"
#include "Scene.h"
#include "Tweens.h"
#include "TransformComponents.h"
#include "UI/UiAnimComponent.h"
#include "UI/UiElement.h"

namespace GPC::UiAnim
{
    inline UiAnimComponent* EnsureComponent(Scene& scene, EntityID eid)
    {
        if (scene.HasComponent<UiAnimComponent>(eid))
        {
            return scene.GetComponent<UiAnimComponent>(eid);
        }
        return scene.AddComponent<UiAnimComponent>(eid);
    }

    inline void PushAnim(Scene& scene, EntityID eid, UiAnimSlot&& slot)
    {
        auto* comp = EnsureComponent(scene, eid);
        if (!comp) return;
        comp->Active.push_back(std::move(slot));
    }

    template<typename UiT>
    inline void FadeIn(Scene& scene, UiT* elt, float duration,
                       Tweening::EasingType ease = Tweening::EasingType::EaseOutQuad)
    {
        if (!elt) return;
        const EntityID eid = elt->GetEntityID();
        Scene* pScene = &scene;

        elt->SetTint(Color(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));

        UiAnimSlot slot{};
        slot.Duration = duration;
        slot.Ease     = ease;
        slot.Setter   = [pScene, eid](float t)
        {
            if (!pScene->HasComponent<UiT>(eid)) return;
            auto* e = pScene->GetComponent<UiT>(eid);
            if (e) e->SetTint(Color(glm::vec4(1.0f, 1.0f, 1.0f, t)));
        };
        PushAnim(scene, eid, std::move(slot));
    }

    template<typename UiT>
    inline void FadeOut(Scene& scene, UiT* elt, float duration,
                        Tweening::EasingType ease = Tweening::EasingType::EaseInQuad)
    {
        if (!elt) return;
        const EntityID eid = elt->GetEntityID();
        Scene* pScene = &scene;

        UiAnimSlot slot{};
        slot.Duration = duration;
        slot.Ease     = ease;
        slot.Setter   = [pScene, eid](float t)
        {
            if (!pScene->HasComponent<UiT>(eid)) return;
            auto* e = pScene->GetComponent<UiT>(eid);
            if (e) e->SetTint(Color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f - t)));
        };
        PushAnim(scene, eid, std::move(slot));
    }

    template<typename UiT>
    inline void FadeTo(Scene& scene, UiT* elt, float targetAlpha, float duration,
                       Tweening::EasingType ease = Tweening::EasingType::EaseOutQuad)
    {
        if (!elt) return;
        const EntityID eid = elt->GetEntityID();
        Scene* pScene = &scene;
        const float start = static_cast<glm::vec4>(elt->GetTint()).a;

        UiAnimSlot slot{};
        slot.Duration = duration;
        slot.Ease     = ease;
        slot.Setter   = [pScene, eid, start, targetAlpha](float t)
        {
            if (!pScene->HasComponent<UiT>(eid)) return;
            auto* e = pScene->GetComponent<UiT>(eid);
            if (e)
            {
                const float a = start + (targetAlpha - start) * t;
                e->SetTint(Color(glm::vec4(1.0f, 1.0f, 1.0f, a)));
            }
        };
        PushAnim(scene, eid, std::move(slot));
    }

    template<typename UiT>
    inline void ScaleTo(Scene& scene, UiT* elt, glm::vec2 target, float duration,
                        Tweening::EasingType ease = Tweening::EasingType::EaseOutQuad)
    {
        if (!elt) return;
        const EntityID eid = elt->GetEntityID();
        Scene* pScene = &scene;
        const glm::vec2 start = elt->GetScale();

        UiAnimSlot slot{};
        slot.Duration = duration;
        slot.Ease     = ease;
        slot.Setter   = [pScene, eid, start, target](float t)
        {
            if (!pScene->HasComponent<UiT>(eid)) return;
            auto* e = pScene->GetComponent<UiT>(eid);
            if (e) e->SetScale(start + (target - start) * t);
        };
        PushAnim(scene, eid, std::move(slot));
    }

    template<typename UiT>
    inline void ScalePulse(Scene& scene, UiT* elt, float peak, float duration,
                           Tweening::EasingType ease = Tweening::EasingType::EaseOutBack,
                           float startScale = 1.0f, float endScale = 1.0f)
    {
        if (!elt) return;
        const EntityID eid = elt->GetEntityID();
        Scene* pScene = &scene;

        UiAnimSlot up{};
        up.Duration = duration * 0.5f;
        up.Ease     = ease;
        up.Setter   = [pScene, eid, startScale, peak](float t)
        {
            if (!pScene->HasComponent<UiT>(eid)) return;
            auto* e = pScene->GetComponent<UiT>(eid);
            if (!e) return;
            const float s = startScale + (peak - startScale) * t;
            e->SetScale({ s, s });
        };
        PushAnim(scene, eid, std::move(up));

        UiAnimSlot down{};
        down.Duration = duration * 0.5f;
        down.Delay    = duration * 0.5f;
        down.Ease     = ease;
        down.Setter   = [pScene, eid, peak, endScale](float t)
        {
            if (!pScene->HasComponent<UiT>(eid)) return;
            auto* e = pScene->GetComponent<UiT>(eid);
            if (!e) return;
            const float s = peak + (endScale - peak) * t;
            e->SetScale({ s, s });
        };
        PushAnim(scene, eid, std::move(down));
    }

    template<typename UiT>
    inline void MoveTo(Scene& scene, UiT* elt, glm::vec2 destination, float duration,
                       Tweening::EasingType ease = Tweening::EasingType::EaseOutCubic)
    {
        if (!elt) return;
        const EntityID eid = elt->GetEntityID();
        Scene* pScene = &scene;

        auto* t2d = scene.GetComponent<Transform2D>(eid);
        if (!t2d) return;

        const glm::vec3 startPos = t2d->LocalTransform.GetPosition();
        const glm::vec3 endPos   = { destination.x, destination.y, startPos.z };

        UiAnimSlot slot{};
        slot.Duration = duration;
        slot.Ease     = ease;
        slot.Setter   = [pScene, eid, startPos, endPos](float t)
        {
            if (!pScene->HasComponent<Transform2D>(eid)) return;
            auto* tr = pScene->GetComponent<Transform2D>(eid);
            if (tr) tr->LocalTransform.SetPosition(startPos + (endPos - startPos) * t);
        };
        PushAnim(scene, eid, std::move(slot));
    }

    template<typename UiT>
    inline void SlideTo(Scene& scene, UiT* elt, glm::vec2 destination, float duration,
                        Tweening::EasingType ease = Tweening::EasingType::EaseOutCubic)
    {
        MoveTo(scene, elt, destination, duration, ease);
    }

    template<typename UiT>
    inline void SlideInX(Scene& scene, UiT* elt, float fromDeltaX, float duration,
                         Tweening::EasingType ease = Tweening::EasingType::EaseOutCubic)
    {
        if (!elt) return;
        const EntityID eid = elt->GetEntityID();
        Scene* pScene = &scene;

        auto* t2d = scene.GetComponent<Transform2D>(eid);
        if (!t2d) return;

        const glm::vec3 basePos = t2d->LocalTransform.GetPosition();
        t2d->LocalTransform.SetPosition({ basePos.x + fromDeltaX, basePos.y, basePos.z });

        UiAnimSlot slot{};
        slot.Duration = duration;
        slot.Ease     = ease;
        slot.Setter   = [pScene, eid, basePos, fromDeltaX](float t)
        {
            if (!pScene->HasComponent<Transform2D>(eid)) return;
            auto* tr = pScene->GetComponent<Transform2D>(eid);
            if (tr)
            {
                const float x = basePos.x + fromDeltaX * (1.0f - t);
                tr->LocalTransform.SetPosition({ x, basePos.y, basePos.z });
            }
        };
        PushAnim(scene, eid, std::move(slot));
    }

    template<typename UiT>
    inline void TintTo(Scene& scene, UiT* elt, Color target, float duration,
                       Tweening::EasingType ease = Tweening::EasingType::EaseOutQuad)
    {
        if (!elt) return;
        const EntityID eid = elt->GetEntityID();
        Scene* pScene = &scene;
        const Color start = elt->GetTint();

        UiAnimSlot slot{};
        slot.Duration = duration;
        slot.Ease     = ease;
        slot.Setter   = [pScene, eid, start, target](float t)
        {
            if (!pScene->HasComponent<UiT>(eid)) return;
            auto* e = pScene->GetComponent<UiT>(eid);
            if (e) e->SetTint(Color::Lerp(start, target, t));
        };
        PushAnim(scene, eid, std::move(slot));
    }

    inline void Cancel(Scene& scene, EntityID eid)
    {
        if (!scene.HasComponent<UiAnimComponent>(eid)) return;
        auto* comp = scene.GetComponent<UiAnimComponent>(eid);
        if (comp) comp->Active.clear();
    }

    template<typename UiT>
    inline void Cancel(Scene& scene, UiT* elt)
    {
        if (elt) Cancel(scene, elt->GetEntityID());
    }

    inline size_t CancelTag(Scene& scene, EntityID eid, uint32_t tagHash)
    {
        if (!scene.HasComponent<UiAnimComponent>(eid)) return 0;
        auto* comp = scene.GetComponent<UiAnimComponent>(eid);
        if (!comp) return 0;
        return std::erase_if(comp->Active, [tagHash](const UiAnimSlot& s){ return s.TagHash == tagHash; });
    }

    template<typename UiT>
    inline size_t CancelTag(Scene& scene, UiT* elt, uint32_t tagHash)
    {
        return elt ? CancelTag(scene, elt->GetEntityID(), tagHash) : 0;
    }

    template<typename UiT>
    inline size_t CancelTag(Scene& scene, UiT* elt, const char* tag)
    {
        return elt ? CancelTag(scene, elt->GetEntityID(), UiAnimHashTag(tag)) : 0;
    }

    inline bool IsAnimating(Scene& scene, EntityID eid)
    {
        if (!scene.HasComponent<UiAnimComponent>(eid)) return false;
        auto* comp = scene.GetComponent<UiAnimComponent>(eid);
        return comp && !comp->Active.empty();
    }

    template<typename UiT>
    inline bool IsAnimating(Scene& scene, UiT* elt)
    {
        return elt ? IsAnimating(scene, elt->GetEntityID()) : false;
    }

    inline void Pause(Scene& scene, EntityID eid)
    {
        if (!scene.HasComponent<UiAnimComponent>(eid)) return;
        auto* comp = scene.GetComponent<UiAnimComponent>(eid);
        if (comp) comp->PlaybackActive = false;
    }

    inline void Resume(Scene& scene, EntityID eid)
    {
        if (!scene.HasComponent<UiAnimComponent>(eid)) return;
        auto* comp = scene.GetComponent<UiAnimComponent>(eid);
        if (comp) comp->PlaybackActive = true;
    }

    inline bool IsPaused(Scene& scene, EntityID eid)
    {
        if (!scene.HasComponent<UiAnimComponent>(eid)) return false;
        auto* comp = scene.GetComponent<UiAnimComponent>(eid);
        return comp && !comp->PlaybackActive;
    }

    inline void SetRate(Scene& scene, EntityID eid, float rate)
    {
        if (!scene.HasComponent<UiAnimComponent>(eid)) return;
        auto* comp = scene.GetComponent<UiAnimComponent>(eid);
        if (comp) comp->PlaybackRate = rate;
    }

    template<typename UiT>
    inline void Pause(Scene& scene, UiT* elt)    { if (elt) Pause(scene, elt->GetEntityID()); }
    template<typename UiT>
    inline void Resume(Scene& scene, UiT* elt)   { if (elt) Resume(scene, elt->GetEntityID()); }
    template<typename UiT>
    inline bool IsPaused(Scene& scene, UiT* elt) { return elt ? IsPaused(scene, elt->GetEntityID()) : false; }
    template<typename UiT>
    inline void SetRate(Scene& scene, UiT* elt, float rate) { if (elt) SetRate(scene, elt->GetEntityID(), rate); }
}
