#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Buffer.h"
#include "Color.h"
#include "Object.h"
#include "Forward.h"

namespace GPC
{

    struct alignas(16) LightData
    {
        glm::vec4 Position;

        glm::vec4 Direction;
        glm::vec4 Color;

        glm::vec4 Ambient;

        float Intensity;
        float Range;
        float InnerCone;
        float OuterCone;

        int32_t Type;
    };

    using Radian = float;

    GPC_INHERIT_OBJECT(Light)
    {
        public:

        enum class LightType : uint32_t
        {
            DIRECTIONAL = 0,
            POINT       = 1,
            SPOT        = 2
        };

        public:

        explicit Light(LightType type);
        ~Light() override = default;

        virtual void Create();

        virtual void Destroy();

        virtual void SetType(const LightType type) { m_type = type; }
        virtual void SetColor(const glm::vec4& color) { m_color = color; }
        virtual void SetAmbient(const glm::vec4& ambient) { m_ambient = ambient; }
        virtual void SetPosition(const glm::vec3& position) {  }
        virtual void SetDirection(const glm::vec4& direction) {  }
        virtual void SetRange(const float range) {  }
        virtual void SetInnerCone(const Radian angle) {  }
        virtual void SetOuterCone(const Radian angle) {  }
        virtual void SetIntensity(const float intensity) { m_intensity = intensity; }

        [[nodiscard]] virtual uint32_t GetType() const { return static_cast<uint32_t>(m_type); }
        [[nodiscard]] virtual glm::vec4 const& GetColor() const { return m_color; }
        [[nodiscard]] virtual glm::vec4 const& GetAmbient() const { return m_ambient; }
        [[nodiscard]] virtual glm::vec4 GetPosition() const { return { 0, 0, 0, 0}; }
        [[nodiscard]] virtual glm::vec4 GetDirection() const { return { 0, 0, 0, 0}; }
        [[nodiscard]] virtual float GetRange() const { return 0.0f; }
        [[nodiscard]] virtual Radian GetInnerCone() const { return 0.0f; }
        [[nodiscard]] virtual Radian GetOuterCone() const { return 0.0f; }
        [[nodiscard]] virtual float GetIntensity() const { return m_intensity; }
        [[nodiscard]] virtual uint32_t const& GetLightCount() const { return LightCount; }

        protected:

        LightType m_type;

        glm::vec4 m_color{};
        glm::vec4 m_ambient{};
        float m_intensity = 1.0f;

        uint32_t m_index = 0;

        inline static uint32_t LightCount = 0;
    };

    class DirectionalLight : public Light
    {
        public:

        DirectionalLight();

        void Create() override;

        void SetDirection(const glm::vec4& direction) override { m_direction = direction; }

        [[nodiscard]] glm::vec4 GetDirection() const override { return m_direction; }

        private:

        glm::vec4 m_direction{};
    };

    class PointLight : public Light
    {
        public:

        PointLight();

        void Create() override;

        void SetPosition(const glm::vec3& position) override { m_position = glm::vec4(position.x, position.y, position.z, 0); }
        void SetRange(const float range) override { m_range = range; }

        [[nodiscard]] glm::vec4 GetPosition() const override { return m_position; }
        [[nodiscard]] float GetRange() const override { return m_range; };

    private:
        glm::vec4 m_position{};
        float m_range = 5.f;
    };



    class SpotLight : public Light
    {
        public:

        SpotLight();

        void Create() override;


        void SetPosition(const glm::vec3& position) override { m_position = glm::vec4(position.x, position.y, position.z, 0); }
        void SetDirection(const glm::vec4& direction) override { m_direction = direction; }
        void SetRange(const float range) override { m_range = range; }
        void SetInnerCone(const Radian angle) override { m_innerCone = angle; }
        void SetOuterCone(const Radian angle) override { m_outerCone = angle; }

        [[nodiscard]] glm::vec4 GetPosition() const override { return m_position; }
        [[nodiscard]] glm::vec4 GetDirection() const override { return m_direction; }
        [[nodiscard]] Radian GetOuterCone() const override { return m_outerCone; }
        [[nodiscard]] Radian GetInnerCone() const override { return m_innerCone; }
        [[nodiscard]] float GetRange() const override { return m_range; }


    private:

        glm::vec4 m_direction{};
        glm::vec4 m_position{};

        Radian m_innerCone{};
        Radian m_outerCone{};
        float m_range = 5.f;
    };

    class LightFactory {

        Light* pLight = nullptr;
        LightFactory() = default;

    public:
        static LightFactory CreateAsDirectional();
        static LightFactory CreateAsSpot();
        static LightFactory CreateAsPoint();

        LightFactory& Intensity(const float intensity);
        LightFactory& Diffuse(const Color& color);
        LightFactory& Ambient(const Color& ambient);
        LightFactory& Range(const float range);
        LightFactory& InnerCone(const Radian angle);
        LightFactory& OuterCone(const Radian angle);

        Light* Build() const { return pLight; }
    };

}
