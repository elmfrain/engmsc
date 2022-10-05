#include "engmsc/Engine2DRenderer.hpp"

#include "Mesh.hpp"
#include "Logger.hpp"
#include "UIRender.hpp"
#include "Shaders.hpp"

#include <glm/gtx/transform.hpp>

#define HALF_PI glm::half_pi<float>()
#define PI glm::pi<float>()
#define TWO_PI glm::two_pi<float>()
#define THREE_HALF_PI glm::three_over_two_pi<float>()

// Note: any name with prefix "u_" indicates that it's a variable used for shaders
// Note: any function with prefix "i_" indicates that it is a static private method

static EMLogger m_logger("Engine2D Renderer");

// Used for sending engine profile to shaders (contains uniform locations)
struct u_EngineProfile
{
    u_EngineProfile() {}

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

    void sendUniform(EMEngineCylinder& cyl)
    {
        glUniform1f(stroke, cyl.stroke);
        glUniform1f(bore, cyl.bore);
        glUniform1f(rodLength, cyl.rodLength);
        glUniform1f(camIntakeAngle, cyl.camIntakeAngle);
        glUniform1f(camIntakeLift, cyl.camIntakeLift);
        glUniform1f(camIntakeDuration, cyl.camIntakeDuration);
        glUniform1f(camExhaustAngle, cyl.camExhaustAngle);
        glUniform1f(camExhaustLift, cyl.camExhaustLift);
        glUniform1f(camExhaustDuration, cyl.camExhaustDuration);
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

struct CylinderRenderInfo
{
    std::unique_ptr<EMMeshBuilder> intakeCamMesh;
    std::unique_ptr<EMMeshBuilder> exhaustCamMesh;
    std::unique_ptr<EMMeshBuilder> conrodMesh;
    float cylHeadHeight;
};

// Static meshes
static EMVertexFormat m_vtxFmt;
static EMMesh::Ptr m_crankshaftMesh;
static EMMesh::Ptr m_conrodBaseMesh;
static EMMesh::Ptr m_pistonMesh;
static EMMesh::Ptr m_cylheadMesh;
static EMMesh::Ptr m_intakeValveMesh;
static EMMesh::Ptr m_exhaustValveMesh;

// Shader Uniforms
static u_EngineProfile u_profile;
static int u_crankAngle = 0;
static int u_crankAngleDelta = 0;
static int u_numInstances = 0;
static int u_partID = 0;

static bool m_hasInit = false;
static double m_prevTime = 0.0;

// References to inputs
static EMWindow* m_window;
static const EMKeyboard* m_keyboard;
static const EMMouse* m_mouse;

// Engine Reference
static EMEngineAssembly* m_engineAssembly = NULL;
static std::vector<CylinderRenderInfo> m_cylMeshes;

// Camera
static glm::vec2 m_camPos(0.0f);
static float m_zoom = 1.0f;

// Settings
static int m_motionblurSamples = 30;
static float m_motionblurAmount = 0.025f;

static void i_updateCamFromInputs();
static void i_genConrodMesh(EMEngineCylinder& cyl, std::unique_ptr<EMMeshBuilder>& meshBuilder);
static void i_genCamMesh(EMEngineCylinder& cyl, std::unique_ptr<EMMeshBuilder>& meshBuilder,
                         float base, float lift, float duration);
static void i_renderCylHeadAssembly(EMEngineCylinder& cyl, CylinderRenderInfo& cylInfo);

void EMEngine2DRenderer::init()
{
    if(m_hasInit) return;

    m_vtxFmt.size = 2;
    m_vtxFmt[0].data = EMVF_ATTRB_USAGE_POS
                   | EMVF_ATTRB_SIZE(3)
                   | EMVF_ATTRB_TYPE_FLOAT
                   | EMVF_ATTRB_NORMALIZED_FALSE;
    m_vtxFmt[1].data = EMVF_ATTRB_USAGE_COLOR
                   | EMVF_ATTRB_SIZE(4)
                   | EMVF_ATTRB_TYPE_FLOAT
                   | EMVF_ATTRB_NORMALIZED_FALSE;

    // Load and prepare static meshes
    m_crankshaftMesh = EMMesh::load("res/engine2D/crankshaft.ply")[0];
    m_conrodBaseMesh = EMMesh::load("res/engine2D/conrod.ply")[0];
    m_pistonMesh = EMMesh::load("res/engine2D/piston.ply")[0];
    m_cylheadMesh = EMMesh::load("res/engine2D/cylhead.ply")[0];
    m_intakeValveMesh = EMMesh::load("res/engine2D/intake-valve.ply")[0];
    m_exhaustValveMesh = EMMesh::load("res/engine2D/exhaust-valve.ply")[0];
    m_crankshaftMesh->makeRenderable(m_vtxFmt);
    m_conrodBaseMesh->makeRenderable(m_vtxFmt);
    m_pistonMesh->makeRenderable(m_vtxFmt);
    m_cylheadMesh->makeRenderable(m_vtxFmt);
    m_intakeValveMesh->makeRenderable(m_vtxFmt);
    m_exhaustValveMesh->makeRenderable(m_vtxFmt);

    // Get shader uniform locations
    ems::ENGINE2D_shader();
    int shader = ems::getProgramID();
    u_crankAngle = glGetUniformLocation(shader, "u_crankAngle");
    u_crankAngleDelta = glGetUniformLocation(shader, "u_crankAngleDelta");
    u_numInstances = glGetUniformLocation(shader, "u_numInstances");
    u_partID = glGetUniformLocation(shader, "u_partID");
    u_profile = u_EngineProfile(shader);

    // Get input references
    m_window = &emui::getWindow();
    m_keyboard = &m_window->getKeyboard();
    m_mouse = &m_window->getMouse();

    m_hasInit = true;
    m_logger.infof("Initialized Module");
}

void EMEngine2DRenderer::setEngineAssembly(EMEngineAssembly& engine)
{
    m_cylMeshes.clear();

    m_engineAssembly = &engine;

    for(EMEngineCylinder& cyl : engine.cylinders)
    {
        CylinderRenderInfo& back = m_cylMeshes.emplace_back();

        i_genConrodMesh(cyl, back.conrodMesh);
        i_genCamMesh(cyl, back.intakeCamMesh, 0.1f, cyl.camIntakeLift / cyl.bore, cyl.camIntakeDuration);
        i_genCamMesh(cyl, back.exhaustCamMesh, 0.1f, cyl.camExhaustLift / cyl.bore, cyl.camExhaustDuration);

        // Note: Piston pin height is always 0.3 * bore (corrisponds to mesh)

        back.cylHeadHeight = cyl.stroke / 2.0f + cyl.rodLength + 0.3f * cyl.bore + cyl.gasketHeight + cyl.deckClearance;
    }
}

void EMEngine2DRenderer::render()
{
    i_updateCamFromInputs();

    if(!m_engineAssembly) return;

    const double time = glfwGetTime();
    const float delta = float(time - m_prevTime);
    const float crankDelta =
    glm::max(-1.5f, glm::min(m_motionblurAmount * (float) m_engineAssembly->crankSpeed, 1.5f));

    float viewRatio = emui::getUIWidth() * m_zoom / emui::getUIHeight();
    glm::mat4 projection = glm::ortho(-viewRatio, viewRatio, -m_zoom, m_zoom, -500.0f, 500.0f);
    projection = glm::translate(projection, {-m_camPos.x, -m_camPos.y, 0.0f});
    glm::mat4 modelview = glm::mat4(1.0f);

    ems::setProjectionMatrix(projection);
    ems::setModelviewMatrix(modelview);
    ems::ENGINE2D_shader();
    glUniform1f(u_crankAngleDelta, crankDelta);
    glUniform1f(u_numInstances, (float) m_motionblurSamples);

    int numCylinders = (int) m_engineAssembly->cylinders.size();
    for(int i = numCylinders - 1; i >= 0; i--)
    {  
        EMEngineCylinder& cyl = m_engineAssembly->cylinders[i];
        CylinderRenderInfo& cylMeshes = m_cylMeshes[i];

        modelview = glm::rotate((float) -cyl.bankAngle, glm::vec3(0.0, 0.0, 1.0f));
        ems::setModelviewMatrix(modelview);

        glUniform1f(
            u_crankAngle, (float) glm::mod(m_engineAssembly->crankAngle + cyl.angleOffset, 2.0 * glm::two_pi<double>()));
        u_profile.sendUniform(cyl);

        i_renderCylHeadAssembly(cyl, cylMeshes);
        ems::setModelviewMatrix(modelview);

        glUniform1i(u_partID, 2);
        cylMeshes.conrodMesh->drawElemenentsInstanced(GL_TRIANGLES, m_motionblurSamples);

        glUniform1i(u_partID, 1);
        m_pistonMesh->renderInstanced(GL_TRIANGLES, m_motionblurSamples);

        glUniform1i(u_partID, 0);
        m_crankshaftMesh->renderInstanced(GL_TRIANGLES, m_motionblurSamples);
    }

    ems::setColor(glm::vec4(1.0f));
    m_prevTime = time;
}

static double m_resetCooldown = 0.0;
static void i_updateCamFromInputs()
{
    float viewRatio = emui::getUIWidth() / emui::getUIHeight();

    if(m_mouse->buttonJustPressed(GLFW_MOUSE_BUTTON_1))
    {
        double time = glfwGetTime();
        if(time - m_resetCooldown < 0.25)
        {
            m_camPos = glm::vec2(0.0f, 0.0f);
            m_zoom = 1.0f;
        }
        m_resetCooldown = time;
    }

    if(m_mouse->isButtonPressed(GLFW_MOUSE_BUTTON_1))
    {
        m_camPos.x -= viewRatio * 2.0f * m_mouse->cursorXDelta() * m_zoom / m_window->getWidth();
        m_camPos.y += 2.0f * m_mouse->cursorYDelta() * m_zoom / m_window->getHeight();
    }

    if(m_mouse->justScrolled())
    {
        m_zoom *= m_mouse->scrollDeltaY() < 0 ? 1.2f : (1.0f / 1.2f);
    }
}

static void i_genConrodMesh(EMEngineCylinder& cyl, std::unique_ptr<EMMeshBuilder>& meshBuilder)
{
    meshBuilder = std::make_unique<EMMeshBuilder>(m_vtxFmt);
    const glm::vec3* positions = m_conrodBaseMesh->getPositions();
    const glm::vec2* uvs = m_conrodBaseMesh->getUVs();
    const glm::vec4* colors = m_conrodBaseMesh->getColors();

    if(!uvs)
    {
        m_conrodBaseMesh->putMeshElements(*meshBuilder);
        return;
    }

    meshBuilder->indexv(m_conrodBaseMesh->numIndicies(), m_conrodBaseMesh->getIndicies());

    glm::mat4* modelview = &meshBuilder->pushMatrix();

    glm::mat4 bigEnd = glm::scale(*modelview, {cyl.stroke, cyl.stroke, 1.0f});
    glm::mat4 smallEnd = glm::translate(*modelview, {0.0f, cyl.rodLength, 0.0f});
    smallEnd = glm::scale(smallEnd, {cyl.bore, cyl.bore, 1.0f});

    int numVerticies = (int) m_conrodBaseMesh->numVerticies();
    for(int i = 0; i < numVerticies; i++)
    {
        int group = uvs[i].x < 0.5f ? 0 : 1;

        if(group == 0) *modelview = bigEnd;
        else *modelview = smallEnd;

        meshBuilder->position(positions[i].x, positions[i].y, positions[i].z);
        if(!colors) meshBuilder->colorDefault();
        else meshBuilder->colorRGBA(colors[i].r, colors[i].g, colors[i].b, colors[i].a);
    }

    meshBuilder->popMatrix();
}

static void i_genCamMesh(EMEngineCylinder& cyl, std::unique_ptr<EMMeshBuilder>& meshBuilder,
                         float base, float lift, float duration)
{
    meshBuilder = std::make_unique<EMMeshBuilder>(m_vtxFmt);

    const int resolution = 64;
    const float segArc = TWO_PI / resolution;
    float angle = 0.0f;

    const auto camProfile = [base, lift, duration] (float a) -> float
    {
        float x = base;
        if(PI - 0.5f * duration < a && a < PI + 0.5f * duration)
        { x = glm::cos(TWO_PI * (a - PI) / duration) * 0.5f * lift + 0.5f * lift + base; }
        return x;
    };

    glm::mat4* modelview = &meshBuilder->pushMatrix();

    for(int s = 0; s < resolution; s++)
    {
        meshBuilder->index(6, 0, 1, 2, 0, 2, 3);

        float x = camProfile(angle);

        meshBuilder->
         vertex(NULL, 0.0f,    x, 0.0f, 0.7f, 0.7f, 0.7f, 1.0f)
        .vertex(NULL, 0.0f, 0.0f, 0.0f, 0.7f, 0.7f, 0.7f, 1.0f);

        *modelview = glm::rotate(*modelview, segArc, {0, 0, 1});
        angle += segArc;

        x = camProfile(angle);

        meshBuilder->
         vertex(NULL, 0.0f, 0.0f, 0.0f, 0.7f, 0.7f, 0.7f, 1.0f)
        .vertex(NULL, 0.0f,    x, 0.0f, 0.7f, 0.7f, 0.7f, 1.0f);
    }

    meshBuilder->popMatrix();
}

static void i_renderCylHeadAssembly(EMEngineCylinder& cyl, CylinderRenderInfo& cylInfo)
{
    glm::mat4 modelview = ems::getModelviewMatrix();
    modelview = 
    glm::translate(modelview, {0.0f, cylInfo.cylHeadHeight, 0.0f});
    modelview = glm::scale(modelview, {cyl.headFlipped ? -cyl.bore : cyl.bore, cyl.bore, 1.0f});

    ems::setModelviewMatrix(modelview);
    ems::setColor(1.0f, 1.0f, 1.0f, 0.8f);

    glUniform1i(u_partID, - 1);
    m_cylheadMesh->render(GL_TRIANGLES);

    ems::setColor(1.0f, 1.0f, 1.0f, 0.1f);

    glm::mat4 mvIntakeV = glm::translate(modelview, {-0.2069f, 0.0773f, 0.0f});
    mvIntakeV = glm::rotate(mvIntakeV, 0.1745f, {0, 0, 1});
    ems::setModelviewMatrix(mvIntakeV);
    glUniform1i(u_partID, 3);
    m_intakeValveMesh->renderInstanced(GL_TRIANGLES, m_motionblurSamples);

    glm::mat4 mvExhaustV = glm::translate(modelview, {0.2158f, 0.0753f, 0.0f});
    mvExhaustV = glm::rotate(mvExhaustV, -0.1745f, {0, 0, 1});
    ems::setModelviewMatrix(mvExhaustV);
    glUniform1i(u_partID, 4);
    m_exhaustValveMesh->renderInstanced(GL_TRIANGLES, m_motionblurSamples);

    glm::mat4 mvIntakeCam = glm::translate(mvIntakeV, {0.0f, 0.93f, 0.0f});
    ems::setModelviewMatrix(mvIntakeCam);
    glUniform1i(u_partID, 5);
    cylInfo.intakeCamMesh->drawElemenentsInstanced(GL_TRIANGLES, m_motionblurSamples);

    glm::mat4 mvExhaustCam = glm::translate(mvExhaustV, {0.0f, 0.93f, 0.0f});
    ems::setModelviewMatrix(mvExhaustCam);
    glUniform1i(u_partID, 6);
    cylInfo.exhaustCamMesh->drawElemenentsInstanced(GL_TRIANGLES, m_motionblurSamples);
}