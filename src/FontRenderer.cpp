#include "FontRenderer.hpp"

#include <cstring>
#include <glm/glm.hpp>
#include <stb_image.h>
#include <cstring>

// Font placed here
#include "Consolas_font.hpp"

#include "GLInclude.hpp"
#include "Logger.hpp"
#include "UIRender.hpp"

#define getVec4Color(intcolor)\
{ ((intcolor >> 16) & 0xFF) / 255.0f,\
  ((intcolor >>  8) & 0xFF) / 255.0f,\
  (intcolor & 0xFF) / 255.0f,\
  ((intcolor >> 24) & 0xFF) / 255.0f }
#define vec4Color(vec4color) vec4color[0], vec4color[1], vec4color[2], vec4color[3]

static const char* m_FORMATTING_KEYS = "0123456789abcdefklmnor";

static const uint32_t m_COLOR_CODES[] = 
{
    0xFF000000, 0xFF0000AA, 0xFF00AA00, 0xFF00AAAA, 0xFFAA0000, 0xFFAA00AA, 0xFFFFAA00, 0xFFAAAAAA,
    0xFF555555, 0xFF5555FF, 0xFF55FF55, 0xFF55FFFF, 0xFFFF5555, 0xFFFF55FF, 0xFFFFFF55, 0xFFFFFFFF
};

static EMLogger m_logger("Font Renderer");

EMFontRenderer::EMFontRenderer() :
    m_isRenderable(false),
    m_glAtlasTex(0),
    m_font(Consolas_Font()),
    m_scale(1.0f),
    m_textSize((float) m_font.fontHeight),
    m_anchor(EMFR_TOP_LEFT),
    m_texUnit(0)
{
    setTextSize(16);
}

EMFontRenderer::~EMFontRenderer()
{
    if(m_isRenderable)
    {
        glDeleteTextures(1, &m_glAtlasTex);
    }
}

const char* EMFontRenderer::getFontName() const
{
    return m_font.name;
}

float EMFontRenderer::getFontHeight() const
{
    return (float) m_font.fontHeight;
}

float EMFontRenderer::getFontLineHeight() const
{
    return (float) m_font.lineHeight;
}

float EMFontRenderer::getStringWidth(const char* str) const
{
    return getStringWidth(str, strlen(str));
}

float EMFontRenderer::getStringWidth(const char* str, size_t strLen) const
{
    int cursor = 0;

    for(int i = 0; i < strLen;)
    {
        int bytesRead = 0;
        int unicode = getUnicodeFromUTF8((const uint8_t*) &str[i], &bytesRead);
        
        if(unicode == 167 && i + bytesRead < strLen)
        {
            i++;
        }
        else
        {
            Glyph glyph = m_font.glyphs[unicode];

            cursor += glyph.xAdvance - m_font.leftPadding - m_font.rightPadding;
        }

        i += bytesRead;
    }

    return cursor * m_scale;
}

float EMFontRenderer::getStringHeight() const
{
    return m_textSize;
}

void EMFontRenderer::setTextSize(float size)
{
    size = glm::max(0.1f, size);
    m_textSize = size;
    m_scale = size / m_font.fontHeight;
}

float EMFontRenderer::getTextSize() const
{
    return m_textSize;
}

void EMFontRenderer::setAnchor(int anchorEnum)
{
    m_anchor = anchorEnum;
}

int EMFontRenderer::getAnchor() const
{
    return m_anchor;
}

void EMFontRenderer::genString(EMMeshBuilder& meshBuilder, const char* str, size_t strLen, float x, float y, uint32_t color)
{
    glm::vec2 anch = anchor(str, strLen, x, y);
    x = anch.x;
    y = anch.y;

    float xCursor = x;

    bool bold = false;
    bool strikethrough = false;
    bool underline = false;
    bool italic = false;
    uint32_t charColor = 0xFFFFFFFF;

    for(int i = 0; i < strLen;)
    {
        int bytesRead = 0;
        int unicode = getUnicodeFromUTF8((const uint8_t*) &str[i], &bytesRead);

        if(unicode == 167 && i + bytesRead < strLen)
        {
            char key[] = { (char) tolower((int) str[i + bytesRead]), (char) 0 };
            int formatKey = (int) (strstr(m_FORMATTING_KEYS, key) - m_FORMATTING_KEYS);

            if(formatKey < 16)
            {
                if(formatKey < 0) formatKey = 15;

                strikethrough = false;
                underline = false;
                italic = false;
                bold = false;
                charColor = m_COLOR_CODES[formatKey];
            }
            else
            {
                switch(formatKey)
                {
                case 17:
                    bold = true;
                    break;
                case 18:
                    strikethrough = true;
                    break;
                case 19:
                    underline = true;
                    break;
                case 20:
                    italic = true;
                    break;
                case 21:
                    bold = false;
                    strikethrough = false;
                    underline = false;
                    italic = false;
                    charColor = 0xFFFFFFFF;
                    break;
                }
            }

            i++;
        }
        else
        {
            Glyph glyph = m_font.glyphs[unicode];

            float advance = (glyph.xAdvance - m_font.leftPadding - m_font.rightPadding) * m_scale;
            float italics = italic ? m_textSize * 0.07f : 0;
            glm::vec4 colorv4     = getVec4Color(color);
            glm::vec4 charColorv4 = getVec4Color(charColor);
            glm::vec4 mixedColor = colorv4 * charColorv4;

            genChar(meshBuilder, unicode, xCursor, y, italics, bold, mixedColor);
            if(strikethrough)
            {
                emui::genHorizontalLine(y + m_textSize / 2, xCursor, xCursor + advance, mixedColor, m_textSize * 0.09f);
            }
            if(underline)
            {
                emui::genHorizontalLine(y + m_textSize * 1.05f, xCursor, xCursor + advance, mixedColor, m_textSize * 0.09f);
            }

            xCursor += advance;
        }

        i += bytesRead;
    }
}

void EMFontRenderer::genString(EMMeshBuilder& meshBuilder, const char* text, float x, float y, uint32_t color)
{
    genString(meshBuilder, text, strlen(text), x, y, color);
}

void EMFontRenderer::bindAtlas()
{
    if(!m_isRenderable)
    {
        initToRender();
    }

    glActiveTexture(GL_TEXTURE0 + m_texUnit - 1);
    glBindTexture(GL_TEXTURE_2D, m_glAtlasTex);
    glActiveTexture(GL_TEXTURE0);
}

void EMFontRenderer::setAtlasTexUnit(int texUnit)
{
    m_texUnit = texUnit;
}

int EMFontRenderer::getTexUnit() const
{
    return m_texUnit;
}

int EMFontRenderer::getUnicodeFromUTF8(const uint8_t* str, int* bytesRead) const
{
    int unicode = 0;

    if(*str < 128)
    {
        *bytesRead = 1;
        return *str;
    }
    else if((*str & 224) == 192) * bytesRead = 2;
    else if((*str & 240) == 224) * bytesRead = 3;
    else if((*str & 248) == 240) * bytesRead = 4;

    switch(*bytesRead)
    {
        case 2:
            unicode |= ((int) str[0] & 31) << 6;
            unicode |= str[1] & 63;
            break;
        case 3:
            unicode |= ((int) str[0] & 15) << 12;
            unicode |= ((int) str[1] & 63) << 6;
            unicode |= str[2] & 63;
            break;
        case 4:
            unicode |= ((int) str[0] & 8) << 18;
            unicode |= ((int) str[1] & 63) << 12;
            unicode |= ((int) str[2] & 63) << 6;
            unicode |= str[3] & 63;
    }

    return unicode;
}

void EMFontRenderer::genChar(EMMeshBuilder& meshBuilder, int unicode, float x, float y, float italics, bool bold, glm::vec4& color)
{
    if(256 <= unicode) // Cannot handle unicodes more than 256 at the moment
    {
        return;
    }

    int texUnit = m_texUnit + bold;
    Glyph glyph = m_font.glyphs[unicode];

    float left = (float) glyph.xOffset;
    float right = (float) glyph.xOffset + glyph.width;
    float top = (float) glyph.yOffset;
    float bottom = (float) glyph.yOffset + glyph.height;
    left *= m_scale; right *= m_scale;
    top *= m_scale; bottom *= m_scale;
    left += x; right += x;
    top += y; bottom += y;

    meshBuilder.index(6, 0, 1, 2, 0, 2, 3);
    meshBuilder.vertex(NULL, left  - italics, bottom, 0.0f, glyph.uvLeft , glyph.uvBottom, vec4Color(color), texUnit);
    meshBuilder.vertex(NULL, right - italics, bottom, 0.0f, glyph.uvRight, glyph.uvBottom, vec4Color(color), texUnit);
    meshBuilder.vertex(NULL, right + italics, top   , 0.0f, glyph.uvRight, glyph.uvTop   , vec4Color(color), texUnit);
    meshBuilder.vertex(NULL, left  + italics, top   , 0.0f, glyph.uvLeft,  glyph.uvTop   , vec4Color(color), texUnit);
}

glm::vec2 EMFontRenderer::anchor(const char* str, size_t strLen, float x, float y)
{
    glm::vec2 newPos;
    float stringWidth = getStringWidth(str, strLen);
    float stringHeight = m_textSize;

    switch(m_anchor)
    {
    case EMFR_LEFT:
    case EMFR_CENTER:
    case EMFR_RIGHT:
        newPos.y = y - stringHeight / 2.0f;
        break;
    case EMFR_BOTTOM_LEFT:
    case EMFR_BOTTOM_CENTER:
    case EMFR_BOTTOM_RIGHT:
        newPos.y = y - stringHeight;
        break;
    default:
        newPos.y = y;
    }

    switch(m_anchor)
    {
    case EMFR_TOP_CENTER:
    case EMFR_CENTER:
    case EMFR_BOTTOM_CENTER:
        newPos.x = x - stringWidth / 2.0f;
        break;
    case EMFR_TOP_RIGHT:
    case EMFR_RIGHT:
    case EMFR_BOTTOM_RIGHT:
        newPos.x = x - stringWidth;
        break;
    default:
        newPos.x = x;
    }

    return newPos;
}

void EMFontRenderer::initToRender()
{
    int x, y, channels;
    stbi_set_flip_vertically_on_load(true);

    stbi_uc* image = stbi_load_from_memory(m_font.atlas, (int) m_font.atlasLen, &x, &y, &channels, STBI_grey_alpha);
    m_atlasWidth = (float) x;
    m_atlasHeight = (float) y;

    if(!image)
    {
        m_logger.errorf("Failed to load texture of font \"%s\".", m_font.name);
        m_isRenderable = true;
    }

    glGenTextures(1, &m_glAtlasTex);

    glBindTexture(GL_TEXTURE_2D, m_glAtlasTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, x, y, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(image);
    m_logger.infof("Loaded font texture of font \"%s\".", m_font.name);
    m_isRenderable = true;

    // Precalculate all uv mappings for each glyph
    for(size_t i = 0; i < m_font.numGlyphs; i++)
    {
        const Glyph& glyph = m_font.glyphs[i];

        glyph.uvLeft = glyph.x / m_atlasWidth;
        glyph.uvRight = (glyph.x + glyph.width) / m_atlasWidth;
        glyph.uvTop = 1.0f - (glyph.y / m_atlasHeight);
        glyph.uvBottom = 1.0f - ((glyph.y + glyph.height) / m_atlasHeight);
    }
}