#include "ComputeProgram.h"

#include <random>

#include "Geometry.h"
#include "Random.h"
#include "Shader.h"

namespace GPC
{
    VkVertexInputBindingDescription ComputeParticle::GetBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription;
        bindingDescription.binding = 1;
        bindingDescription.stride = sizeof(ComputeParticle);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        return bindingDescription;
    }

    std::vector<VkVertexInputAttributeDescription> ComputeParticle::GetAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

        attributeDescriptions[0].binding = 1;
        attributeDescriptions[0].location = 3;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(ComputeParticle, Position);

        attributeDescriptions[1].binding = 1;
        attributeDescriptions[1].location = 4;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(ComputeParticle, Color_);

        attributeDescriptions[2].binding = 1;
        attributeDescriptions[2].location = 5;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(ComputeParticle, Size);

        attributeDescriptions[3].binding = 1;
        attributeDescriptions[3].location = 6;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(ComputeParticle, Rotation);

        return attributeDescriptions;
    }

    void ComputeProgram::CreateParticle(ComputeParticle& particle, ParticleComputeInformation const &information)
    {
        particle.Velocity   = glm::vec4(
            Random::Float(information.StartMinSpeed.x, information.StartMaxSpeed.x),
            Random::Float(information.StartMinSpeed.y, information.StartMaxSpeed.y),
            Random::Float(information.StartMinSpeed.z, information.StartMaxSpeed.z),
            1.0f
        );
        particle.AngularVelocity   = glm::quat(
           Random::Float(information.StartMinRotationSpeed.w, information.StartMaxRotationSpeed.w),
           Random::Float(information.StartMinRotationSpeed.x, information.StartMaxRotationSpeed.x),
           Random::Float(information.StartMinRotationSpeed.y, information.StartMaxRotationSpeed.y),
           Random::Float(information.StartMinRotationSpeed.z, information.StartMaxRotationSpeed.z)
       );
        particle.Size       = glm::vec4(
            Random::Float(information.StartMinSize.x, information.StartMaxSize.x),
            Random::Float(information.StartMinSize.y, information.StartMaxSize.y),
            Random::Float(information.StartMinSize.z, information.StartMaxSize.z),
            1.0f
        );
        particle.BaseSize = particle.Size;
        particle.Rotation   = glm::quat(
            Random::Float(information.StartMinRotation.w, information.StartMaxRotation.w),
            Random::Float(information.StartMinRotation.x, information.StartMaxRotation.x),
            Random::Float(information.StartMinRotation.y, information.StartMaxRotation.y),
            Random::Float(information.StartMinRotation.z, information.StartMaxRotation.z)
        );
        particle.Color_      = Color::Lerp(information.StartMinColor, information.StartMaxColor, Random::Float());
        particle.LifeTime    = glm::vec4(
             Random::Float(information.LifeTimeMin, information.LifeTimeMax),
             information.LifeTimeMin,
             information.LifeTimeMax,
             information.Duration
         );
        particle.BasePositionMax = glm::vec4(information.StartMaxPosition, 1.0f);
        particle.BasePositionMin = glm::vec4(information.StartMinPosition, 1.0f);

    }

    void ParticleThread::Callback(void *pData, uint64_t data_size)
    {

        ParticleSynchronisation* synchronisation = static_cast<ParticleSynchronisation*>(pData);
        while (!synchronisation->IsFinished) {

            switch (synchronisation->Shape) {
                case ParticleEmissionShape::CUBE: ComputeProgram::GenerateAsCube(synchronisation->Particles, synchronisation->Information); break;
                case ParticleEmissionShape::RECTANGLE: ComputeProgram::GenerateAsRect(synchronisation->Particles, synchronisation->Information); break;
                case ParticleEmissionShape::CIRCLE: ComputeProgram::GenerateAsCircle(synchronisation->Particles, synchronisation->Information); break;
                case ParticleEmissionShape::SHPERE: ComputeProgram::GenerateAsSphere(synchronisation->Particles, synchronisation->Information); break;
            }

            synchronisation->IsFinished = true;
        }

    }

    void ComputeProgram::Create(ComputeProgramInformation const &information)
    {

        m_MaxParticle = std::min(information.Type.MaxParticle, MAX_PARTICLES);

        BindLayout( {
            .Name = "UNIFORM_COMPUTE_BUFFER",
            .Type = GPC::DescriptorType::UNIFORM_BUFFER,
            .Usages = GPC::UNIFORM_BUFFER_BIT | GPC::VERTEX_BUFFER_BIT | GPC::TRANSFER_DST_BIT,
            .BindingSlot = 0,
            .DescriptorCount = 1,
            .ElementCount = 1,
            .ElementStride = sizeof(GPC::ComputeContext),
            .Flags = VK_SHADER_STAGE_COMPUTE_BIT,
            .Sampler_ = nullptr
        } );

        BindLayout( {
            .Name = "STORAGE_COMPUTE_1",
            .Type = GPC::DescriptorType::STORAGE_BUFFER,
            .Usages = GPC::STORAGE_BUFFER_BIT | GPC::VERTEX_BUFFER_BIT | GPC::TRANSFER_DST_BIT,
            .BindingSlot = 1,
            .DescriptorCount = 1,
            .ElementCount = m_MaxParticle,
            .ElementStride = sizeof(GPC::ComputeParticle),
            .Flags = VK_SHADER_STAGE_COMPUTE_BIT,
            .Sampler_ = nullptr
        } );

        BindLayout( {
            .Name = "STORAGE_COMPUTE_2",
            .Type = GPC::DescriptorType::STORAGE_BUFFER,
            .Usages = GPC::STORAGE_BUFFER_BIT | GPC::VERTEX_BUFFER_BIT | GPC::TRANSFER_DST_BIT,
            .BindingSlot = 2,
            .DescriptorCount = 1,
            .ElementCount = m_MaxParticle,
            .ElementStride = sizeof(GPC::ComputeParticle),
            .Flags = VK_SHADER_STAGE_COMPUTE_BIT,
            .Sampler_ = nullptr
        } );

        for (auto const& info : m_DescriptorSetInfo) {
            m_MaxSize = std::max(info.ElementCount * info.ElementStride, m_MaxSize);
        }

        BufferInformation bufferInformation {};
        bufferInformation.Flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        bufferInformation.Usages = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInformation.TotalSize = m_MaxSize;
        m_StagingBuffer.Create("COMPUTE_UPLOAD_SHADER", bufferInformation);

        GPC_SUCCESS(CreateDescriptorLayout());
        GPC_SUCCESS(CreatePipelineLayout(information.Base));

        GPC_SUCCESS(CreatePipeline(information.Base));

        GPC_SUCCESS(CreateDescriptorPool(information.Base));
        GPC_SUCCESS(CreateDescriptorSets());

        CreateUniformBuffers();

        m_ParticleSynchronisation.Information = information.Type;
        m_ParticleSynchronisation.Shape = information.Type.Shape;
        m_ParticleSynchronisation.Particles = std::vector<GPC::ComputeParticle>(information.Type.MaxParticle);
        m_EmissionMode = information.Type.Mode;
        m_Thread.SubmitData(&m_ParticleSynchronisation, sizeof(m_ParticleSynchronisation));

    }

    void ComputeProgram::Destroy()
    {
        m_StagingBuffer.Destroy();
        m_Thread.Stop();
        IProgram::Destroy();
    }

    bool ComputeProgram::IsAvailable() const
    {
        return m_ParticleSynchronisation.IsFinished && m_ParticleSynchronisation.IsUploaded;
    }

    void ComputeProgram::Reset()
    {
        m_ParticleSynchronisation.IsUploaded = false;
        m_ParticleSynchronisation.IsFinished = false;

        m_Thread.Start();
    }

    ErrorType ComputeProgram::CreatePipelineLayout(ProgramInformation const &information)
    {

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount           = 1; // Optional
        pipelineLayoutInfo.pSetLayouts              = &m_DescriptorSetLayout; // Optional
        pipelineLayoutInfo.pushConstantRangeCount   = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges      = nullptr; // Optional

        if (vkCreatePipelineLayout(RenderDevice::GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            return ErrorType::VULKAN_PROGRAM_BINDINGS_CREATION;
        }
        return ErrorType::SUCCESS;

    }

    ErrorType ComputeProgram::CreatePipeline(ProgramInformation const &information)
    {

        std::vector<VkPipelineShaderStageCreateInfo> shaders;
        for (auto* shader : information.Shaders)
        {
            shaders.push_back(shader->GetShaderInformation());
        }

        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.stage = shaders[0];

        if (vkCreateComputePipelines(RenderDevice::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS) {
            return ErrorType::VULKAN_PROGRAM_PIPELINE_CREATION;
        }

        return ErrorType::SUCCESS;

    }

    void ComputeProgram::UpdateContext(ComputeContext const& computeContext)
    {

        Update(0, &computeContext, sizeof(computeContext), 0);

        if (!m_ParticleSynchronisation.IsUploaded && m_ParticleSynchronisation.IsFinished) {
            UpdateAll(reinterpret_cast<uint8_t const *>(m_ParticleSynchronisation.Particles.data()),
                sizeof(uint8_t),
                m_MaxSize, 0);
            m_ParticleSynchronisation.IsUploaded = true;
        }
    }

    void ComputeProgram::UpdateAll(uint8_t const*data, uint32_t elementSizeInByte, uint32_t elementCount, uint32_t offset)
    {
        uint8_t* stagingData = nullptr;
        m_StagingBuffer.Map<uint8_t>(&stagingData, offset, elementSizeInByte * elementCount);
        memcpy(stagingData, data, elementSizeInByte * elementCount);
        m_StagingBuffer.Unmap();

        for (int i = 1; i < m_UniformBuffers.size(); i++) {
            auto& uniformBuffer = m_UniformBuffers[i];
            uniformBuffer.Copy(m_StagingBuffer);
        }
    }

    void ComputeProgram::Update()
    {

        for (size_t i = 0; i < MAX_GENERATED_FRAME; i++) {
            VkDescriptorBufferInfo uniformBufferInfo{};
            uniformBufferInfo.buffer = m_UniformBuffers[i].GetBuffer();
            uniformBufferInfo.offset = 0;
            uniformBufferInfo.range = sizeof(ComputeContext);

            std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = m_DescriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &uniformBufferInfo;

            const uint32_t offset = 3;
            VkDescriptorBufferInfo storageBufferInfoLastFrame{};
            storageBufferInfoLastFrame.buffer = m_UniformBuffers[offset + ((i - 1) % MAX_GENERATED_FRAME)].GetBuffer();
            storageBufferInfoLastFrame.offset = 0;
            storageBufferInfoLastFrame.range = m_MaxSize;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = m_DescriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pBufferInfo = &storageBufferInfoLastFrame;

            VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
            storageBufferInfoCurrentFrame.buffer = m_UniformBuffers[offset + i].GetBuffer();
            storageBufferInfoCurrentFrame.offset = 0;
            storageBufferInfoCurrentFrame.range = m_MaxSize;

            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2].dstSet = m_DescriptorSets[i];
            descriptorWrites[2].dstBinding = 2;
            descriptorWrites[2].dstArrayElement = 0;
            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrites[2].descriptorCount = 1;
            descriptorWrites[2].pBufferInfo = &storageBufferInfoCurrentFrame;

            vkUpdateDescriptorSets(RenderDevice::GetDevice(), 3, descriptorWrites.data(), 0, nullptr);
        }
    }

    void ComputeProgram::Update(int32_t binding, void const *data, uint32_t size, uint32_t offset) const
    {
        IProgram::Update(binding, data, size, offset);
    }

    VkBuffer ComputeProgram::GetBuffer() const
    {
        uint32_t lastOutputIndex = (RenderCommands::GetCurrentFrame() == 0) ? 3 : 4;
        return m_UniformBuffers[lastOutputIndex].GetBuffer();
    }

    uint32_t ComputeProgram::GetMaxParticle() const
    {
        return m_MaxParticle;
    }

    void ComputeProgram::GenerateAsCircle(std::vector<ComputeParticle>& particles, ParticleComputeInformation const &information)
    {
        for (auto& particle : particles) {
            float theta = Random::Float() * 2 * glm::pi<float>();
            float x = Random::Float(information.StartMinPosition.x, information.StartMaxPosition.x) * cos(theta);
            float y = Random::Float(information.StartMinPosition.y, information.StartMaxPosition.y) * sin(theta);

            particle.Position   = glm::vec4(x, y, 0.0f, 1.0f);
            CreateParticle(particle, information);
        }
    }

    void ComputeProgram::GenerateAsRect(std::vector<ComputeParticle>& particles, ParticleComputeInformation const&information)
    {
        for (auto& particle : particles) {
            float x = Random::Float(information.StartMinPosition.x, information.StartMaxPosition.x);
            float y = Random::Float(information.StartMinPosition.y, information.StartMaxPosition.y);

            particle.Position   = glm::vec4(x, y, 0.0f, 1.0f);
            CreateParticle(particle, information);
        }

    }

    void ComputeProgram::GenerateAsCube(std::vector<ComputeParticle>& particles, ParticleComputeInformation const &information)
    {
        for (auto& particle : particles) {
            float x = Random::Float(information.StartMinPosition.x, information.StartMaxPosition.x);
            float y = Random::Float(information.StartMinPosition.y, information.StartMaxPosition.y);
            float z = Random::Float(information.StartMinPosition.z, information.StartMaxPosition.z);

            particle.Position   = glm::vec4(x, y, z, 1.0f);
            CreateParticle(particle, information);
        }

    }

    void ComputeProgram::GenerateAsSphere(std::vector<ComputeParticle>& particles, ParticleComputeInformation const &information)
    {
        for (auto& particle : particles) {
            float theta = Random::Float() * glm::pi<float>();
            float phi   = Random::Float() * glm::pi<float>();
            float x = Random::Float(information.StartMinPosition.x, information.StartMaxPosition.x) * cos(theta) * cos(phi);
            float y = Random::Float(information.StartMinPosition.y, information.StartMaxPosition.y) * sin(phi);
            float z = Random::Float(information.StartMinPosition.z, information.StartMaxPosition.z) * sin(theta) * cos(phi);

            particle.Position   = glm::vec4(x, y, z, 1.0f);
            CreateParticle(particle, information);
        }
    }

} // GPC