#include <iostream>

#include "Window.hpp"
#include "MeshBuilder.hpp"
#include <glm/gtx/transform.hpp>

int main(int argc, char* argv[])
{
    glfwInit();

    EMWindow window(1280, 720, "Engmsc by Elmfer");
    EMVertexFormat vtxFmt;
    vtxFmt.size = 1;
    vtxFmt.attributes[0].data = EMVF_ATTRB_USAGE_POS
                              | EMVF_ATTRB_TYPE_FLOAT
                              | EMVF_ATTRB_SIZE(3)
                              | EMVF_ATTRB_NORMALIZED_FALSE;

    EMMeshBuilder mbTest(vtxFmt);
    mbTest.getModelView() = mbTest.getModelView() * glm::scale(glm::vec3(0.5f, 1.5f, 1.0f));

    while(!window.shouldClose())
    {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        mbTest.position(-0.5f, -0.5f, 0.0f)
        .position(0.5f, -0.5f, 0.0f)
        .position(0.5f, 0.5f, 0.0f)
        .position(-0.5f, 0.5f, 0.0f)
        .index(6, 0, 1, 3, 1, 2, 3);

        mbTest.drawElements(GL_TRIANGLES);
        mbTest.reset();

        window.swapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}