#include "UIRender.hpp"

#include "MeshBuilder.hpp"
#include "Shaders.hpp"
#include "Logger.hpp"

#include <assert.h>
#include <memory>
#include <glm/gtc/matrix_transform.hpp>

#define vec4Color(vec4color) vec4color[0], vec4color[1], vec4color[2], vec4color[3]

static bool m_hasInit = false;

// Logger
static EMLogger m_logger("UI Renderer");

// Batch Rendering
static std::unique_ptr<EMMeshBuilder> m_meshBuilder;

// Window
static const EMWindow* m_currentWindow = NULL;
static float m_UIscaleFactor = 1.0f;

namespace emui
{
    void init()
    {
        if(m_hasInit) return;
        assert(m_currentWindow != 0);

        EMVertexFormat vtxFmt;

        vtxFmt.size = 4; // Number of attributes

        // Position attribute
        vtxFmt[0].data = EMVF_ATTRB_USAGE_POS
                       | EMVF_ATTRB_TYPE_FLOAT
                       | EMVF_ATTRB_SIZE(3)
                       | EMVF_ATTRB_NORMALIZED_FALSE;
        
        // Texture coordinate attribute
        vtxFmt[1].data = EMVF_ATTRB_USAGE_UV
                       | EMVF_ATTRB_TYPE_FLOAT
                       | EMVF_ATTRB_SIZE(2)
                       | EMVF_ATTRB_NORMALIZED_FALSE;

        // Color attribute
        vtxFmt[2].data = EMVF_ATTRB_USAGE_COLOR
                       | EMVF_ATTRB_TYPE_FLOAT
                       | EMVF_ATTRB_SIZE(4)
                       | EMVF_ATTRB_NORMALIZED_FALSE;

        // Texture ID attribute
        vtxFmt[3].data = EMVF_ATTRB_USAGE_TEXID
                       | EMVF_ATTRB_TYPE_UINT
                       | EMVF_ATTRB_SIZE(1)
                       | EMVF_ATTRB_NORMALIZED_FALSE;

        m_meshBuilder.reset(new EMMeshBuilder(vtxFmt));

        m_logger.infof("Initialized Module");
        m_hasInit = true;
    }

    void setWindow(const EMWindow& window)
    {
        m_currentWindow = &window;

        init();
    }

    void genQuad(float left, float top, float right ,float bottom, ColorARGB8 color, uint8_t texId)
    {
        assert(texId <= ems::getMaxTextureUnits());

        glm::vec4 colorv4 = getVec4Color(color);

        m_meshBuilder->index(6, 0, 1, 2, 0, 2, 3);
        m_meshBuilder->
        vertex(NULL,  left, bottom, 0.0f, 0.0f, 0.0f, vec4Color(colorv4), texId).
        vertex(NULL, right, bottom, 0.0f, 1.0f, 0.0f, vec4Color(colorv4), texId).
        vertex(NULL, right, top   , 0.0f, 1.0f, 1.0f, vec4Color(colorv4), texId).
        vertex(NULL,  left, top   , 0.0f, 0.0f, 1.0f, vec4Color(colorv4), texId);
    }

    void genGradientQuad(float left, float top, float right, float bottom, ColorARGB8 startColor,
                         ColorARGB8 endColor, Direction direction, uint8_t texId)
    {
        assert(texId <= ems::getMaxTextureUnits());

        glm::vec4 c0, c1, c2, c3;

        switch(direction)
        {
        case TO_RIGHT:
            c0 = c3 = getVec4Color(startColor);
            c1 = c2 = getVec4Color(endColor);
            break;
        case TO_LEFT:
            c1 = c2 = getVec4Color(startColor);
            c0 = c3 = getVec4Color(endColor);
            break;
        case TO_TOP:
            c0 = c1 = getVec4Color(startColor);
            c2 = c3 = getVec4Color(endColor);
            break;
        default:
            c2 = c3 = getVec4Color(startColor);
            c0 = c1 = getVec4Color(endColor);
            break;
        }

        m_meshBuilder->index(6, 0, 1, 2, 0, 2, 3);
        m_meshBuilder->
        vertex(NULL,  left, bottom, 0.0f, 0.0f, 0.0f, vec4Color(c0), texId).
        vertex(NULL, right, bottom, 0.0f, 1.0f, 0.0f, vec4Color(c1), texId).
        vertex(NULL, right, top   , 0.0f, 1.0f, 1.0f, vec4Color(c2), texId).
        vertex(NULL,  left, top   , 0.0f, 0.0f, 1.0f, vec4Color(c3), texId);
    }

    void genVerticalLine(float x, float startY, float endY, ColorARGB8 color, float thickness)
    {
        const float thickness_2 = thickness / 2.0f;

        genQuad(x - thickness_2, startY, x + thickness_2, endY, color);
    }

    void genHorizontalLine(float y, float startX, float endX, ColorARGB8 color, float thickness)
    {
        const float thickness_2 = thickness / 2.0f;

        genQuad(startX, y - thickness_2, endX, y + thickness_2, color);
    }

    void genLine(float x1, float y1, float x2, float y2, ColorARGB8 color, float thickness)
    {
        const float thickness_2 = thickness / 2.0f;

        glm::vec2 p1(x1, y1), p2(x2, y2);

        float distance = glm::distance(p1, p2);
        float a = glm::asin((y2 - y1) / distance);
        a = x2 < x1 ? glm::pi<float>() - a : a;

        m_meshBuilder->pushMatrix();

        m_meshBuilder->getModelView() = glm::translate(m_meshBuilder->getModelView(), { x1, y1, 0.0f });
        m_meshBuilder->getModelView() = glm::rotate(m_meshBuilder->getModelView(), a, {0.0f, 0.0f, 1.0f});

        genQuad(0, -thickness_2, distance, thickness_2, color);

        m_meshBuilder->popMatrix();
    }

    float getUIWidth()
    {
        return m_currentWindow->getWidth() / m_UIscaleFactor;
    }

    float getUIHeight()
    {
        return m_currentWindow->getHeight() / m_UIscaleFactor;
    }

    float getUIScale()
    {
        return m_UIscaleFactor;
    }

    void setupUIRendering()
    {
        glm::mat4 projection = glm::ortho(0.0f, getUIWidth(), getUIHeight(), 0.0f, -500.0f, 500.0f);
        ems::setProjectionMatrix(projection);

        glDisable(GL_DEPTH_TEST);
        ems::setModelviewMatrix(m_meshBuilder->getModelView());
    }

    void renderBatch()
    {
        ems::POS_UV_COLOR_TEXID_shader();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_meshBuilder->drawElements(GL_TRIANGLES);
        m_meshBuilder->reset();
    }
}