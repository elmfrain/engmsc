#ifndef EMFONT_RENDERER_HPP
#define EMFONT_RENDERER_HPP

#include "MeshBuilder.hpp"

#include <stdint.h>

// Anchor enums
#define EMFR_TOP_LEFT      4
#define EMFR_TOP_CENTER    5
#define EMFR_TOP_RIGHT     6
#define EMFR_LEFT          7
#define EMFR_CENTER        8
#define EMFR_RIGHT         9
#define EMFR_BOTTOM_LEFT   10
#define EMFR_BOTTOM_CENTER 11
#define EMFR_BOTTOM_RIGHT  12

class EMFontRenderer
{
public:
    struct Glyph
    {
        int x, y, width, height, xOffset, yOffset, xAdvance;
        mutable float uvLeft = 0.0f, uvTop = 0.0f, uvRight = 0.0f, uvBottom = 0.0f;
    };

    struct Font
    {
        const char* name;
        int fontHeight;
        int lineHeight;

        int leftPadding;
        int rightPadding;
        int topPadding;
        int bottomPadding;

        const Glyph* glyphs;
        const uint8_t* atlas;
        size_t atlasLen;
        int numGlyphs;
    };

    EMFontRenderer();
    ~EMFontRenderer();

    EMFontRenderer(const EMFontRenderer& copy) = delete;

    const char* getFontName() const;
    float getFontHeight() const;
    float getFontLineHeight() const;

    float getStringWidth(const char* text) const;
    float getStringWidth(const char* text, size_t strLen) const;
    float getStringHeight() const;

    void setTextSize(float size);
    float getTextSize() const;

    void setAnchor(int anchorEnum);
    int getAnchor() const;

    void genString(EMMeshBuilder& meshBuilder, const char* str, size_t strLen, float x, float y, uint32_t color);
    void genString(EMMeshBuilder& meshBuilder, const char* text, float x, float y, uint32_t color);

    void bindAtlas();

    void setAtlasTexUnit(int texUnit);
    int getTexUnit() const;
private:
    bool m_isRenderable;
    GLuint m_glAtlasTex;
    float m_atlasWidth = 0;
    float m_atlasHeight = 0;

    Font m_font;

    float m_scale;
    float m_textSize;

    int m_anchor;
    int m_texUnit;

    int getUnicodeFromUTF8(const uint8_t* str, int* bytesRead) const;
    void genChar(EMMeshBuilder& meshBuilder, int unicode, float x, float y, float italics, bool bold, glm::vec4& color);
    glm::vec2 anchor(const char* str, size_t strLen, float x, float y);
    void initToRender();
};

#endif // EMFONT_RENDERER_HPP