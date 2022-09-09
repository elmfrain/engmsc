#include <iostream>

#include "Window.hpp"
#include "MeshBuilder.hpp"
#include "Logger.hpp"
#include <glm/gtx/transform.hpp>

EMLogger mainLogger("Main");

int main(int argc, char* argv[])
{
    mainLogger.infof("Starting Up");

    glfwInit();

    EMWindow window(1280, 720, "Engmsc by Elmfer");
    EMVertexFormat vtxFmt;
    vtxFmt.size = 1;
    vtxFmt[0].data = EMVF_ATTRB_USAGE_POS
                   | EMVF_ATTRB_TYPE_FLOAT
                   | EMVF_ATTRB_SIZE(3)
                   | EMVF_ATTRB_NORMALIZED_FALSE;

    EMMeshBuilder mbTest(vtxFmt);

    while(!window.shouldClose())
    {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        window.swapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}