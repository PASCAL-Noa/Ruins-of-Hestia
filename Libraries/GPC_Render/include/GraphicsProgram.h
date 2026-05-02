#pragma once

#include <array>
#include <vulkan/vulkan.h>


#include "ErrorType.h"
#include "Object.h"

#include "IProgram.h"

namespace GPC
{

    GPC_INHERIT_OBJECT(GraphicsProgram), public IProgram {
        static inline uint64_t ID = 0;

    public:
        GraphicsProgram() = default;
        ~GraphicsProgram() override;

        void Create(ProgramInformation const& information) override;
        void Destroy() override;
        void InitializeWith(std::vector<Texture*> const& textures, Sampler const& sampler) ;
        void InitializeWith(int32_t binding, std::vector<Texture const*> const& textures, Sampler const& sampler) const;
        void InitializeWith(std::vector<VkDescriptorImageInfo>& images) const;

        void SetImages(std::vector<Texture const*> const& textures, Sampler const& sampler) const;
        void Update(int32_t binding, void const *data, uint32_t size, uint32_t offset) const override;

        ErrorType CreatePipeline(ProgramInformation const &information, VertexInformation&vertexInformation, ViewportInformation const&viewportInformation,
            AttachmentsInformation const&attachmentsInformation);
        ErrorType CreatePipelineLayout(ProgramInformation const &information) override;

        void UseShadows(RenderWindow* pWindow);

        std::vector<VkDescriptorImageInfo> GetImageInfos() const { return m_imageInfos; } ;
    private:
        uint64_t m_ID = 0;
        uint32_t m_BindingIndex = 0;

        std::vector<VkDescriptorImageInfo> m_imageInfos = {};

    };

}
