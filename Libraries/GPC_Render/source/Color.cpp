#include "Color.h"

#include "GLM_Include.h"

namespace GPC
{
    Color & Color::operator=(Hexadecimal const &color)
    {

        m_ColorValue = glm::vec4(
                static_cast<float>((color >> 24) & 0xff) / 255.0f,
                static_cast<float>((color >> 16) & 0xff) / 255.0f,
                static_cast<float>((color >> 8) & 0xff) / 255.0f,
                static_cast<float>(color & 0xff) / 255.0f);
        return *this;

    }

    Color::operator glm::vec<4, float>() const {
        return m_ColorValue;
    }

    Color Color::Lerp(Color a, Color b, float t)
    {
        return Color(glm::lerp(a.m_ColorValue, b.m_ColorValue, t));
    }
} // GPC