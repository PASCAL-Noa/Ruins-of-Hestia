#pragma once
#include "Object.h"

#include <ft2build.h>

#include "ErrorType.h"
#include "TextureArray.h"

#include FT_FREETYPE_H


namespace GPC
{

    struct Character
    {
        TextureID   TextureID_;
        glm::ivec2  Size;
        glm::ivec2  Bearing;
        uint32_t    AdvanceWidth;    // Offset to advance to next glyph
    };

    struct Pixel
    {
        uint8_t r, g, b, a;
    };

    GPC_INHERIT_OBJECT( Font )
    {

    public:
        Font() = default;
        ~Font() override = default;

        ErrorType Create(std::string_view filePath, uint32_t fontSize);
        void Destroy();
        [[nodiscard]] Character const& GetChar(uint32_t c) const;
        [[nodiscard]] TextureArray const& GetTextureArray() const { return m_Textures; } ;
        uint32_t GetMaxWidth() const { return m_MaxWidth; };
        uint32_t GetMaxHeight() const { return m_MaxHeight; };
        uint32_t GetFontSize() const { return m_FontSize; };
    private:

        static inline FT_Library    m_FontLib = {};
        static inline bool          m_Initialized = false;
        FT_Face                     m_FontFace {};

        TextureArray                m_Textures;
        std::unordered_map<uint32_t, Character>     m_CharacterMap;
        std::vector<std::vector<uint32_t>>      m_FontCharacters;

        uint32_t m_MaxWidth     = 0;
        uint32_t m_MaxHeight    = 0;
        uint32_t m_FontSize     = 0;

    };
}
