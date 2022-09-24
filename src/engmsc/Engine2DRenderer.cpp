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
static double prevCrankAngle = 0.0;
static double m_prevTime = 0.0;
static int u_crankAngle = 0;
static int u_crankAngleDelta = 0;
static int u_numInstances = 0;
static int u_partID = 0;

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

    ems::ENGINE2D_shader();
    int shader = ems::getProgramID();
    u_crankAngle = glGetUniformLocation(shader, "u_crankAngle");
    u_crankAngleDelta = glGetUniformLocation(shader, "u_crankAngleDelta");
    u_numInstances = glGetUniformLocation(shader, "u_numInstances");
    u_partID = glGetUniformLocation(shader, "u_partID");

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

    const int motionblurSamples = 30;
    const float motionblurAmount = 0.025f;
    const double time = glfwGetTime();
    const float delta = float(time - m_prevTime);
    const float crankDelta = glm::max(-0.3f, float(m_engine.crankAngle - prevCrankAngle));

    float viewRatio = emui::getUIWidth() / emui::getUIHeight();
    glm::mat4 projection = glm::ortho(-viewRatio, viewRatio, -1.0f, 1.0f, -500.0f, 500.0f);
    glm::mat4 modelview = glm::mat4(1.0f);
    projection = glm::translate(projection, {-m_camPos.x, -m_camPos.y, 0.0f});
    projection = glm::scale(projection, {m_zoom, m_zoom, 1.0f});

    ems::setProjectionMatrix(projection);
    ems::setModelviewMatrix(modelview);
    ems::setColor(1.0f, 1.0f, 1.0f, 0.1f);
    ems::ENGINE2D_shader();
    glUniform1f(u_crankAngle, (float) glm::mod(m_engine.crankAngle, glm::two_pi<double>()));
    glUniform1f(u_crankAngleDelta, motionblurAmount * crankDelta / delta);
    glUniform1f(u_numInstances, motionblurSamples);

    float pistonY =
    glm::sin(glm::acos(glm::cos((float) m_engine.crankAngle) / 2.4f)) * 1.2f +
    glm::sin((float) m_engine.crankAngle) / 2.0f;

    glUniform1i(u_partID, 2);
    m_conrodMesh->renderInstanced(GL_TRIANGLES, motionblurSamples);

    glUniform1i(u_partID, 1);
    m_pistonMesh->renderInstanced(GL_TRIANGLES, motionblurSamples);

    glUniform1i(u_partID, 0);
    m_crankshaftMesh->renderInstanced(GL_TRIANGLES, motionblurSamples);

    ems::setColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_engine.crankSpeed = -(m_engine.crankAngle - prevCrankAngle) / delta;
    prevCrankAngle = m_engine.crankAngle;
    m_prevTime = time;
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