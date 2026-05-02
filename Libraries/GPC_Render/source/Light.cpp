#include "Light.h"

#include "GraphicsProgram.h"

namespace GPC {
    Light::Light(const LightType type) : m_type(type) {
    }

    void Light::Create() {

        m_color = glm::vec4(1, 1, 1, 1);
        m_ambient =glm::vec4(0, 0, 0, 0);
        m_intensity = 10;

        m_index = LightCount++;
    }

    void Light::Destroy() {
        LightCount--;
    }

    DirectionalLight::DirectionalLight() : Light(LightType::DIRECTIONAL) {
    }

    void DirectionalLight::Create() {
        Light::Create();

        m_direction = glm::vec4(0, -1, 0, 0);
    }

    PointLight::PointLight() : Light(LightType::POINT) { }

    void PointLight::Create() {
        Light::Create();

        m_position = glm::vec4({0, 0, 0, 0});
        m_range = 10.0f;
    }

    SpotLight::SpotLight() : Light(LightType::SPOT) {
    }

    void SpotLight::Create() {
        Light::Create();

        m_position = glm::vec4({0, 0, 0, 0});
        m_direction = glm::vec4(0, -1, 0, 0);

        m_innerCone = 10;
        m_outerCone = 30;

        m_range = 10;
    }

    LightFactory LightFactory::CreateAsDirectional() { LightFactory factory{}; factory.pLight = new DirectionalLight(); return factory; }
    LightFactory LightFactory::CreateAsSpot()  { LightFactory factory{}; factory.pLight = new SpotLight(); return factory;  }
    LightFactory LightFactory::CreateAsPoint() { LightFactory factory{}; factory.pLight = new PointLight(); return factory;  }

    LightFactory & LightFactory::Intensity(const float intensity) { pLight->SetIntensity(intensity); return *this; }
    LightFactory & LightFactory::Diffuse(const Color &color) { pLight->SetColor(color); return *this; }
    LightFactory & LightFactory::Ambient(const Color &ambient) { pLight->SetAmbient(ambient); return *this; }
    LightFactory & LightFactory::Range(const float range) { pLight->SetRange(range); return *this; }
    LightFactory & LightFactory::InnerCone(const Radian angle) { pLight->SetInnerCone(angle); return *this;  }
    LightFactory & LightFactory::OuterCone(const Radian angle) { pLight->SetOuterCone(angle); return *this; }

} // GPC