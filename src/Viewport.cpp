#include "Viewport.hpp"

#include "UIRender.hpp"
#include "GLInclude.hpp"

EMViewport::EMViewport() :
    m_wasClipping(false)
{
    m_type = VIEWPORT;
}

void EMViewport::start(bool clipping)
{
    draw();

    if(clipping)
    {
        ClippingState clip;
        clip.modelView = m_modelView;
        clip.left = x;
        clip.top = y;
        clip.right = x + width;
        clip.bottom = y + height;
        clip.clipping = true;
        m_clippingStack.push(clip);
        m_wasClipping = true;

        emui::renderBatch(); // Render and flush the batch before writing to the stencil buffer
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        emui::genQuad(x, y, x + width, y + height, 0xFFFFFFFF);
        glColorMask(0, 0, 0, 0);
        emui::renderBatch();
        glColorMask(1, 1, 1, 1);

        glStencilFunc(GL_EQUAL, 1, 0xFF);
    }

    emui::pushStack();
    emui::translate(x, y);
}

void EMViewport::end()
{
    if(m_wasClipping)
    {
        emui::renderBatch();
        glDisable(GL_STENCIL_TEST);
        m_wasClipping = false;
        m_clippingStack.pop();
    }

    emui::popStack();
}

void EMViewport::doDraw()
{
    
}
