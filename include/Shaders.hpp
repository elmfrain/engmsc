#ifndef EM_SHADERS_HPP
#define EM_SHADERS_HPP

#include <glm/glm.hpp>

namespace ems
{
    const glm::mat4& getProjectionMatrix();
    const glm::mat4& getModelviewMatrix();
    const glm::vec4& getColor();
    int getMaxTextureUnits();

    void setProjectionMatrix(const glm::mat4& projection);
    void setModelviewMatrix(const glm::mat4& modelview);
    void setColor(const glm::vec4& color);
    void setColor(float r, float g, float b, float a);

    void POS_shader();
    void POS_UV_shader();
    void POS_COLOR_shader();
    void POS_UV_COLOR_TEXID_shader();
    void UI_shader();
}

#endif // EM_SHADERS_HPP