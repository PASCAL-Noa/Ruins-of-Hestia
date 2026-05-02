#pragma once

#include <string>

#include "Material.h"
#include "TextureArray.h"
#include "UI/UiElement.h"
#include "UI/UiReactive.h"
#include "UI/UiSignal.h"

namespace GPC
{
    struct Texture;

    struct UiSprite : public UiElement
    {
        BIND_COMPONENT(UI_SPRITE);
        AUTO_UI_COMPONENT_CONSTRUCTOR(UiSprite);

        virtual ~UiSprite() = default;

        void CalcElementSize() override;
        void SetSprite(const std::string& atlas, const std::string& name);
        void SetSprite(GPC::TextureArray* pArray);

        void BindTintReactive(UiReactive<Color>& source);

    private:
        const Texture*  m_Sprite        = nullptr;
        Material        m_Material      = {};

        UiSignalHandle      m_BoundTintHandle = 0;
        UiReactive<Color>*  mp_BoundTintSource = nullptr;

        friend class UiSystem;
    };
}
