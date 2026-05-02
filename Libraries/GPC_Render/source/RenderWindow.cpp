#include "RenderWindow.h"

#include "RenderObjectParticle.h"

namespace GPC
{
    void CameraContext::GetViewProj(glm::mat4 &view, glm::mat4 &proj, float aspect_ratio) const {

        view = Transform;

        if (UseOrthographic) {
            proj = glm::orthoRH_ZO(-Height * aspect_ratio, Height * aspect_ratio, -Height, Height, 0.0f, 10000.0f);
        }
        else {
            proj = glm::perspectiveRH_ZO(FOV, aspect_ratio, 0.1f, 2000.0f);
        }

        proj[1][1] *= -1;

    }

    RenderWindow::~RenderWindow() {}

    ErrorType RenderWindow::Create(const char *title, uint16_t width, uint16_t height, bool fullscreen)
    {
        return Window::Create(title, width, height, fullscreen);
    }

    ErrorType RenderWindow::Create()
    {
        ErrorType err = Window::Create();

	    GPC_SUCCESS(RenderDevice::Create(this));
        GPC_SUCCESS(RenderCommands::Create());
        RenderCommands::SetCurrentFrame(&m_CurrentFrame);

        m_Swapchain.Create(this);

        RenderTargetInformation information;
        information.Format      = m_Swapchain.GetFormat();
        information.Width       = GetSize().x;
        information.Height      = GetSize().y;
        information.UseMSAA     = true;
        information.UseDepthBuffer = true;
        information.Layout      = ImageLayoutType::PRESENT_SRC;
        information.Swapchain_   = &m_Swapchain;

        GPC_SUCCESS(m_RenderTarget.Create(information));

        GPC_SUCCESS(CreateSyncObjects());

        CreateComputeProgramLayer();

        return err;
    }

    void RenderWindow::Destroy()
    {

        vkDeviceWaitIdle(RenderDevice::GetDevice());

        m_particleGraphicProgram.Destroy();
        m_particleRenderPass.Destroy();
        m_ParticleDepthTexture->Destroy();
        delete m_ParticleDepthTexture;

        m_RenderTarget.Destroy();

        m_Swapchain.Destroy();

        for (size_t i = 0; i < m_RenderTarget.GetImages().size(); i++)
        {
            vkDestroySemaphore(RenderDevice::GetDevice(),   m_RenderFinishedSemaphores[i], nullptr); // TODO Encaps Semaphore
        }


        for (size_t i = 0; i < MAX_GENERATED_FRAME; i++) {

            vkDestroySemaphore(RenderDevice::GetDevice(),   m_ImageAvailableSemaphores[i], nullptr);
            vkDestroyFence(RenderDevice::GetDevice(),       m_InFlightFences[i], nullptr);

            vkDestroySemaphore(RenderDevice::GetDevice(),   m_ComputeFinishedSemaphores[i], nullptr);
            vkDestroyFence(RenderDevice::GetDevice(),       m_ComputeInFlightFences[i], nullptr);

        }

        RenderCommands::Destroy();

        vkDestroySurfaceKHR(RenderDevice::GetVulkanInstance(), *mp_Surface, nullptr);

        RenderDevice::Destroy();

    }

    ErrorType RenderWindow::FlushCommands()
    {
        /*
         * Wait for the previous frame to finish
         * Acquire an image from the swap chain
         * Record a command buffer which draws the scene onto that image
         * Submit the recorded command buffer
         * Present the swap chain image
        */

        // Refer to https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation
        // Semaphores for sync the GPU with signal
        // And Fences for sync the CPU with GPU

        vkWaitForFences(RenderDevice::GetDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(RenderDevice::GetDevice(), m_Swapchain.GetSwapchain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            GetIsResized() = false;
            RecreateSwapchain();
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            return ErrorType::WINDOW_SURFACE_ACQUIREMENT;
        }

        vkResetFences(RenderDevice::GetDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

        vkResetCommandBuffer(RenderCommands::GetCommandBuffer(), 0);
        m_ImageIndex = imageIndex;
        return ErrorType::SUCCESS;
    }

    void RenderWindow::BeginDrawing()
    {

        FlushCommands();

        RenderCommands::BeginCommands();

    }

    void RenderWindow::BeginDraw3D()
    {
        m_RenderTarget.BeginDraw3D();
    }

    void RenderWindow::EndDraw3D()
    {
        m_RenderTarget.EndDraw3D();
    }

    void RenderWindow::BeginDraw2D()
    {
        m_RenderTarget.BeginDraw2D();
    }

    void RenderWindow::EndDraw2D()
    {
        m_RenderTarget.EndDraw2D();
    }

    void RenderWindow::BeginDrawParticles()
    {
        m_particleRenderPass.Begin();
        m_particleGraphicProgram.Update(0, &m_RenderTarget.GetCamera(), sizeof(m_RenderTarget.GetCamera()), 0);
        UseProgram(&m_particleGraphicProgram);
    }

    void RenderWindow::EndDrawParticles()
    {
        m_particleRenderPass.End();
    }

    void RenderWindow::UseProgram(GraphicsProgram const* program)
    {
        m_RenderTarget.UseProgram(program);
    }

    void RenderWindow::DrawObject(RenderObject3D const &object)
    {
        m_RenderTarget.DrawObject(object);
    }

    void RenderWindow::DrawObjects(RenderObject3DBatched const &object)
    {
        m_RenderTarget.DrawObjects(object);
    }

    void RenderWindow::DrawObject(RenderObject2D const &object)
    {
        m_RenderTarget.DrawObject(object);
    }

    void RenderWindow::DrawText(RenderText2D const & text)
    {
        m_RenderTarget.DrawText(text);
    }

    void RenderWindow::BlitTexture(BlitInformation const &object)
    {
        m_RenderTarget.BlitTexture(object);
    }

    void RenderWindow::DrawParticles(RenderObjectParticle const& object)
    {

        VkDeviceSize offsets[] = { // Offset to add multiple particle component in
            0, 0
        };
        VkBuffer buffers[] = { object.Mesh->GetVertexBuffer().GetBuffer(), object.ComputeProgram_->GetBuffer() };

        vkCmdPushConstants(
                RenderCommands::GetCommandBuffer(),
                m_particleGraphicProgram.GetLayout(),
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(glm::mat4x4),
                object.Transform
            );

        vkCmdBindVertexBuffers(RenderCommands::GetCommandBuffer(), 0, 2, buffers, offsets);

        vkCmdBindIndexBuffer(RenderCommands::GetCommandBuffer(), object.Mesh->GetIndexBuffer().GetBuffer(), 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(RenderCommands::GetCommandBuffer(), object.Mesh->GetIndexCount(), object.ComputeProgram_->GetMaxParticle(), 0, 0, 0);
    }

    void RenderWindow::BeginCalculation()
    {

        vkWaitForFences(RenderDevice::GetDevice(), 1, &m_ComputeInFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

        VkCommandBuffer computeCommandBuffer = RenderCommands::GetComputeCommandBuffer();

        vkResetFences(RenderDevice::GetDevice(), 1, &m_ComputeInFlightFences[m_CurrentFrame]);

        vkResetCommandBuffer(computeCommandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

        RenderCommands::BeginComputeCommands();

    }

    void RenderWindow::EndCalculation()
    {

        RenderCommands::EndComputeCommands();

        VkCommandBuffer computeCommandBuffer = RenderCommands::GetComputeCommandBuffer();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &computeCommandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_ComputeFinishedSemaphores[m_CurrentFrame];

        if (vkQueueSubmit(RenderDevice::GetComputeQueue(), 1, &submitInfo, m_ComputeInFlightFences[m_CurrentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit compute command buffer!");
        };

    }

    void RenderWindow::Calculate(ComputeProgram const *object)
    {

        vkCmdBindPipeline(RenderCommands::GetComputeCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, object->GetPipeline());
        vkCmdBindDescriptorSets(RenderCommands::GetComputeCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, object->GetLayout(), 0, 1, &object->GetDescriptorSet(), 0, 0);

        vkCmdDispatch(RenderCommands::GetComputeCommandBuffer(), std::max(object->GetMaxParticle() / 256.0f, 1.0f), 1, 1);
    }

    void RenderWindow::UpdateCamera(CameraContext &camera)
    {
        m_CameraContext = camera;
    }

    void RenderWindow::UpdateLights(const std::vector<Light*>& lights)
    {
        m_RenderTarget.UpdateLights(lights);
    }

    void RenderWindow::UpdateLight(const Light* light, uint32_t& id)
    {
        m_RenderTarget.UpdateLight(light, id);
    }

    void RenderWindow::UpdateShadows(const std::vector<Light*>& lights, const glm::vec3 camPos, const bool isSceneVillage)
    {
        m_RenderTarget.UpdateShadows(lights, camPos, isSceneVillage);
    }

    void RenderWindow::Update()
    {
        Window::Update();

        glm::mat4x4 view;
        glm::mat4x4 proj;
        float aspect_ratio = static_cast<float>(m_Swapchain.GetExtent2D().width) / static_cast<float>(m_Swapchain.GetExtent2D().height);
        m_CameraContext.GetViewProj(view, proj, aspect_ratio);

        m_RenderTarget.UpdateCamera(view, proj);
    }

    void RenderWindow::EndDrawing()
    {
        VkCommandBuffer commandBuffer = RenderCommands::GetCommandBuffer();
        RenderCommands::EndCommands();

        VkSubmitInfo submitInfo{};

        VkSemaphore waitSemaphores[] = {
            m_ImageAvailableSemaphores[m_CurrentFrame],
            m_ComputeFinishedSemaphores[m_CurrentFrame]
        };
        VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
        };

        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 2;
        submitInfo.pWaitSemaphores    = waitSemaphores;
        submitInfo.pWaitDstStageMask  = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;

        VkSemaphore signalSemaphores[] = {
            m_RenderFinishedSemaphores[m_ImageIndex]
        };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        if (vkQueueSubmit(RenderDevice::GetGraphicQueue(), 1, &submitInfo,
                          m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;
        VkSwapchainKHR swapChains[]    = {m_Swapchain.GetSwapchain()};
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = swapChains;
        presentInfo.pImageIndices      = &m_ImageIndex;
        presentInfo.pResults           = nullptr;

        VkResult result = vkQueuePresentKHR(RenderDevice::GetGraphicQueue(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || GetIsResized()) {
            GetIsResized() = false;
            RecreateSwapchain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_GENERATED_FRAME;
    }

    ErrorType RenderWindow::CreateSyncObjects()
    {

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        m_RenderFinishedSemaphores.resize(m_Swapchain.GetImageCount());
        for (size_t i = 0; i < m_Swapchain.GetImageCount(); i++)
        {
            if (vkCreateSemaphore(RenderDevice::GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS)
            {
                return ErrorType::VULKAN_RENDER_SYNCHRONIZATION_CREATION;
            }
        }

        for (size_t i = 0; i < MAX_GENERATED_FRAME; i++) {
            if (vkCreateSemaphore(RenderDevice::GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(RenderDevice::GetDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS) {
                return ErrorType::VULKAN_RENDER_SYNCHRONIZATION_CREATION;
            }
            if (vkCreateSemaphore(RenderDevice::GetDevice(), &semaphoreInfo, nullptr, &m_ComputeFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(RenderDevice::GetDevice(), &fenceInfo, nullptr, &m_ComputeInFlightFences[i]) != VK_SUCCESS) {
                return ErrorType::VULKAN_RENDER_SYNCHRONIZATION_CREATION;
            }
        }

        return ErrorType::SUCCESS;

    }

    void RenderWindow::RecreateSwapchain()
    {

        // TODO pause app when minimized
        int width = 0, height = 0;
        glfwGetFramebufferSize(mp_Window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(mp_Window, &width, &height);
            glfwWaitEvents();
        }

        RenderDevice::WaitIdle();

        m_RenderTarget.DestroyTextures();
        m_ParticleDepthTexture->Destroy();

        m_Swapchain.Destroy();

        GPC_SUCCESS(m_Swapchain.Create(this));

        m_Size.x = m_Swapchain.GetExtent2D().width;
        m_Size.y = m_Swapchain.GetExtent2D().height;

        RenderTargetInformation information;
        information.Format      = m_Swapchain.GetFormat();
        information.Width       = GetSize().x;
        information.Height      = GetSize().y;
        information.UseMSAA     = true;
        information.UseDepthBuffer = true;
        information.Layout      = ImageLayoutType::PRESENT_SRC;
        information.Swapchain_   = &m_Swapchain;

        m_RenderTarget.Resize(information);

        CreateParticleDepth();
        m_particleRenderPass.Resize(m_Swapchain.GetExtent2D());

    }

    void RenderWindow::CreateComputeProgramLayer()
    {

        CreateParticleDepth();

        Attachment colorAttachment(AttachmentType::COLOR);
        colorAttachment
            .Format(m_Swapchain.GetFormat())
            .BindTextures(m_RenderTarget.GetImages() )
            .PassLayout(ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL, ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL)
            .UseLayout(ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL)
            .UseClearValue( { .color = {0.0f, 0.0f, 0.0f, 1.0f}})
            .SampleCount(VK_SAMPLE_COUNT_1_BIT)
            .KeepPreviousColor(true);

        Attachment depthAttachment(AttachmentType::DEPTH_STENCIL);
        depthAttachment
            .Format( RenderDevice::GetDepthStencilFormat() )
            .BindTexture( &m_ParticleDepthTexture )
            .PassLayout(ImageLayoutType::UNDEFINED, ImageLayoutType::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            .UseLayout(ImageLayoutType::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            .SampleCount(VK_SAMPLE_COUNT_1_BIT)
            .UseClearValue( { .depthStencil = {1.0f, 0} } )
            .KeepDepthBuffer(false);

        m_particleRenderPass.BindAttachment(colorAttachment);
        m_particleRenderPass.BindAttachment(depthAttachment);

        RenderPassInformation renderPassInfo{};
        renderPassInfo.Name    = "RENDER_PASS_PARTICLE";
        renderPassInfo.Extent  = m_Swapchain.GetExtent2D();
        renderPassInfo.Offset  = {0, 0};

        m_particleRenderPass.Create(renderPassInfo);
        m_particleRenderPass.Resize(m_Swapchain.GetExtent2D());

        Shader fParticles;
        Shader vParticles;
        fParticles.Create("particles.frag", GPC::Shader::FRAGMENT);
        vParticles.Create("particles.vert", GPC::Shader::VERTEX);

        ProgramInformation pInformation;
        pInformation.Parameters.CullMode_ = CullMode::FRONT;
        pInformation.Parameters.PolygoneMode = PolygonMode::MODE_FILL;
        pInformation.RenderPass_ = &m_particleRenderPass;
        pInformation.DepthWriteEnable = true;

        pInformation.Shaders = {
            &fParticles,
            &vParticles
        };

        m_particleGraphicProgram.BindLayout(DescriptorSetInformation::ForCamera(0));

        m_particleGraphicProgram.BindVertexInput({
                    .BindingDescription = GPC::Vertex::GetBindingDescription(),
                    .AttributeDescriptions = GPC::Vertex::GetAttributeDescriptions(),
                });

        m_particleGraphicProgram.BindVertexInput({
                    .BindingDescription = GPC::ComputeParticle::GetBindingDescription(),
                    .AttributeDescriptions = GPC::ComputeParticle::GetAttributeDescriptions(),
                } );

        m_particleGraphicProgram.Create(pInformation);
        m_particleGraphicProgram.InitializeWith({}, {});

        vParticles.Destroy();
        fParticles.Destroy();
    }

    void RenderWindow::CreateParticleDepth()
    {
        TextureCreationInformation information;
        information.Name = "DEPTH_PARTICLE_TEXTURE";
        information.ImageInformation.Format     = RenderDevice::GetDepthStencilFormat();
        information.ImageInformation.Usage      = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        information.ImageInformation.Width      = m_Swapchain.GetExtent2D().width;
        information.ImageInformation.Height     = m_Swapchain.GetExtent2D().height;
        information.ImageInformation.NumSample  = VK_SAMPLE_COUNT_1_BIT;
        information.ImageInformation.Tiling     = VK_IMAGE_TILING_OPTIMAL;
        information.ImageInformation.Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        information.ViewInformation.Format      = RenderDevice::GetDepthStencilFormat();
        information.ViewInformation.Flags       = VK_IMAGE_ASPECT_DEPTH_BIT;

        m_ParticleDepthTexture = new Texture();
        m_ParticleDepthTexture->Create(information);
    }
} // GPC