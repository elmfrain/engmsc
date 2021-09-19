#include <engmsc-app/FlywheelRenderer.hpp>
#include <glad/glad.h>
#include <iostream>
#include <cstring>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static const char* vecShaderCode = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec4 aColor;\n"
    "\n"
    "out vec4 color;"
    "\n"
    "uniform mat4 u_projection;\n"
    "uniform mat4 u_model;\n"
    "uniform float u_numInstances;\n"
    "uniform float u_angle;\n"
    "uniform float u_angleSpeed;\n"
    "\n"
    "mat4 rotationMatrix(vec3 axis, float angle)\n"
    "{\n"
    "   axis = normalize(axis);\n"
    "   float s = sin(angle);\n"
    "   float c = cos(angle);\n"
    "   float oc = 1.0 - c;\n"
    "   \n"
    "   return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,\n"
    "               oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,\n"
    "               oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,\n"
    "               0.0,                                0.0,                                0.0,                                1.0);\n"
    "}\n"
    "void main()\n"
    "{\n"
    "   float lerp = float(gl_InstanceID) / u_numInstances;\n"
    "   mat4 model = u_model * rotationMatrix(vec3(0.0, 0.0, 1.0), u_angle + u_angleSpeed * lerp );\n"
    "   gl_Position = u_projection * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "   color = aColor;"
    "}\n";
static const char* fragShaderCode =
    "#version 330 core\n"
    "in vec4 color;\n"
    "out vec4 FragColor;\n"
    "\n"
    "uniform float u_numInstances;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   FragColor = color / u_numInstances;\n"
    "}\n";

static GLsizei meshSize = 0;
static GLsizei meshIndexSize = 0;
static GLuint meshVAO = 0;

static GLuint flywheelFBO;

static GLuint u_projection = 0;
static GLuint u_model = 0;
static GLuint u_numInstances = 0;
static GLuint u_angle = 0;
static GLuint u_angleSpeed = 0;
static GLuint shaderProgram = 0;

static void setupShaders();
static void setupMesh();
static void setupFramebuffer();
static void updateEngine();

static FlywheelRenderer::Engine* engine = new FlywheelRenderer::Engine();

void FlywheelRenderer::initialize()
{
    setupShaders();
    setupMesh();
    setupFramebuffer();
}

void FlywheelRenderer::draw()
{
    GLuint INSTANCES = 100;

    updateEngine();
    
    glm::mat4 projection = glm::ortho(-1.7778f, 1.7778f, -1.0f, 1.0f, -1.0f, 1.0f);
    glm::mat4 model = glm::identity<glm::mat4>();
    model = glm::scale(model, {0.5f, 0.5f, 0.5f});

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glBindFramebuffer(GL_FRAMEBUFFER, flywheelFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(u_projection, 1, false, glm::value_ptr(projection));
    glUniformMatrix4fv(u_model, 1, false, glm::value_ptr(model));
    glUniform1f(u_numInstances, INSTANCES - 1);
    glUniform1f(u_angle, glm::radians(engine->angle));
    glUniform1f(u_angleSpeed, glm::radians(engine->angleSpeed));
    glBindVertexArray(meshVAO);
    {
        //glDrawElements(GL_TRIANGLES, meshIndexSize, GL_UNSIGNED_INT, 0);
        glDrawElementsInstanced(GL_TRIANGLES, meshIndexSize, GL_UNSIGNED_INT, 0, INSTANCES);
    }
    glBindVertexArray(0);
    glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, flywheelFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, 1280, 720, 0, 0, 1280, 720, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

FlywheelRenderer::Engine* FlywheelRenderer::getEngine()
{
    return engine;
}

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

    u_projection = glGetUniformLocation(shaderProgram, "u_projection");
    u_model = glGetUniformLocation(shaderProgram, "u_model");
    u_numInstances = glGetUniformLocation(shaderProgram, "u_numInstances");
    u_angle = glGetUniformLocation(shaderProgram, "u_angle");
    u_angleSpeed = glGetUniformLocation(shaderProgram, "u_angleSpeed");
}

void setupMesh()
{
    Assimp::Importer importer;

    const aiScene* assimpScene = importer.ReadFile("rsc/mesh/flywheel.ply", aiProcess_Triangulate);

    if(assimpScene && assimpScene->HasMeshes())
    {
        meshIndexSize = assimpScene->mMeshes[0]->mNumFaces * 3;
        meshSize = assimpScene->mMeshes[0]->mNumVertices;
        aiVector3D* verticies = assimpScene->mMeshes[0]->mVertices;
        aiColor4D* colors = assimpScene->mMeshes[0]->mColors[0];

        GLuint* indicies = new GLuint[meshIndexSize];
        for(int i = 0; i < assimpScene->mMeshes[0]->mNumFaces; i++)
        {
            aiFace* face = &assimpScene->mMeshes[0]->mFaces[i];
            indicies[i * 3] = face->mIndices[0];
            indicies[i * 3 + 1] = face->mIndices[1];
            indicies[i * 3 + 2] = face->mIndices[2];
        }

        GLuint buffers[3];
        glGenBuffers(3, buffers);
        glGenVertexArrays(1, &meshVAO);

        glBindVertexArray(meshVAO);
        {
            glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
            glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(aiVector3D), verticies, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(aiVector3D), (void*)0);
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
            glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(aiColor4D), colors, GL_STATIC_DRAW);
            glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(aiColor4D), (void*)0);
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshIndexSize * sizeof(GLuint), indicies, GL_STATIC_DRAW);
        }
        glBindVertexArray(0);

        delete indicies;
    }
    else
    {
        throw std::runtime_error("Unable to create mesh");
    } 
}

void setupFramebuffer()
{
    GLuint rbo = 0;
    glGenFramebuffers(1, &flywheelFBO);
    glGenRenderbuffers(1, &rbo);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_RGBA16F, 1280, 720);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, flywheelFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("Unable to create framebuffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void updateEngine()
{
    if(!engine->limiterOn)
    {
        engine->angleSpeed += engine->throttle * 30;
    }

	if ((engine->idle <= engine->angleSpeed * 10.0) && (engine->angleSpeed * 10.0 <= engine->idle + engine->idleTolerance))
	{
		engine->angleSpeed -= ((10.0 * pow(engine->angleSpeed * 10.0 - engine->idle, 2.0)) / pow(engine->idleTolerance, 2));
	}
    else
    {
    	engine->angleSpeed -= 10.0;
    }

	engine->angleSpeed = engine->angleSpeed < 0.0 ? 0.0 : engine->angleSpeed;
	engine->angle += engine->angleSpeed;
	if (engine->angleSpeed * 10.0 > engine->revLimiter) engine->limiterOn = true;
	if (engine->limiterOn && engine->angleSpeed * 10.0 < engine->revLimiter - 300.0) engine->limiterOn = false;
	if (engine->angle > 360) { int a = (360 / (int)engine->angle) * 360; engine->angle -= a; }
	engine->rpm = engine->angleSpeed * 10.0;
}