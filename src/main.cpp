#include <iostream>

#include "Window.hpp"
#include "Logger.hpp"
#include "UIRender.hpp"
#include "GLInclude.hpp"
#include "Button.hpp"
#include "Viewport.hpp"
#include "Mesh.hpp"
#include "Shaders.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>

EMLogger mainLogger("Main");

void clearGLErrors()
{
    GLenum error;
    do
    {
        error = glGetError();

        if(error != GL_NO_ERROR)
        {
            mainLogger.submodule("OpenGL").errorf("ID: %d", error);
        }
    } while (error != GL_NO_ERROR);
    
}

int main(int argc, char* argv[])
{
    mainLogger.infof("Starting Up");

    glfwInit();

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    EMWindow window(1280, 720, "Engmsc by Elmfer");
    const EMKeyboard& keyboard = window.getKeyboard();
    const EMMouse& mouse = window.getMouse();
    EMButton test("Button");
    test.x = test.y = 10;
    EMViewport viewport;
    viewport.x = 800;
    viewport.y = 100;
    viewport.width = 150;
    viewport.height = 150;
    emui::setWindow(window);

    glEnable(GL_MULTISAMPLE);

    EMMesh::Ptr mesh = EMMesh::load("res/arrow.ply")[0];

    EMVertexFormat vtxFmt;
    vtxFmt.size = 2;
    vtxFmt[0].data = EMVF_ATTRB_USAGE_POS
                   | EMVF_ATTRB_SIZE(3)
                   | EMVF_ATTRB_TYPE_FLOAT;
    vtxFmt[1].data = EMVF_ATTRB_USAGE_COLOR
                   | EMVF_ATTRB_SIZE(4)
                   | EMVF_ATTRB_TYPE_FLOAT;

    mesh->makeRenderable(vtxFmt);

    float y = 0;

    while(!window.shouldClose())
    {
        glViewport(0, 0, window.getWidth(), window.getHeight());
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        if(keyboard.isKeyPressed(GLFW_KEY_SPACE)) 
            glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(mouse.justScrolled())
        {
            y += mouse.scrollDeltaY() * 10;
        }

        ems::setProjectionMatrix(glm::perspective(glm::radians(70.0f), 1.8f, 0.01f, 10.0f));
        ems::setModelviewMatrix(glm::translate(glm::vec3(0, 0, -3)));
        ems::POS_COLOR_shader();
        glEnable(GL_DEPTH_TEST);
        mesh->render(GL_TRIANGLES);

        emui::setupUIRendering();

        emui::genQuad(100, 100, 200, 200, -1);
        emui::genHorizontalLine(500, 50, 1000, 0xFF000000, 2);
        emui::genLine(300, y, mouse.cursorX(), mouse.cursorY(),
        mouse.isButtonPressed(GLFW_MOUSE_BUTTON_1) ? 0xFFFF0000 : 0xFF00FF00, 5);

        char output[1024];
        snprintf(output, 1023, "§n§l%.2f §r%.2f 0xFF555555" ,mouse.cursorX(), mouse.cursorY());
        emui::genString(output, mouse.cursorX(), mouse.cursorY(), -1, emui::CENTER);

        viewport.start(true);
        test.draw();
        viewport.end();

        if(test.justPressed())
        {
            mainLogger.infof("Clicked From Test Button");
        }

        emui::renderBatch();

        clearGLErrors();
        window.swapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}