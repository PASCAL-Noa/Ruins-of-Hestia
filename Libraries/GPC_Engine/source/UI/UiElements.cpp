#include "UI/UiCanvas.h"
#include "UI/UiText.h"
#include "UI/UiSprite.h"
#include "UI/UiButton.h"

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <string>

#include "Assets.h"
#include "Font.h"
#include "Inputs.h"
#include "Random.h"
#include "Scene.h"
#include "TextureArray.h"

namespace GPC
{
    void UiCanvas::SetSize(const glm::vec2& size)
    {
        m_Size = size;

        if (Tint.r() <= 0.99f) return;
        uint8_t color = static_cast<uint8_t>(Random::Integer(200, 255));
        Tint = Color(color, color, color, 100);
    }

    void UiCanvas::SetBackgroundSprite(const std::string& atlas, const std::string& name)
    {
        auto* arrayAsset = GPC_TEXTURE_ARRAY(atlas);
        if (!arrayAsset || !arrayAsset->HasTexture(name)) return;

        m_Material.TextureArray = arrayAsset->Array.GetUploadId();
        m_Material.Diffuse      = arrayAsset->GetTextureID(name);
        mp_Sprite               = &arrayAsset->Array;
    }

    void UiText::CalcElementSize()
    {
        if (!m_Updated) return;
        m_Updated = false;

        if (!mp_Font)
        {
            m_Size = { 0.0f, 0.0f };
            return;
        }

        float minX = FLT_MAX, maxX = -FLT_MAX;
        float minY = FLT_MAX, maxY = -FLT_MAX;

        float cursorX = 0.0f;

        for (char c : m_Text)
        {
            const auto ch = mp_Font->GetChar(c);

            const float x0 = cursorX + static_cast<float>(ch.Bearing.x);
            const float x1 = x0 + static_cast<float>(ch.Size.x);

            const float y0 = static_cast<float>(ch.Bearing.y) - ch.Size.y;
            const float y1 = static_cast<float>(ch.Bearing.y);

            minX = std::min(minX, x0);
            maxX = std::max(maxX, x1);
            minY = std::min(minY, y0);
            maxY = std::max(maxY, y1);

            cursorX += static_cast<float>(ch.AdvanceWidth >> 6);
        }

        m_Size = { maxX - minX, maxY - minY };
    }

    void UiText::SetText(std::string_view text)
    {
        m_Updated = true;
        m_Text    = text;
    }

    void UiText::SetFontSize(float size)
    {
        m_FontSize = size;
        if (mp_Font)
        {
            m_FontSizeRatio = m_FontSize / static_cast<float>(mp_Font->GetFontSize());
        }
    }

    void UiText::SetFont(const Font* pFont)
    {
        mp_Font = pFont;
        if (mp_Font && m_FontSize > 0.0f)
        {
            m_FontSizeRatio = m_FontSize / static_cast<float>(mp_Font->GetFontSize());
        }
    }

    void UiText::SetFont(const std::string& fontName)
    {
        auto* asset = GPC_ASSETS->GetAssets(fontName);
        if (!asset) return;
        auto* fontAsset = asset->ToFont();
        if (!fontAsset) return;
        SetFont(&fontAsset->Font_);
    }

    void UiText::BindReactive(UiReactive<std::string>& source)
    {
        if (mp_BoundStringSource && m_BoundHandle)
        {
            mp_BoundStringSource->OnChanged.Disconnect(m_BoundHandle);
        }
        mp_BoundStringSource = &source;
        auto* self = this;
        m_BoundHandle = source.OnChanged.ConnectLambda(
            [self](const std::string& v) { self->SetText(v); });
        SetText(source.Get());
    }

    void UiText::BindReactive(UiReactive<int>& source)
    {
        if (mp_BoundIntSource && m_BoundHandle)
        {
            mp_BoundIntSource->OnChanged.Disconnect(m_BoundHandle);
        }
        mp_BoundIntSource = &source;
        auto* self = this;
        m_BoundHandle = source.OnChanged.ConnectLambda(
            [self](const int& v)
            {
                std::snprintf(self->m_FormatBuffer, sizeof(self->m_FormatBuffer), "%d", v);
                self->SetText(self->m_FormatBuffer);
            });
        std::snprintf(m_FormatBuffer, sizeof(m_FormatBuffer), "%d", source.Get());
        SetText(m_FormatBuffer);
    }

    void UiText::BindReactive(UiReactive<float>& source)
    {
        if (mp_BoundFloatSource && m_BoundHandle)
        {
            mp_BoundFloatSource->OnChanged.Disconnect(m_BoundHandle);
        }
        mp_BoundFloatSource = &source;
        auto* self = this;
        m_BoundHandle = source.OnChanged.ConnectLambda(
            [self](const float& v)
            {
                std::snprintf(self->m_FormatBuffer, sizeof(self->m_FormatBuffer), "%.2f", v);
                self->SetText(self->m_FormatBuffer);
            });
        std::snprintf(m_FormatBuffer, sizeof(m_FormatBuffer), "%.2f", source.Get());
        SetText(m_FormatBuffer);
    }

    void UiSprite::CalcElementSize()
    {
        if (m_Sprite) m_Size = m_Sprite->GetSize();
    }

    void UiSprite::SetSprite(const std::string& atlas, const std::string& name)
    {
        auto* arrayAsset = GPC_TEXTURE_ARRAY(atlas);
        if (!arrayAsset || !arrayAsset->HasTexture(name)) return;

        m_Material.TextureArray = arrayAsset->Array.GetUploadId();
        m_Material.Diffuse      = arrayAsset->GetTextureID(name);
        m_Sprite                = &arrayAsset->Array;
    }

    void UiSprite::BindTintReactive(UiReactive<Color>& source)
    {
        if (mp_BoundTintSource && m_BoundTintHandle)
        {
            mp_BoundTintSource->OnChanged.Disconnect(m_BoundTintHandle);
        }
        mp_BoundTintSource = &source;
        auto* self = this;
        m_BoundTintHandle = source.OnChanged.ConnectLambda(
            [self](const Color& v) { self->SetTint(v); });
        SetTint(source.Get());
    }

    void UiButton::OnCreate(const BehaviorCreateContext* pCtx)
    {
        Behavior::OnCreate(pCtx);
        if (!pCtx || !pCtx->pScene) return;

        const EntityID eid = GetEntityID();
        pTransform = pCtx->pScene->GetComponent<Transform2D>(eid);

        if (pCtx->pScene->HasComponent<UiCanvas>(eid))       pElement = pCtx->pScene->GetComponent<UiCanvas>(eid);
        else if (pCtx->pScene->HasComponent<UiText>(eid))    pElement = pCtx->pScene->GetComponent<UiText>(eid);
        else if (pCtx->pScene->HasComponent<UiSprite>(eid))  pElement = pCtx->pScene->GetComponent<UiSprite>(eid);

        if (pElement) pElement->IsButton = true;
    }

    void UiButton::OnUpdate(const BehaviorUpdateContext* pCtx)
    {
        Behavior::OnUpdate(pCtx);

        bool mouseOver = IsMouseOver();

        if (mouseOver && !IsHovered)
        {
            IsHovered = true;
            OnHover.Emit();
        }
        else if (!mouseOver && IsHovered)
        {
            IsHovered = false;
            OnLeave.Emit();
        }
    }

    bool UiButton::IsMouseOver() const
    {
        if (!pElement || IsDisabled) return false;

        const glm::vec2 pos  = pElement->GetPosition();
        const glm::vec2 size = pElement->GetScaledSize();

        return PointInRect(Inputs::GetMouseX(), Inputs::GetMouseY(),
                           pos.x, pos.y, size.x, size.y);
    }

    bool UiButton::PointInRect(float mx, float my, float rx, float ry, float rw, float rh)
    {
        return mx >= rx && mx <= rx + rw && my >= ry && my <= ry + rh;
    }
}
