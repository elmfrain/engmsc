#include "engmsc/Engine2DRenderer.hpp"

#include "Mesh.hpp"
#include "Shaders.hpp"
#include "UIRender.hpp"
#include "Logger.hpp"

#include <glm/gtx/transform.hpp>

static EMMesh::Ptr m_crankshaftMesh;
static EMMesh::Ptr m_conrodMesh;
static EMMesh::Ptr m_pistonMesh;
static EMEngine m_engine;

static bool m_hasInit = false;

static EMWindow* window;
static const EMKeyboard* keyboard;
static const EMMouse* mouse;

static glm::vec2 m_camPos(0.0);
static float m_zoom = 1.0f;

static EMLogger m_logger("Engine2D Renderer");

static void i_updateCamFromInputs();

void EMEngine2DRenderer::init()
{
    if(m_hasInit) return;

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

    m_crankshaftMesh = EMMesh::load("res/engine2D/crankshaft.ply")[0];
    m_conrodMesh = EMMesh::load("res/engine2D/conrod.ply")[0];
    m_pistonMesh = EMMesh::load("res/engine2D/piston.ply")[0];

    m_crankshaftMesh->makeRenderable(vtxFmt);
    m_conrodMesh->makeRenderable(vtxFmt);
    m_pistonMesh->makeRenderable(vtxFmt);

    window = &emui::getWindow();
    keyboard = &window->getKeyboard();
    mouse = &window->getMouse();

    m_hasInit = true;
    m_logger.infof("Initialized Module");
}

EMEngine& EMEngine2DRenderer::getEngine()
{
    return m_engine;
}

void EMEngine2DRenderer::render()
{
    i_updateCamFromInputs();

    float viewRatio = emui::getUIWidth() / emui::getUIHeight();
    glm::mat4 projection = glm::ortho(-viewRatio, viewRatio, -1.0f, 1.0f, -500.0f, 500.0f);
    glm::mat4 modelview = glm::mat4(1.0f);
    projection = glm::translate(projection, {-m_camPos.x, -m_camPos.y, 0.0f});
    projection = glm::scale(projection, {m_zoom, m_zoom, 1.0f});

    ems::setProjectionMatrix(projection);
    ems::setModelviewMatrix(modelview);

    float pistonY =
    glm::sin(glm::acos(glm::cos((float) m_engine.crankAngle) / 2.4f)) * 1.2f +
    glm::sin((float) m_engine.crankAngle) / 2.0f;

    {
        glm::vec4 conrodPos(0.0f, 0.5f, 0.0f, 1.0f);
        conrodPos = glm::rotate((float) m_engine.crankAngle - glm::half_pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f)) * conrodPos;

        float a = glm::asin((pistonY - conrodPos.y) / 1.2f);
        float rodAngle = glm::mod((float) m_engine.crankAngle + glm::half_pi<float>(), glm::two_pi<float>());
        rodAngle = rodAngle > glm::pi<float>() ? a : glm::pi<float>() - a;
        rodAngle -= glm::half_pi<float>();

        modelview = glm::mat4(1.0f);
        modelview = glm::translate(modelview, {conrodPos.x, conrodPos.y, 0.0f});
        modelview = glm::rotate(modelview, rodAngle, {0, 0, 1});
        ems::setModelviewMatrix(modelview);
        ems::POS_COLOR_shader();
        m_conrodMesh->render(GL_TRIANGLES);
    }

    modelview = glm::mat4(1.0f);
    modelview = glm::translate(modelview, {0.0f, pistonY, 0.0f});
    ems::setModelviewMatrix(modelview);
    ems::POS_COLOR_shader();
    m_pistonMesh->render(GL_TRIANGLES);

    modelview = glm::mat4(1.0f);
    modelview = glm::rotate(modelview, (float) m_engine.crankAngle, {0, 0, 1});
    ems::setModelviewMatrix(modelview);
    ems::POS_COLOR_shader();
    m_crankshaftMesh->render(GL_TRIANGLES);
}

static void i_updateCamFromInputs()
{
    float viewRatio = emui::getUIWidth() / emui::getUIHeight();

    if(mouse->isButtonPressed(GLFW_MOUSE_BUTTON_1))
    {
        m_camPos.x -= viewRatio * 2.0f * mouse->cursorXDelta() / window->getWidth() ;
        m_camPos.y += 2.0f * mouse->cursorYDelta() / window->getHeight();
    }

    if(mouse->justScrolled())
    {
        m_zoom *= mouse->scrollDeltaY() > 0 ? 1.2f : (1.0f / 1.2f);
    }
}