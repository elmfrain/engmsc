#ifndef EMGAUGE_HPP
#define EMGAUGE_HPP

#include "MeshBuilder.hpp"
#include "Widget.hpp"
#include "Mesh.hpp"
#include "Animation.hpp"

#include <memory>

class EMGauge : public EMWidget
{
public:
    struct Profile
    {
        float radius;
        float girth;
        float markingGirth;
        float tilt;
        int numSegments = 64;
        int numMarkings = 9;
        int subdivisions = 4;
    };
    EMGauge();

    Profile& getProfile();

    void setValue(float value);
    float getValue() const;
    void setRange(float minValue, float maxValue);
    void getRange(float* getMin, float* getMax) const;
protected:
    static EMMesh::Ptr m_needleMesh;
    static EMMesh::Ptr m_stubbyNeedleMesh;

    // Shader Uniforms
    static int u_angleDelta;
    static int u_numInstances;

    // Overrides EMWidget::doDraw()
    virtual void doDraw();
private:
    Profile m_profile;
    std::unique_ptr<EMMeshBuilder> m_meshBuilder;

    float m_minValue;
    float m_maxValue;
    float m_value;
    float m_prevNeedleAngle;
    EMSmoother m_smoother;
    double m_prevTime;

    void generateBacking();
    void generateMarkings();
    void renderText();
    void renderNeedle();

    static void initRendering(EMVertexFormat& vtxFmt);
};

#endif // EMGAUGE_HPP