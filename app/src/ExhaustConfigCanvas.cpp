#include <engmsc-app/ExhaustConfigCanvas.hpp>

#include <glad/glad.h>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static const char* vecShaderCode = 
    "#version 330\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec4 aColor;\n"
    "out vec4 color;\n"
    "uniform mat4 u_projMatrix;\n"
    "uniform mat4 u_modlMatrix;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   gl_Position = u_projMatrix * u_modlMatrix * vec4(aPos, 1.0);\n"
    "   color = aColor;"
    "}\n"
    ;

static const char* fragShaderCode = 
    "#version 330\n"
    "in vec4 color;\n"
    "out vec4 FragColor;"
    "\n"
    "void main()\n"
    "{\n"
    "   FragColor = color;\n"
    "}\n"
    ;

static bool initShaders = false;
static GLuint shaderProgram = 0;
static GLuint u_projMatrix;
static GLuint u_modlMatrix;

static GLuint meshVAO = 0;
static void setupShaders();
static void setupMeshes();

struct Mesh
{
    GLuint vao;
    GLuint indexCount;
    GLuint vertCount;
};
static Mesh cyl_markerMesh;
static Mesh offset_markerMesh;
static Mesh cyl_marker_highlightedMesh;

static GLuint backgroundVAO;
static GLuint backgroundBuffer;

ExhaustConfigCanvas::ExhaustConfigCanvas(nanogui::Widget* parent) :
    nanogui::Canvas(parent)
{
    if(!initShaders)
    {
        setupShaders();
        setupMeshes();
    }
}

void ExhaustConfigCanvas::draw_contents()
{
    const nanogui::Vector2i thisSize = size();
    float sizeX = static_cast<float>(thisSize.x());
    float sizeY = static_cast<float>(thisSize.y());
    glm::mat4 projMatrix = glm::ortho(0.0f, sizeX, sizeY, 0.0f);
    glm::mat4 modlMatrix = glm::mat4(1.0f);

    float backgroundMesh[] =
    {
         0.0f, sizeY,
        sizeX, sizeY,
        sizeX,  0.0f,
         0.0f,  0.0f
    };

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindBuffer(GL_ARRAY_BUFFER, backgroundBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(backgroundMesh), backgroundMesh);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(u_projMatrix, 1, false, glm::value_ptr(projMatrix));
    glUniformMatrix4fv(u_modlMatrix, 1, false, glm::value_ptr(modlMatrix));

    //Render background gradient
    glBindVertexArray(backgroundVAO);
    {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, NULL);
    }

    //Render offset markers
    for(int i = 1; i <= m_nbCylinders; i++)
    {
        float x = (sizeX / (m_nbCylinders + 1)) * 0.5f + (sizeX / (m_nbCylinders + 1)) * (i - 1);
        x += (sizeX / (m_nbCylinders + 1)) * (m_offsets[i - 1] * 0.5f + 0.5f);
        modlMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, sizeY, 0.0f));
        modlMatrix = glm::scale(modlMatrix, glm::vec3(sizeY, -sizeY, 1.0f));

        glUniformMatrix4fv(u_modlMatrix, 1, false, glm::value_ptr(modlMatrix));
        glBindVertexArray(offset_markerMesh.vao);
        {
            glDrawElements(GL_TRIANGLES, offset_markerMesh.indexCount, GL_UNSIGNED_INT, NULL);
        }
    }

    //Render cyl markers
    for(int i = 1; i <= m_nbCylinders; i++)
    {
        float x = (sizeX / (m_nbCylinders + 1)) * i;
        modlMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, sizeY, 0.0f));
        modlMatrix = glm::scale(modlMatrix, glm::vec3(sizeY, -sizeY, 1.0f));

        glUniformMatrix4fv(u_modlMatrix, 1, false, glm::value_ptr(modlMatrix));
        Mesh* mesh = i == m_selectedCylinder ? &cyl_marker_highlightedMesh : &cyl_markerMesh;
        glBindVertexArray(mesh->vao);
        {
            glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, NULL);
        }
    }
}

bool ExhaustConfigCanvas::mouse_button_event(const nanogui::Vector2i& pos, int button, bool down, int modifiers)
{
    float sizeX = static_cast<float>(size().x());

    for(int i = 0; i < m_nbCylinders; i++)
    {
        float boundsX0 = (sizeX / (m_nbCylinders + 1)) * 0.5f + (sizeX / (m_nbCylinders + 1)) * i;
        float boundsX1 = boundsX0 + (sizeX / (m_nbCylinders + 1));
        if(boundsX0 < pos.x() && pos.x() < boundsX1)
        {
            m_selectedCylinder = i + 1;
            if(m_callback) m_callback();
            return true;
        }
    }

    return false;
}

void ExhaustConfigCanvas::setNbCylinders(int cyls)
{
    m_nbCylinders = cyls;
    if(m_selectedCylinder > cyls)
    {
        m_selectedCylinder = cyls;
    }
}

void ExhaustConfigCanvas::setOffsets(const float* offsets, int length)
{
    for(int i = 0; i < length; i++)
    {
        m_offsets[i] = offsets[i];
    }
}

void ExhaustConfigCanvas::setCallback(std::function<void()> callback)
{
    m_callback = callback;
}

int ExhaustConfigCanvas::getSelectedCylinder() const
{
    return m_selectedCylinder;
}

#include <iostream>

void setupShaders()
{
    char log[512];

    GLuint vecShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vecShader, 1, &vecShaderCode, 0);
    glShaderSource(fragShader, 1, &fragShaderCode, 0);

    GLint status = 0;
    glCompileShader(vecShader);
    glGetShaderiv(vecShader, GL_COMPILE_STATUS, &status);
    if(!status)
    {
        glGetShaderInfoLog(vecShader, sizeof(log), 0, log);
        std::cerr << "[Vertex Shader] [ERROR]: \n" << log << std::endl;
        throw std::runtime_error("Shader compilation error");
    }

    memset(log, 0, sizeof(log));
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &status);
    if(!status)
    {
        glGetShaderInfoLog(fragShader, sizeof(log), 0, log);
        std::cerr << "[Fragment Shader] [ERROR]: \n" << log << std::endl;
        throw std::runtime_error("Shader compilation error");
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vecShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);

    memset(log, 0, sizeof(log));
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if(!status)
    {
        glGetProgramInfoLog(shaderProgram, sizeof(log), 0, log);
        std::cerr << "[Shader Program] [ERROR]: \n" << log << std::endl;
        throw std::runtime_error("Program linking error");
    }

    u_projMatrix = glGetUniformLocation(shaderProgram, "u_projMatrix");
    u_modlMatrix = glGetUniformLocation(shaderProgram, "u_modlMatrix");

    initShaders = true;
}

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void loadMesh(Mesh& p_mesh, const char* fileName)
{
    Assimp::Importer importer;
    GLuint buffers[3];

    const aiScene* cyl_marker = importer.ReadFile(fileName, aiProcess_Triangulate);

    const aiMesh* mesh = cyl_marker->mMeshes[0];
    p_mesh.indexCount = mesh->mNumFaces * 3;
    p_mesh.vertCount = mesh->mNumVertices;
    GLuint* indicies = new GLuint[p_mesh.indexCount];
    for(int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace* face = &mesh->mFaces[i];
        indicies[i * 3] = face->mIndices[0];
        indicies[i * 3 + 1] = face->mIndices[1];
        indicies[i * 3 + 2] = face->mIndices[2];
    }
    glGenBuffers(3, buffers);
    glGenVertexArrays(1, &p_mesh.vao);
    glBindVertexArray(p_mesh.vao);
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
        glBufferData(GL_ARRAY_BUFFER, p_mesh.vertCount * sizeof(aiVector3D), mesh->mVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(aiVector3D), (void*) 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
        glBufferData(GL_ARRAY_BUFFER, p_mesh.vertCount * sizeof(aiColor4D), mesh->mColors[0], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(aiColor4D), (void*) 0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, p_mesh.indexCount * sizeof(GLuint), indicies, GL_STATIC_DRAW);
    }
    delete indicies;
}

#define GRAD_0 180
#define GRAD_1 240

void setupMeshes()
{
    Assimp::Importer importer;
    GLuint buffers[3];

    //Background gradient
    glGenBuffers(2, buffers);

    GLubyte colors[] =
    {
        GRAD_0, GRAD_0, GRAD_0,
        GRAD_0, GRAD_0, GRAD_0,
        GRAD_1, GRAD_1, GRAD_1,
        GRAD_1, GRAD_1, GRAD_1
    };

    GLubyte indicies0[] = { 0, 1, 2, 0, 2, 3 };

    glGenBuffers(1, &backgroundBuffer);

    glGenVertexArrays(1, &backgroundVAO);
    glBindVertexArray(backgroundVAO);
    {
        glBindBuffer(GL_ARRAY_BUFFER, backgroundBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, NULL, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, true, 3, (void*)0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies0), indicies0, GL_STATIC_DRAW);
    }

    loadMesh(cyl_markerMesh, "rsc/mesh/cyl_marker.ply");
    loadMesh(offset_markerMesh, "rsc/mesh/offset_marker.ply");
    loadMesh(cyl_marker_highlightedMesh, "rsc/mesh/cyl_marker_highlighted.ply");
}