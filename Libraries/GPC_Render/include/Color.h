#pragma once
#include <cstdint>
#include <glm/vec4.hpp>

#include "Object.h"

namespace GPC
{

    struct Color {
    public:
        //   RRGGBBAA
        // 0xffffffff
        using Hexadecimal = uint32_t;

        Color() : m_ColorValue() {}

        // From an hexadecimal code (transparency included).
        Color(Hexadecimal code) :
            m_ColorValue(
                static_cast<float>((code >> 24) & 0xff) / 255.0f,
                static_cast<float>((code >> 16) & 0xff) / 255.0f,
                static_cast<float>((code >> 8) & 0xff) / 255.0f,
                static_cast<float>(code & 0xff) / 255.0f){}
        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) :
        m_ColorValue(
            static_cast<float>(r) / 255.0f,
            static_cast<float>(g) / 255.0f,
            static_cast<float>(b) / 255.0f,
            static_cast<float>(a) / 255.0f){}
        Color(glm::vec4 color) : m_ColorValue(color) {}

        Color& operator= (Hexadecimal const& color);
        operator glm::vec4() const;

        static Color Lerp(Color a, Color b, float t);
        float r() const { return m_ColorValue.r; }

    private:
        // Unsigned normalized RGBA value in the range [0, 1].
        glm::vec4 m_ColorValue;

    };

    namespace Colors
    {
        // — Neutres —
        static inline Color BLACK           = Color(0x0C0A09FF);
        static inline Color WHITE           = Color(0xFFFFFFFF);
        static inline Color GRAY            = Color(0x808080FF);
        static inline Color LIGHT_GRAY      = Color(0xD3D3D3FF);
        static inline Color DARK_GRAY       = Color(0x404040FF);
        static inline Color SILVER          = Color(0xC0C0C0FF);
        static inline Color CHARCOAL        = Color(0x36454FFF);
        static inline Color IVORY           = Color(0xFFFFF0FF);
        static inline Color BEIGE           = Color(0xF5F5DCFF);
        static inline Color CREAM           = Color(0xFFFDD0FF);

        // — Rouges & Roses —
        static inline Color RED             = Color(0xE7180BFF);
        static inline Color DARK_RED        = Color(0x8B0000FF);
        static inline Color CRIMSON         = Color(0xDC143CFF);
        static inline Color FIREBRICK       = Color(0xB22222FF);
        static inline Color TOMATO          = Color(0xFF6347FF);
        static inline Color SALMON          = Color(0xFA8072FF);
        static inline Color LIGHT_SALMON    = Color(0xFFA07AFF);
        static inline Color CORAL           = Color(0xFF7F50FF);
        static inline Color HOT_PINK        = Color(0xFF69B4FF);
        static inline Color DEEP_PINK       = Color(0xFF1493FF);
        static inline Color PINK            = Color(0xFFC0CBFF);
        static inline Color LIGHT_PINK      = Color(0xFFB6C1FF);
        static inline Color ROSE            = Color(0xFF007FFF);
        static inline Color MAGENTA         = Color(0xFF00FFFF);
        static inline Color FUCHSIA         = Color(0xFF0090FF);
        static inline Color MAROON          = Color(0x800000FF);
        static inline Color BLUSH           = Color(0xDE5D83FF);

        // — Oranges & Jaunes —
        static inline Color ORANGE          = Color(0xFF692AFF);
        static inline Color DARK_ORANGE     = Color(0xFF8C00FF);
        static inline Color GOLD            = Color(0xFFD700FF);
        static inline Color YELLOW          = Color(0xFFFF00FF);
        static inline Color LIGHT_YELLOW    = Color(0xFFFFE0FF);
        static inline Color LEMON           = Color(0xFFF44FFF);
        static inline Color AMBER           = Color(0xFFBF00FF);
        static inline Color PEACH           = Color(0xFFCBA4FF);
        static inline Color APRICOT         = Color(0xFBCEB1FF);
        static inline Color TANGERINE       = Color(0xF28500FF);
        static inline Color SAFFRON         = Color(0xF4C430FF);
        static inline Color MUSTARD         = Color(0xFFDB58FF);
        static inline Color KHAKI           = Color(0xF0E68CFF);
        static inline Color GOLDEN_ROD      = Color(0xDAA520FF);
        static inline Color SANDY_BROWN     = Color(0xF4A460FF);

        // — Verts —
        static inline Color GREEN           = Color(0x31D492FF);
        static inline Color DARK_GREEN      = Color(0x006400FF);
        static inline Color LIME            = Color(0x00FF00FF);
        static inline Color LIME_GREEN      = Color(0x32CD32FF);
        static inline Color FOREST_GREEN    = Color(0x228B22FF);
        static inline Color SEA_GREEN       = Color(0x2E8B57FF);
        static inline Color MEDIUM_GREEN    = Color(0x3CB371FF);
        static inline Color SPRING_GREEN    = Color(0x00FF7FFF);
        static inline Color OLIVE           = Color(0x808000FF);
        static inline Color OLIVE_DRAB      = Color(0x6B8E23FF);
        static inline Color YELLOW_GREEN    = Color(0x9ACD32FF);
        static inline Color CHARTREUSE      = Color(0x7FFF00FF);
        static inline Color LAWN_GREEN      = Color(0x7CFC00FF);
        static inline Color PALE_GREEN      = Color(0x98FB98FF);
        static inline Color MINT            = Color(0x98FF98FF);
        static inline Color EMERALD         = Color(0x50C878FF);
        static inline Color TEAL            = Color(0x008080FF);
        static inline Color DARK_CYAN       = Color(0x008B8BFF);
        static inline Color CYAN            = Color(0x00FFFFFFF);
        static inline Color AQUA            = Color(0x00FFFEFF);
        static inline Color TURQUOISE       = Color(0x40E0D0FF);
        static inline Color JADE            = Color(0x00A36CFF);
        static inline Color SAGE            = Color(0x9DC183FF);

        // — Bleus —
        static inline Color BLUE            = Color(0x155DFCFF);
        static inline Color DARK_BLUE       = Color(0x00008BFF);
        static inline Color NAVY            = Color(0x000080FF);
        static inline Color ROYAL_BLUE      = Color(0x4169E1FF);
        static inline Color STEEL_BLUE      = Color(0x4682B4FF);
        static inline Color DODGER_BLUE     = Color(0x1E90FFFF);
        static inline Color CORNFLOWER      = Color(0x6495EDFF);
        static inline Color SKY_BLUE        = Color(0x87CEEBFF);
        static inline Color LIGHT_BLUE      = Color(0xADD8E6FF);
        static inline Color DEEP_SKY_BLUE   = Color(0x00BFFFFF);
        static inline Color CADET_BLUE      = Color(0x5F9EA0FF);
        static inline Color POWDER_BLUE     = Color(0xB0E0E6FF);
        static inline Color SLATE_BLUE      = Color(0x6A5ACDFF);
        static inline Color MEDIUM_BLUE     = Color(0x0000CDFF);
        static inline Color CERULEAN        = Color(0x007BA7FF);
        static inline Color AZURE           = Color(0xF0FFFFFF);
        static inline Color INDIGO          = Color(0x4B0082FF);

        // — Violets & Mauves —
        static inline Color PURPLE          = Color(0xAD46FFFF);
        static inline Color DARK_PURPLE     = Color(0x800080FF);
        static inline Color VIOLET          = Color(0xEE82EEFF);
        static inline Color DARK_VIOLET     = Color(0x9400D3FF);
        static inline Color MEDIUM_PURPLE   = Color(0x9370DBFF);
        static inline Color BLUE_VIOLET     = Color(0x8A2BE2FF);
        static inline Color ORCHID          = Color(0xDA70D6FF);
        static inline Color MEDIUM_ORCHID   = Color(0xBA55D3FF);
        static inline Color PLUM            = Color(0xDDA0DDFF);
        static inline Color THISTLE         = Color(0xD8BFD8FF);
        static inline Color LAVENDER        = Color(0xE6E6FAFF);
        static inline Color LILAC           = Color(0xC8A2C8FF);
        static inline Color WISTERIA        = Color(0xC9A0DCFF);
        static inline Color MAUVE           = Color(0xE0B0FFFF);
        static inline Color AMETHYST        = Color(0x9966CCFF);
        static inline Color PERIWINKLE      = Color(0xCCCCFFFF);

        // — Marrons & Terres —
        static inline Color BROWN           = Color(0xA52A2AFF);
        static inline Color PERFECT_BROWN   = Color(0x270A00FF);
        static inline Color DARK_BROWN      = Color(0x654321FF);
        static inline Color SIENNA          = Color(0xA0522DFF);
        static inline Color SADDLE_BROWN    = Color(0x8B4513FF);
        static inline Color CHOCOLATE       = Color(0xD2691EFF);
        static inline Color TAN             = Color(0xD2B48CFF);
        static inline Color BISQUE          = Color(0xFFE4C4FF);
        static inline Color BURLYWOOD       = Color(0xDEB887FF);
        static inline Color WHEAT           = Color(0xF5DEB3FF);
        static inline Color PERU            = Color(0xCD853FFF);
        static inline Color RUSSET          = Color(0x80461BFF);
        static inline Color UMBER           = Color(0x635147FF);
        static inline Color MAHOGANY        = Color(0xC04000FF);
        static inline Color SEPIA           = Color(0x704214FF);
        static inline Color TERRACOTTA      = Color(0xE2725BFF);

        // — Spéciaux —
        static inline Color TRANSPARENT     = Color(0x00000000);
        static inline Color SNOW            = Color(0xFFFAFAFF);
        static inline Color GHOST_WHITE     = Color(0xF8F8FFFF);
        static inline Color FLORAL_WHITE    = Color(0xFFFAF0FF);
        static inline Color LINEN           = Color(0xFAF0E6FF);
        static inline Color ANTIQUE_WHITE   = Color(0xFAEBD7FF);
        static inline Color MISTY_ROSE      = Color(0xFFE4E1FF);
        static inline Color LAVENDER_BLUSH  = Color(0xFFF0F5FF);
        static inline Color ALICE_BLUE      = Color(0xF0F8FFFF);
        static inline Color HONEYDEW        = Color(0xF0FFF0FF);
        static inline Color MINT_CREAM      = Color(0xF5FFFAFF);
        static inline Color OLD_LACE        = Color(0xFDF5E6FF);
    }

} // GPC