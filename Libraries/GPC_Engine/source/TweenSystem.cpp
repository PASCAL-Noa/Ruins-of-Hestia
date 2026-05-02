#include "TweenSystem.h"
#include "Scene.h"
#include "Clock.h"

namespace GPC
{

    void TweenSystem::OnUpdate()
    {
        System::OnUpdate();
        if (!m_Ctx.pScene || !m_Ctx.pClock)
            return;

        auto block = m_Ctx.pScene->GetComponentBlock<TweenComponent>();
        if (!block)
            return;

        float dt = static_cast<float>(m_Ctx.pClock->GetDeltaTime());
        uint64_t count = block->GetEntityCount();
        TweenComponent* components = block->ComponentData();

        for (uint64_t i = 0; i < count; ++i)
        {
            TweenComponent& tweenComp = components[i];

            if (!tweenComp.IsEnable)
                continue;

            std::erase_if(tweenComp.ActiveTweens, [dt](const std::shared_ptr<ITween>& tween)
            {
                tween->Update(dt);
                return tween->IsFinished();
            });
        }
    }
}