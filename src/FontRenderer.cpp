#include "FontRenderer.hpp"

#include <cstring>
#include <glm/glm.hpp>
#include <stb_image.h>

// Font placed here
#include "Consolas_font.hpp"

#include "GLInclude.hpp"
#include "Logger.hpp"

#define getVec4Color(intcolor)\
{ ((intcolor >> 16) & 0xFF) / 255.0f,\
  ((intcolor >>  8) & 0xFF) / 255.0f,\
  (intcolor & 0xFF) / 255.0f,\
  ((intcolor >> 24) & 0xFF) / 255.0f }
#define vec4Color(vec4color) vec4color[0], vec4color[1], vec4color[2], vec4color[3]

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

    for(size_t i = 0; i < strLen; i++)
    {
        const Glyph& glyph = m_font.glyphs[str[i]];
        
        cursor += glyph.xAdvance - m_font.leftPadding - 1;
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
    float xCursor = x;

    for(int i = 0; i < strLen;)
    {
        int bytesRead = 0;
        int unicode = getUnicodeFromUTF8((const uint8_t*) &str[i], &bytesRead);

        Glyph glyph = m_font.glyphs[unicode];

        genChar(meshBuilder, unicode, xCursor, y, 0.0f, color);

        xCursor += (glyph.xAdvance - m_font.leftPadding - m_font.rightPadding) * m_scale;

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

void EMFontRenderer::genChar(EMMeshBuilder& meshBuilder, int unicode, float x, float y, float italics, uint32_t color)
{
    if(256 <= unicode) // Cannot handle unicodes more than 256 at the moment
    {
        return;
    }

    Glyph glyph = m_font.glyphs[unicode];

    float left = (float) glyph.xOffset;
    float right = (float) glyph.xOffset + glyph.width;
    float top = (float) glyph.yOffset;
    float bottom = (float) glyph.yOffset + glyph.height;
    left *= m_scale; right *= m_scale;
    top *= m_scale; bottom *= m_scale;
    left += x; right += x;
    top += y; bottom += y;

    glm::vec4 colorv4 = getVec4Color(color);

    meshBuilder.index(6, 0, 1, 2, 0, 2, 3);
    meshBuilder.vertex(NULL, left  - italics, bottom, 0.0f, glyph.uvLeft , glyph.uvBottom, vec4Color(colorv4), m_texUnit);
    meshBuilder.vertex(NULL, right - italics, bottom, 0.0f, glyph.uvRight, glyph.uvBottom, vec4Color(colorv4), m_texUnit);
    meshBuilder.vertex(NULL, right + italics, top   , 0.0f, glyph.uvRight, glyph.uvTop   , vec4Color(colorv4), m_texUnit);
    meshBuilder.vertex(NULL, left  + italics, top   , 0.0f, glyph.uvLeft,  glyph.uvTop   , vec4Color(colorv4), m_texUnit);
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