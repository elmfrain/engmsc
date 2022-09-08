#include <iostream>

#include "EMWindow.hpp"
#include "MeshBuilder.hpp"

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

    while(!window.shouldClose())
    {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        mbTest.position(-0.5f, -0.5f, 0.0f)
        .position(0.5f, -0.5f, 0.0f)
        .position(0.0f, 0.5f, 0.0f);

        mbTest.drawArrays(GL_TRIANGLES);

        window.swapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}