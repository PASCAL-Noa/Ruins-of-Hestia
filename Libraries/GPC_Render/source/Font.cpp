#include "Font.h"

namespace GPC
{
    ErrorType Font::Create(std::string_view filePath, uint32_t fontSize)
    {
        m_FontSize = fontSize;

        if (!m_Initialized) {
            if (FT_Init_FreeType(&m_FontLib))
                return ErrorType::FT_FONT_LIB_INITIALIZATION;
            m_Initialized = true;
        }

        if (FT_New_Face(m_FontLib, filePath.data(), 0, &m_FontFace))
            return ErrorType::FT_FONT_NOT_LOAD;

        FT_Set_Pixel_Sizes(m_FontFace, 0, fontSize);

        const std::vector<std::pair<uint32_t, uint32_t>> ranges = {
            { 0x0020, 0x007E },
            { 0x00A0, 0x00FF },
            { 0x2018, 0x201E },
        };

        std::vector<std::vector<Pixel>> allCharsPixels;
        uint32_t index = 0;

        for (auto const& [first, last] : ranges)
        {
            for (uint32_t cp = first; cp <= last; cp++)
            {
                if (FT_Load_Char(m_FontFace, static_cast<FT_ULong>(cp), FT_LOAD_RENDER))
                    continue;

                ImageInformation info {};
                info.Width  = m_FontFace->glyph->bitmap.width;
                info.Height = m_FontFace->glyph->bitmap.rows;
                info.Stride = 4;

                allCharsPixels.emplace_back(info.Width * info.Height);
                auto& pixels = allCharsPixels.back();

                for (size_t i = 0; i < pixels.size(); i++) {
                    Pixel& pixel = pixels[i];
                    pixel.r = m_FontFace->glyph->bitmap.buffer[i];
                    pixel.g = pixel.b = pixel.r;
                    pixel.a = pixel.r > 0 ? 255 : 0;
                }
                info.Data = reinterpret_cast<uint8_t*>(pixels.data());

                Character character {};
                character.Size.x       = info.Width;
                character.Size.y       = info.Height;
                character.Bearing.x    = m_FontFace->glyph->bitmap_left;
                character.Bearing.y    = m_FontFace->glyph->bitmap_top;
                character.TextureID_   = m_Textures.AddImage(info, index++);
                character.AdvanceWidth = m_FontFace->glyph->advance.x;
                m_CharacterMap[cp]     = character;

                if (info.Width  > m_MaxWidth)  m_MaxWidth  = info.Width;
                if (info.Height > m_MaxHeight) m_MaxHeight = info.Height;
            }
        }

        TextureArrayInformation arrayInfo {};
        arrayInfo.Width  = m_MaxWidth;
        arrayInfo.Height = m_MaxHeight;
        arrayInfo.Format = VK_FORMAT_R8G8B8A8_SRGB;
        m_Textures.Create(arrayInfo);

        FT_Done_Face(m_FontFace);
        return ErrorType::SUCCESS;
    }

    void Font::Destroy()
    {
        FT_Done_FreeType(m_FontLib);
    }

    Character const& Font::GetChar(const uint32_t c) const
    {
        auto it = m_CharacterMap.find(c);
        if (it != m_CharacterMap.end())
            return it->second;

        it = m_CharacterMap.find('?');
        if (it != m_CharacterMap.end())
            return it->second;

        return m_CharacterMap.begin()->second;
    }
}
