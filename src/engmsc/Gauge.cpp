#include "engmsc/Gauge.hpp"

#include "UIRender.hpp"
#include "Shaders.hpp"

#include <glm/gtc/matrix_transform.hpp>

#define layoutVertex(v) (const float)   v.pos.x,\
                        (const float)   v.pos.y,\
                        (const float)      0.0f,\
                        (const float) v.color.r,\
                        (const float) v.color.g,\
                        (const float) v.color.b,\
                        (const float) v.color.a

EMMesh::Ptr EMGauge::m_needleMesh;
EMMesh::Ptr EMGauge::m_stubbyNeedleMesh;

struct Vertex
{
    glm::vec2 pos;
    glm::vec4 color;
};

const Vertex i_backingSegment[] =
{
    {{0.0f, 0.10f}, {0.0f, 0.0f, 0.0f, 0.2f}},
    {{0.0f, 0.66f}, {0.0f, 0.0f, 0.0f, 0.2f}},
    {{0.0f, 0.66f}, {0.0f, 0.0f, 0.0f, 0.2f}},
    {{0.0f, 0.97f}, {0.0f, 0.0f, 0.0f, 0.5f}},
    {{0.0f, 0.97f}, {0.4f, 0.4f, 0.4f, 1.0f}},
    {{0.0f, 1.00f}, {0.4f, 0.4f, 0.4f, 1.0f}},
    {{0.0f, 1.00f}, {0.0f, 0.0f, 0.0f, 0.1f}},
    {{0.0f, 1.05f}, {0.0f, 0.0f, 0.0f, 0.0f}}
};

EMGauge::EMGauge() :
    m_minValue(0.0f),
    m_maxValue(1.0f)
{
    EMVertexFormat vtxFmt;
    vtxFmt.size = 2;
    vtxFmt[0].data = EMVF_ATTRB_USAGE_POS
                   | EMVF_ATTRB_SIZE(3)
                   | EMVF_ATTRB_TYPE_FLOAT
                   | EMVF_ATTRB_NORMALIZED_FALSE;
    vtxFmt[1].data = EMVF_ATTRB_USAGE_COLOR
                   | EMVF_ATTRB_SIZE(4)
                   | EMVF_ATTRB_TYPE_FLOAT
                   | EMVF_ATTRB_NORMALIZED_FALSE;

    m_meshBuilder = std::make_unique<EMMeshBuilder>(vtxFmt);

    m_profile.radius = 200;
    m_profile.girth = 360;
    m_profile.markingGirth = 270;
    m_profile.tilt = 0;

    m_type = GAUGE;

    // Make sure needle mesh is ready
    initMeshes(vtxFmt);
}

EMGauge::Profile& EMGauge::getProfile()
{
    return m_profile;
}

void EMGauge::setValue(float value)
{
    m_value = value;
    float amount = m_value / (m_maxValue - m_minValue) + m_minValue;
    m_smoother.grab(amount);
}

float EMGauge::getValue() const
{
    return m_value;
}

void EMGauge::getRange(float* getMin, float* getMax) const
{
    if(getMin) *getMin = m_minValue;
    if(getMax) *getMax = m_maxValue;
}

void EMGauge::doDraw()
{
    m_meshBuilder->reset();
    glm::mat4* modelView = &m_meshBuilder->getModelView();
    *modelView = emui::getModelView();

    const float translateX = x + width / 2.0f;
    const float translateY = y + height / 2.0f;

    *modelView = glm::translate(*modelView, {translateX, translateY, 0.0f});
    *modelView = glm::scale(*modelView, {m_profile.radius, m_profile.radius, 0.0f});

    generateBacking();
    generateMarkings();

    ems::POS_COLOR_shader();
    m_meshBuilder->drawElements(GL_TRIANGLES);

    renderText();
    renderNeedle();
}

void EMGauge::generateBacking()
{
    glm::mat4* modelView = &m_meshBuilder->pushMatrix();

    const float segArc = glm::radians(m_profile.girth) / m_profile.numSegments;
    const float arcOffset = glm::radians(360 - m_profile.girth) / 2.0f + glm::radians(m_profile.tilt);
    *modelView = glm::rotate(*modelView, arcOffset, {0, 0, 1});

    for(int s = 0; s < m_profile.numSegments; s++)
    {
        *modelView = glm::rotate(*modelView, segArc, {0, 0, 1});
        for(int i = 0; i < 4; i++)
        {
            const Vertex& v1 = i_backingSegment[i * 2];
            const Vertex& v2 = i_backingSegment[i * 2 + 1];

            m_meshBuilder->index(6, 0, 1, 2, 0, 2, 3);

            modelView = &m_meshBuilder->pushMatrix();
            m_meshBuilder->vertex(NULL, layoutVertex(v2));
            m_meshBuilder->vertex(NULL, layoutVertex(v1));

            *modelView = glm::rotate(*modelView, segArc, {0, 0, 1});

            m_meshBuilder->vertex(NULL, layoutVertex(v1));
            m_meshBuilder->vertex(NULL, layoutVertex(v2));
            modelView = &m_meshBuilder->popMatrix();
        }
    }

    m_meshBuilder->popMatrix();
}

void EMGauge::generateMarkings()
{
    glm::mat4* modelView = &m_meshBuilder->pushMatrix();

    const float markArc = glm::radians(m_profile.markingGirth) / (m_profile.numMarkings - 1);
    const float arcOffset = glm::radians(360 - m_profile.markingGirth) / 2.0f + glm::radians(m_profile.tilt);
    *modelView = glm::rotate(*modelView, arcOffset, {0, 0, 1});

    for(int m = 0; m < m_profile.numMarkings; m++)
    {   
        m_meshBuilder->index(6, 0, 1, 2, 0, 2, 3);

        m_meshBuilder->
        vertex(NULL, -0.014f, 0.73f, 0.0f, 1.0f, 1.0f, 1.0f, 0.9f).
        vertex(NULL,  0.014f, 0.73f, 0.0f, 1.0f, 1.0f, 1.0f, 0.9f).
        vertex(NULL,  0.014f, 0.87f, 0.0f, 1.0f, 1.0f, 1.0f, 0.9f).
        vertex(NULL, -0.014f, 0.87f, 0.0f, 1.0f, 1.0f, 1.0f, 0.9f);

        *modelView = glm::rotate(*modelView, markArc, {0, 0, 1});
    }

    m_meshBuilder->popMatrix();
}

void EMGauge::renderText()
{
    const float midX = x + width / 2.0f;
    const float midY = y + height / 2.0f;

    float prevTextSize = emui::getFontRenderer().getTextSize();
    emui::getFontRenderer().setTextSize(m_profile.radius * (m_profile.radius < 120 ? 0.19f : 0.14f));

    // Show gauge name
    emui::genString(getText(), midX, midY - m_profile.radius / 3, 0xFFFFFFFF, emui::CENTER);

    glm::mat4* modelView = &m_meshBuilder->pushMatrix();
    glm::vec4 textCursor(0.0f, 0.64f, 0.0f, 1.0f);

    const float markArc = glm::radians(m_profile.markingGirth) / (m_profile.numMarkings - 1);
    const float arcOffset = glm::radians(360 - m_profile.markingGirth) / 2.0f + glm::radians(m_profile.tilt);

    *modelView = glm::rotate(*modelView, arcOffset, {0, 0, 1});

    for(int i = 0; i < m_profile.numMarkings; i++)
    {
        char text[128];
        snprintf(text, sizeof(text), "§l%d", i);

        glm::vec4 textPlacement = *modelView * textCursor;
        emui::genString(text, textPlacement.x, textPlacement.y, 0xFFFFFFFF, emui::CENTER);

        *modelView = glm::rotate(*modelView, markArc, {0, 0, 1});
    }

    m_meshBuilder->popMatrix();

    emui::getFontRenderer().setTextSize(prevTextSize);

    emui::renderBatch();
}

void EMGauge::renderNeedle()
{
    glm::mat4* modelView = &m_meshBuilder->pushMatrix();
    
    const float amount = glm::max(0.0f, glm::min(m_smoother.getValuef(), 1.0f));
    const float needleAngle = glm::radians(m_profile.markingGirth) * amount;
    const float arcOffset = glm::radians(360 - m_profile.markingGirth) / 2.0f + glm::radians(m_profile.tilt);
    *modelView = glm::rotate(*modelView, arcOffset + needleAngle, {0, 0, 1});

    glm::mat4 prevShaderModelView = ems::getModelviewMatrix();
    ems::setModelviewMatrix(*modelView);
    ems::POS_COLOR_shader();

    if(m_profile.radius < 120) m_stubbyNeedleMesh->render(GL_TRIANGLES);
    else m_needleMesh->render(GL_TRIANGLES);

    ems::setModelviewMatrix(prevShaderModelView);

    m_meshBuilder->popMatrix();
}

void EMGauge::initMeshes(EMVertexFormat& vtxFmt)
{
    if(!m_needleMesh)
    {
        m_needleMesh = EMMesh::load("res/needle.ply")[0];
        m_needleMesh->makeRenderable(vtxFmt);

        m_stubbyNeedleMesh = EMMesh::load("res/stubby_needle.ply")[0];
        m_stubbyNeedleMesh->makeRenderable(vtxFmt);
    }
}