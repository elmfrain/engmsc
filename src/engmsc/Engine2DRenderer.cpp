#include "engmsc/Engine2DRenderer.hpp"

#include "Mesh.hpp"
#include "Shaders.hpp"
#include "UIRender.hpp"
#include "Logger.hpp"

#include <glm/gtx/transform.hpp>

#define HALF_PI glm::half_pi<float>()
#define PI glm::pi<float>()
#define TWO_PI glm::two_pi<float>()
#define THREE_HALF_PI glm::three_over_two_pi<float>()

// Note: any name with prefirx "u_" indicates that it's a variable used for shaders

struct u_EngineProfile
{
    u_EngineProfile()
    {
    }

    u_EngineProfile(int program)
    {
        stroke =             glGetUniformLocation(program, "u_profile.stroke");
        bore =               glGetUniformLocation(program, "u_profile.bore");
        rodLength =          glGetUniformLocation(program, "u_profile.rodLength");
        camIntakeAngle =     glGetUniformLocation(program, "u_profile.camIntakeAngle");
        camIntakeLift =      glGetUniformLocation(program, "u_profile.camIntakeLift");
        camIntakeDuration =  glGetUniformLocation(program, "u_profile.camIntakeDuration");
        camExhaustAngle =    glGetUniformLocation(program, "u_profile.camExhaustAngle");
        camExhaustLift =     glGetUniformLocation(program, "u_profile.camExhaustLift");
        camExhaustDuration = glGetUniformLocation(program, "u_profile.camExhaustDuration");
    }

    void uniformProfile(EMEngine::Profile& profile)
    {
        glUniform1f(stroke, profile.stroke);
        glUniform1f(bore, profile.bore);
        glUniform1f(rodLength, profile.rodLength);
        glUniform1f(camIntakeAngle, profile.camIntakeAngle);
        glUniform1f(camIntakeLift, profile.camIntakeLift);
        glUniform1f(camIntakeDuration, profile.camIntakeDuration);
        glUniform1f(camExhaustAngle, profile.camExhaustAngle);
        glUniform1f(camExhaustLift, profile.camExhaustLift);
        glUniform1f(camExhaustDuration, profile.camExhaustDuration);
    }

    int stroke;
    int bore;
    int rodLength;
    int camIntakeAngle;
    int camIntakeLift;
    int camIntakeDuration;
    int camExhaustAngle;
    int camExhaustLift;
    int camExhaustDuration;
};

static EMMesh::Ptr m_crankshaftMesh;
static EMMesh::Ptr m_conrodBaseMesh;
static EMMesh::Ptr m_pistonMesh;
static EMMesh::Ptr m_cylheadMesh;
static EMMesh::Ptr m_intakeValveMesh;
static EMMesh::Ptr m_exhaustValveMesh;
static std::unique_ptr<EMMeshBuilder> m_intakeCamMesh;
static std::unique_ptr<EMMeshBuilder> m_exhaustCamMesh;
static std::unique_ptr<EMMeshBuilder> m_conrodMesh;
static EMEngine m_engine;
static double prevCrankAngle = 0.0;
static double m_prevTime = 0.0;
static u_EngineProfile u_profile;
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
static void i_genConrodMesh(EMVertexFormat& vtxFmt);
static void i_genCamMesh(EMVertexFormat& vtxFmt);
static void i_renderCylheadAssembly(int instances);

// Default profile
EMEngine::Profile::Profile() :
    stroke(1.0f),
    bore(1.0f),
    rodLength(1.2f),
    camIntakeAngle(-1.004f),
    camIntakeLift(0.068f),
    camIntakeDuration(2.217f),
    camExhaustAngle(1.004f),
    camExhaustLift(0.068f),
    camExhaustDuration(2.217f)
{
}

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

    m_engine.crankAngle = 0.0;
    m_engine.crankSpeed = 0.0;

    m_crankshaftMesh = EMMesh::load("res/engine2D/crankshaft.ply")[0];
    m_conrodBaseMesh = EMMesh::load("res/engine2D/conrod.ply")[0];
    m_pistonMesh = EMMesh::load("res/engine2D/piston.ply")[0];
    m_cylheadMesh = EMMesh::load("res/engine2D/cylhead.ply")[0];
    m_intakeValveMesh = EMMesh::load("res/engine2D/intake-valve.ply")[0];
    m_exhaustValveMesh = EMMesh::load("res/engine2D/exhaust-valve.ply")[0];

    m_crankshaftMesh->makeRenderable(vtxFmt);
    m_conrodBaseMesh->makeRenderable(vtxFmt);
    m_pistonMesh->makeRenderable(vtxFmt);
    m_cylheadMesh->makeRenderable(vtxFmt);
    m_intakeValveMesh->makeRenderable(vtxFmt);
    m_exhaustValveMesh->makeRenderable(vtxFmt);

    i_genConrodMesh(vtxFmt);
    i_genCamMesh(vtxFmt);

    ems::ENGINE2D_shader();
    int shader = ems::getProgramID();
    u_crankAngle = glGetUniformLocation(shader, "u_crankAngle");
    u_crankAngleDelta = glGetUniformLocation(shader, "u_crankAngleDelta");
    u_numInstances = glGetUniformLocation(shader, "u_numInstances");
    u_partID = glGetUniformLocation(shader, "u_partID");
    u_profile = u_EngineProfile(shader);

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
    const float crankDelta = glm::min(0.3f, float(m_engine.crankAngle - prevCrankAngle));

    float viewRatio = emui::getUIWidth() / emui::getUIHeight();
    glm::mat4 projection = glm::ortho(-viewRatio, viewRatio, -1.0f, 1.0f, -500.0f, 500.0f);
    glm::mat4 modelview = glm::mat4(1.0f);
    projection = glm::translate(projection, {-m_camPos.x, -m_camPos.y, 0.0f});
    projection = glm::scale(projection, {m_zoom, m_zoom, 1.0f});

    ems::setProjectionMatrix(projection);
    ems::setModelviewMatrix(modelview);
    ems::ENGINE2D_shader();
    glUniform1f(
        u_crankAngle, (float) glm::mod(m_engine.crankAngle, 2.0 * glm::two_pi<double>()));
    glUniform1f(u_crankAngleDelta, motionblurAmount * crankDelta / delta);
    glUniform1f(u_numInstances, motionblurSamples);
    u_profile.uniformProfile(m_engine.profile);
 
    i_renderCylheadAssembly(motionblurSamples);

    ems::setModelviewMatrix(modelview);
    ems::ENGINE2D_shader();
    glUniform1i(u_partID, 2);
    m_conrodMesh->drawElemenentsInstanced(GL_TRIANGLES, motionblurSamples);

    glUniform1i(u_partID, 1);
    m_pistonMesh->renderInstanced(GL_TRIANGLES, motionblurSamples);

    glUniform1i(u_partID, 0);
    m_crankshaftMesh->renderInstanced(GL_TRIANGLES, motionblurSamples);

    ems::setColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_engine.crankSpeed = (m_engine.crankAngle - prevCrankAngle) / delta;
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

static void i_genConrodMesh(EMVertexFormat& vtxFmt)
{
    m_conrodMesh = std::make_unique<EMMeshBuilder>(vtxFmt);
    const glm::vec3* positions = m_conrodBaseMesh->getPositions();
    const glm::vec2* uvs = m_conrodBaseMesh->getUVs();
    const glm::vec4* colors = m_conrodBaseMesh->getColors();

    if(!uvs)
    {
        m_conrodBaseMesh->putMeshElements(*m_conrodMesh);
        return;
    }

    m_conrodMesh->indexv(m_conrodBaseMesh->numIndicies(), m_conrodBaseMesh->getIndicies());

    glm::mat4* modelview = &m_conrodMesh->pushMatrix();

    glm::mat4 bigEnd = glm::scale(*modelview, {m_engine.profile.stroke, m_engine.profile.stroke, 1.0f});
    glm::mat4 smallEnd = glm::translate(*modelview, {0.0f, m_engine.profile.rodLength, 0.0f});
    smallEnd = glm::scale(smallEnd, {m_engine.profile.bore, m_engine.profile.bore, 1.0f}); 

    int numVerticies = (int) m_conrodBaseMesh->numVerticies();
    for(int i = 0; i < numVerticies; i++)
    {
        int group = uvs[i].x < 0.5f ? 0 : 1;

        if(group == 0) *modelview = bigEnd;
        else *modelview = smallEnd;

        m_conrodMesh->position(positions[i].x, positions[i].y, positions[i].z);
        if(!colors) m_conrodMesh->colorDefault();
        else m_conrodMesh->colorRGBA(colors[i].r, colors[i].g, colors[i].b, colors[i].a);
    }

    m_conrodMesh->popMatrix();
}

static void i_genSingleCamMesh(EMMeshBuilder& meshBuilder, float base, float lift, float duration)
{
    const int segments = 64;
    const float segArc = TWO_PI / segments;
    float angle = 0.0f;

    glm::mat4* modelview = &meshBuilder.pushMatrix();

    for(int s = 0; s < segments; s++)
    {
        meshBuilder.index(6, 0, 1, 2, 0, 2, 3);

        float x = base;
        if(PI - 0.5f * duration < angle && angle < PI + 0.5f * duration)
        { x = glm::cos(TWO_PI * (angle - PI) / duration) * 0.5f * lift + 0.5f * lift + base; }

        meshBuilder
        .vertex(NULL, 0.0f,    x, 0.0f, 0.7f, 0.7f, 0.7f, 1.0f)
        .vertex(NULL, 0.0f, 0.0f, 0.0f, 0.7f, 0.7f, 0.7f, 1.0f);

        *modelview = glm::rotate(*modelview, segArc, {0, 0, 1});
        angle += segArc;

        x = base;
        if(PI - 0.5f * duration < angle && angle < PI + 0.5f * duration)
        { x = glm::cos(TWO_PI * (angle - PI) / duration) * 0.5f * lift + 0.5f * lift + base; }

        meshBuilder
        .vertex(NULL, 0.0f, 0.0f, 0.0f, 0.7f, 0.7f, 0.7f, 1.0f)
        .vertex(NULL, 0.0f,    x, 0.0f, 0.7f, 0.7f, 0.7f, 1.0f);
    }

    meshBuilder.popMatrix();
}

static void i_genCamMesh(EMVertexFormat& vtxFmt)
{
    m_intakeCamMesh = std::make_unique<EMMeshBuilder>(vtxFmt);
    m_exhaustCamMesh = std::make_unique<EMMeshBuilder>(vtxFmt);
    
    float intakeLift = m_engine.profile.camIntakeLift / m_engine.profile.bore;
    float exhaustLift = m_engine.profile.camExhaustLift / m_engine.profile.bore;

    i_genSingleCamMesh(
        *m_intakeCamMesh, 0.1f, intakeLift, m_engine.profile.camIntakeDuration);
    i_genSingleCamMesh(
        *m_exhaustCamMesh, 0.1f, exhaustLift, m_engine.profile.camExhaustDuration);
}

static void i_renderCylheadAssembly(int instances)
{
    float scale = m_engine.profile.bore;
    glm::mat4 modelview = glm::mat4(1.0f);
    modelview = glm::translate(modelview, 
    {0.0f, m_engine.profile.stroke / 2.0f + m_engine.profile.rodLength + 0.3f * scale + 0.03f, 0.0f});
    modelview = glm::scale(modelview, {scale, scale, 1.0f});

    ems::setModelviewMatrix(modelview);
    ems::setColor(1.0f, 1.0f, 1.0f, 0.8f);
    ems::ENGINE2D_shader();

    glUniform1i(u_partID, -1);
    m_cylheadMesh->render(GL_TRIANGLES);

    ems::setColor(1.0f, 1.0f, 1.0f, 0.1f);

    glm::mat4 mvIntakeV = glm::translate(modelview, {-0.2069f, 0.0773f, 0.0f});
    mvIntakeV = glm::rotate(mvIntakeV, 0.1745f, {0, 0, 1});
    ems::setModelviewMatrix(mvIntakeV);
    ems::ENGINE2D_shader();
    glUniform1i(u_partID, 3);
    m_intakeValveMesh->renderInstanced(GL_TRIANGLES, instances);

    glm::mat4 mvExhaustV = glm::translate(modelview, {0.2158f, 0.0753f, 0.0f});
    mvExhaustV = glm::rotate(mvExhaustV, -0.1745f, {0, 0, 1});
    ems::setModelviewMatrix(mvExhaustV);
    ems::ENGINE2D_shader();
    glUniform1i(u_partID, 4);
    m_exhaustValveMesh->renderInstanced(GL_TRIANGLES, instances);

    glm::mat4 mvIntakeCam = glm::translate(mvIntakeV, {0.0f, 0.93f, 0.0f});
    ems::setModelviewMatrix(mvIntakeCam);
    ems::ENGINE2D_shader();
    glUniform1i(u_partID, 5);
    m_intakeCamMesh->drawElemenentsInstanced(GL_TRIANGLES, instances);

    glm::mat4 mvExhaustCam = glm::translate(mvExhaustV, {0.0f, 0.93f, 0.0f});
    ems::setModelviewMatrix(mvExhaustCam);
    ems::ENGINE2D_shader();
    glUniform1i(u_partID, 6);
    m_exhaustCamMesh->drawElemenentsInstanced(GL_TRIANGLES, instances);
}