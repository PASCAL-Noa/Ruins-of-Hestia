#pragma once
#include "Color.h"
#include "ErrorType.h"
#include "Object.h"
#include "GraphicsProgram.h"
#include "WorkerThread.h"

namespace GPC
{

    enum ComputeProgramType
    {
        PARTICLE,
        CALCULATION,
    };

    enum class ParticleEmissionMode
    {
        BURST,
        OVER_TIME
    };

    enum class ParticleEmissionShape
    {
        RECTANGLE,
        CIRCLE,
        SHPERE,
        CUBE,
    };

    struct ParticleComputeInformation
    {
        bool                    Looping             = false;
        uint32_t                MaxParticle         = 1'024;
        ParticleEmissionMode    Mode                = ParticleEmissionMode::BURST;
        ParticleEmissionShape   Shape               = ParticleEmissionShape::RECTANGLE;

        glm::vec3               StartMinPosition        = glm::vec3(1.0f);
        glm::vec3               StartMaxPosition        = glm::vec3(1.0f);

        float                   StartAfterSeconds       = 0;
        float                   LifeTimeMin             = 0.0f;
        float                   LifeTimeMax             = 0.0f;

        glm::vec3               StartMinSpeed           = glm::vec3(1.0f);
        glm::vec3               StartMaxSpeed           = glm::vec3(1.0f);

        glm::quat               StartMinRotationSpeed   = glm::quat();
        glm::quat               StartMaxRotationSpeed   = glm::quat();

        glm::vec3               StartMinSize            = glm::vec3(1.0f);
        glm::vec3               StartMaxSize            = glm::vec3(1.0f);

        glm::quat               StartMinRotation        = glm::quat();
        glm::quat               StartMaxRotation        = glm::quat();

        Color                   StartMaxColor           = Colors::WHITE;
        Color                   StartMinColor           = Colors::WHITE;

        float                   Duration                = 2.0f;
    };

    struct CalculationComputeInformation
    {
        uint8_t*    Data;
        uint32_t    Size;
    };

    struct ComputeProgramInformation
    {
        ProgramInformation Base {};

        ParticleComputeInformation Type {};

    };

    struct ComputeContext
    {
        float       DeltaTime = 1.0f;
    };

    struct alignas(16) ComputeParticle
    {
        glm::vec4   Position;
        glm::vec4   BasePositionMin;
        glm::vec4   BasePositionMax;
        glm::vec4   BaseSize;
        glm::vec4   Size;
        Color       Color_;
        glm::quat   Rotation;
        glm::vec4   Velocity;
        glm::quat   AngularVelocity;
        glm::vec4   LifeTime;

        static VkVertexInputBindingDescription GetBindingDescription(); // TODO  AUTOMATISER

        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
    };

    struct ParticleSynchronisation
    {
        std::vector<ComputeParticle>    Particles;
        ParticleEmissionShape           Shape;
        ParticleComputeInformation      Information;
        bool                            IsFinished = false;
        bool                            IsUploaded = false;
    };

    class ParticleThread : public WorkerThread
    {
        void Callback(void *pData, uint64_t data_size) override;
    };

    GPC_INHERIT_OBJECT( ComputeProgram ), public IProgram
    {
        public:
        ComputeProgram() = default;
        ~ComputeProgram() override = default;

        void Create(ComputeProgramInformation const& information);
        void Destroy() override;

        bool IsAvailable() const;
        void Reset();

        void UpdateContext(ComputeContext const& computeContext);
        void UpdateAll(uint8_t const* data, uint32_t elementSizeInByte, uint32_t elementCount, uint32_t offset);
        void Update();
        void Update(int32_t binding, void const *data, uint32_t size, uint32_t offset) const override;

        ParticleEmissionMode const& GetParticleMode() const { return m_EmissionMode; };

        VkBuffer GetBuffer() const;
        uint32_t GetMaxParticle() const;
    private:
        void Create(ProgramInformation const& information) override {};

        Buffer   m_StagingBuffer;
        uint32_t m_MaxSize = 0;
        uint32_t m_MaxParticle = 0;

        ErrorType CreatePipelineLayout(ProgramInformation const &information) override;
        ErrorType CreatePipeline(ProgramInformation const &information);

        ParticleSynchronisation m_ParticleSynchronisation;
        ParticleEmissionMode    m_EmissionMode;
        ParticleThread          m_Thread;

        uint32_t m_ComputeInIndex   = 0;
        uint32_t m_ComputeOutIndex  = 1;

        uint32_t m_SwapIndex = 0;
        uint32_t m_Indices[4] = { 0, 2, 1, 3 };

        // TODO Changer ca de place
        static void GenerateAsCircle(std::vector<ComputeParticle>& particles, ParticleComputeInformation const& information);
        static void GenerateAsRect(  std::vector<ComputeParticle>& particles, ParticleComputeInformation const& information);
        static void GenerateAsCube(  std::vector<ComputeParticle>& particles, ParticleComputeInformation const& information);
        static void GenerateAsSphere(std::vector<ComputeParticle>& particles, ParticleComputeInformation const& information);

        static void CreateParticle(ComputeParticle& particle, ParticleComputeInformation const &information);

        friend ParticleThread;

    };

}
