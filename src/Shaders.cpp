#include "Shaders.hpp"

#ifndef EMSCRIPTEN
#include "gl3_shaders.hpp"
#else
#include "gles3_shaders.h"
#endif

#include "GLInclude.hpp"
#include "Logger.hpp"

#define U_PROJECTION_M_BIT 0
#define U_MODELVIEW_M_BIT 1
#define U_COLOR_BIT 2
#define U_TEXTURES_BIT 3

static glm::mat4 m_projectionMatrix(1.0f);
static glm::mat4 m_modelviewMatrix(1.0f);
static glm::vec4 m_color(1.0f, 1.0f, 1.0f, 1.0f);
static int m_texUnits[32];
static uint32_t m_uniformUpdateFlags = 0xFFFFFFFF;

static GLuint m_currentBoundProgram;
static int m_maxTexUnits = 32;
static bool m_hasShadersInit = false;

static EMLogger m_logger("Shaders");

static void i_init();
static inline void i_setNthBit(uint32_t* flags, int bit);
static inline void i_clearNthBit(uint32_t* flags, int bit);
static inline int i_getNthBit(uint32_t* flags, int bit);

struct BasicShader
{
    GLuint programID;
    GLuint u_projectionMatrix;
    GLuint u_modelViewMatrix;
    GLuint u_color;
    GLuint u_textures;
    bool hasInit = false;
    const char* vcode;
    const char* fcode;
    const char* name;

    BasicShader(const char* namep, const char* vcodep, const char* fcodep) :
    vcode(vcodep),
    fcode(fcodep),
    name(namep)
    { }

    void init()
    {
        if(programID)
        {
            glDeleteProgram(programID);
        }

        programID = glCreateProgram();
        GLuint vecShader = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(vecShader, 1, &vcode, NULL);
        glCompileShader(vecShader);
        GLint success;
        glGetShaderiv(vecShader, GL_COMPILE_STATUS, &success);
        if(success == GL_FALSE)
        {
            char log[1024] = {0};
            glGetShaderInfoLog(vecShader, sizeof(log), NULL, log);
            
            m_logger.submodule("Vertex Shader")
            .errorf("Compilation Failed:\n%s\n", log);

            goto link;
        }

        glShaderSource(fragShader, 1, &fcode, NULL);
        glCompileShader(fragShader);
        glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
        if(success == GL_FALSE)
        {
            char log[1024] = {0};
            glGetShaderInfoLog(fragShader, sizeof(log), NULL, log);

            m_logger.submodule("Fragment Shader")
            .errorf("Compilation Failed:\n%s\n", log);
        }

    link:
        if(success)
        {
            glAttachShader(programID, vecShader);
            glAttachShader(programID, fragShader);
            glLinkProgram(programID);

            glGetProgramiv(programID, GL_LINK_STATUS, &success);
            if(success == GL_FALSE)
            {
                char log[1024] = {0};
                glGetProgramInfoLog(programID, sizeof(log), NULL, log);

                m_logger.submodule("Program Shader")
                .errorf("Linking Failed:\n%s\n", log);
            }

            u_projectionMatrix = glGetUniformLocation(programID, "u_projectionMatrix");
            u_modelViewMatrix = glGetUniformLocation(programID, "u_modelViewMatrix");
            u_color = glGetUniformLocation(programID, "u_color");
            u_textures = glGetUniformLocation(programID, "u_textures");
        }
        else
        {
            glDeleteProgram(programID);
            programID = 0;
        }

        glDeleteShader(vecShader);
        glDeleteShader(fragShader);

        if(programID) m_logger.submodule(name).infof("Successfully created program");
        hasInit = true;
    }

#define u_Assert(t, b) if(i_getNthBit(&m_uniformUpdateFlags, b)) { t; i_clearNthBit(&m_uniformUpdateFlags, b); }

    void use()
    {
        if(!m_hasShadersInit) i_init();
        if(!hasInit) init();

        if(!programID) return;

        if(m_currentBoundProgram != programID)
        {
            glUseProgram(programID);
            m_currentBoundProgram = programID;
            m_uniformUpdateFlags = 0xFFFFFFFF;
        }

        u_Assert(glUniformMatrix4fv(u_projectionMatrix, 1, GL_FALSE, (float*) &m_projectionMatrix), U_PROJECTION_M_BIT)
        u_Assert(glUniformMatrix4fv(u_modelViewMatrix, 1, GL_FALSE, (float*) &m_modelviewMatrix), U_MODELVIEW_M_BIT)
        u_Assert(glUniform4fv(u_color, 1, (float*) &m_color), U_COLOR_BIT)
        u_Assert(glUniform1iv(u_textures, m_maxTexUnits, m_texUnits);, U_TEXTURES_BIT);
    }
};

static BasicShader POS_SHADER
("POS", POS_SHADER_vcode, POS_SHADER_fcode);

static BasicShader POS_UV_SHADER
("POS_UV", POS_UV_SHADER_vcode, POS_UV_SHADER_fcode);

static BasicShader POS_COLOR_SHADER
("POS_COLOR", POS_COLOR_SHADER_vcode, POS_COLOR_SHADER_fcode);

static BasicShader POS_UV_COLOR_TEXID_SHADER
("POS_UV_COLOR_TEXID", POS_UV_COLOR_TEXID_SHADER_vcode, POS_UV_COLOR_TEXID_SHADER_fcode);

namespace ems
{
    const glm::mat4& getProjectionMatrix()
    {
        return m_projectionMatrix;
    }

    const glm::mat4& getModelviewMatrix()
    {
        return m_modelviewMatrix;
    }

    const glm::vec4& getColor()
    {
        return m_color;
    }

    int getMaxTextureUnits()
    {
        return m_maxTexUnits;
    }

    void setProjectionMatrix(const glm::mat4& projection)
    {
        m_projectionMatrix = projection;
        i_setNthBit(&m_uniformUpdateFlags, U_PROJECTION_M_BIT);
    }

    void setModelviewMatrix(const glm::mat4& modelview)
    {
        m_modelviewMatrix = modelview;
        i_setNthBit(&m_uniformUpdateFlags, U_MODELVIEW_M_BIT);
    }

    void setColor(const glm::vec4& color)
    {
        m_color = color;
        i_setNthBit(&m_uniformUpdateFlags, U_COLOR_BIT);
    }

    void setColor(float r, float g, float b, float a)
    {
        setColor(glm::vec4(r, g, b, a));
    }

    void POS_shader()
    {
        POS_SHADER.use();
    }

    void POS_UV_shader()
    {
        POS_UV_SHADER.use();
    }

    void POS_COLOR_shader()
    {
        POS_COLOR_SHADER.use();
    }

    void POS_UV_COLOR_TEXID_shader()
    {
        POS_UV_COLOR_TEXID_SHADER.use();
    }
}

static void i_init()
{
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_maxTexUnits);
    m_maxTexUnits = m_maxTexUnits <= 32 ? m_maxTexUnits : 32;
    m_logger.infof("%d Max Tex Units", m_maxTexUnits);

    for(int i = 0; i < m_maxTexUnits; i++)
    {
        m_texUnits[i] = i;
    }

    m_logger.infof("Initialized Module");
    m_hasShadersInit = true;
}

static inline void i_setNthBit(uint32_t* flags, int bit)
{
    uint32_t set = ((uint32_t) 1) << bit;
    *flags |= set;
}

static inline void i_clearNthBit(uint32_t* flags, int bit)
{
    uint32_t set = ~(((uint32_t) 1) << bit);
    *flags &= set;
}

static inline int i_getNthBit(uint32_t* flags, int bit)
{
    uint32_t value = ((uint32_t) 1) << bit;
    value &= *flags;
    value = value >> bit;

    return value;
}