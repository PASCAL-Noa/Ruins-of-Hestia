
#include "RenderTarget.h"

namespace GPC
{

    RenderTarget::~RenderTarget(){}

    ErrorType RenderTarget::Create(RenderTargetInformation const &information)
    {

        m_3DCameraBuffer.ViewProj = glm::mat4(1.0f);
        m_3DCameraBuffer.Proj = glm::mat4(1.0f);
        m_3DCameraBuffer.View = glm::mat4(1.0f);

        m_2DCameraBuffer.ViewProj = glm::mat4(1.0f);
        m_2DCameraBuffer.Proj = glm::mat4(1.0f);
        m_2DCameraBuffer.View = glm::mat4(1.0f);

        // m_format = infos.format;
        m_Extent = VkExtent2D( information.Width, information.Height );
        m_Offset = VkOffset2D( information.OffsetX, information.OffsetY );
        m_RenderInformation = information;

        if (information.Format == VK_FORMAT_UNDEFINED) {
            m_RenderInformation.Format = RenderDevice::FindImageSupportedFormat(
                { VK_FORMAT_B8G8R8A8_SRGB  },
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT
            );
        }

        for (int i = 0; i < MAX_GENERATED_FRAME; i++) {
            m_Images[i] = new Texture*();
            *m_Images[i] = information.Swapchain_->GetTexture(i);
        }

        ErrorType error = ErrorType::SUCCESS;

        error = CreateShadowRenderPass();
        GPC_SUCCESS(error);

        error = CreateRenderPass();
        GPC_SUCCESS(error);

        Create2DFeatures(); // TODO GPC Success missing ?

        error = CreateHDRResources();
        GPC_SUCCESS(error);

        error = CreatePostEffectResources();
        GPC_SUCCESS(error);

        error = CreateBatchingResources();
        GPC_SUCCESS(error);

        Resize(m_RenderInformation);
        return error;
    }


    void RenderTarget::Destroy()
    {
        m_DirShadowRenderPass.Destroy();
        m_SpotShadowRenderPass.Destroy();
        m_PointShadowRenderPass.Destroy();

        m_RenderPass3D.Destroy();
        m_RenderPass2D.Destroy();

        m_SpriteSampler.Destroy();
        m_TextSampler.Destroy();
        m_2DGeometry.Destroy();

        m_2DBaseShader[0].Destroy();
        m_2DBaseShader[1].Destroy();
        m_2DBaseShader[2].Destroy();
        m_2DBaseShader[3].Destroy();

        m_2DSpriteBaseProgram.Destroy();
        m_2DTextBaseProgram.Destroy();

        for (uint8_t i = 0; i < MAX_GENERATED_FRAME; ++i)
        {
            m_2DSpriteBuffer[i].Unmap();
            m_2DSpriteBuffer[i].Destroy();

            m_2DTextBuffer[i].Unmap();
            m_2DTextBuffer[i].Destroy();
        }

        m_Batched3DObjectBuffer.Unmap();
        m_Batched3DObjectBuffer.Destroy();

        if (m_RenderInformation.Swapchain_ == nullptr) {
            for (Texture** image : m_Images)
                (*image)->Destroy();
        }

        m_DepthStencilTexture->Destroy();

        m_MsaaTexture->Destroy();

    }


    void RenderTarget::BeginDraw3D()
    {
        m_EntityDrawIndex = 0;

        m_RenderPass3D.Begin();
    }



    void RenderTarget::EndDraw3D()
    {
        m_RenderPass3D.End();
        UpdateProgramValues();

        DrawHDRObjects();

        ApplyPostEffects();

    }


    void RenderTarget::BeginDirShadows(const ShadowsUpdateInformation& information, const std::vector<Light*>& lights, const glm::vec3 camPos, const bool isSceneVillage)
    {
        information.RenderPass_->Begin();

        //DEBUG DIRECTIONAL


        for (int i = 0; i <lights.size(); i++)
        {
            if (lights[i] == nullptr)
                continue;

            if (lights[i]->GetType() != static_cast<uint32_t>(Light::LightType::DIRECTIONAL))
                continue;

            const glm::vec3 direction = lights[i]->GetDirection();

            const glm::vec3 lightDir = glm::normalize(direction);

            glm::vec3 up;
            if (glm::abs(glm::dot(lightDir, glm::vec3(0, 1, 0))) > 0.999f)
                up = glm::vec3(1, 0, 0);
            else
                up = glm::vec3(0, 1, 0);

            const auto sceneCenter = glm::vec3(camPos.x, 0.0f, camPos.z);

            constexpr float pullBack    = 1;

            const glm::vec3 lightPos = sceneCenter - lightDir * pullBack;
            const glm::mat4 lightView = glm::lookAt(lightPos, sceneCenter, up);

            glm::mat4 lightProj;

            if (isSceneVillage)
            {
                lightProj = glm::ortho(200.0f,  700.0f, -700.0f,  -200.0f, -400.0f, 400.0f);
            }
            else
            {
                lightProj = glm::ortho(-750.0f, -50.0f, -900.0f, -200.0f, -500.0f, 1000.0f);
            }

            lightProj[1][1] *= -1;

            m_DirTransformBuffer.Transform[i] = lightProj * lightView;

            m_HasDirectionalLight = true;

            break;
        }

        information.Program->Update( information.Program->GetBindingSlot("LightTransform"), &m_DirTransformBuffer, sizeof(m_DirTransformBuffer), 0);
    }

    void RenderTarget::EndDirShadows(const ShadowsUpdateInformation& information)
    {
        m_EntityDrawIndex = 0;


        for (int i = 0; i < m_shadowObjectsCount; i++)
        {
            auto& shadow = m_shadowObjects[i];

            ShadowPushConstant pushConst = {
                static_cast<uint32_t>(m_EntityDrawIndex),
                0,
                shadow.Material_->TextureArray,
                shadow.Material_->Diffuse
            };

            vkCmdPushConstants(
                RenderCommands::GetCommandBuffer(),
                information.Program->GetLayout(),
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
                0,
                sizeof(ShadowPushConstant),
                &pushConst
            );

            information.Program->Update(
                information.Program->GetBindingSlot("transforms"),
                &shadow.Transform,
                sizeof(glm::mat4x4),
                m_EntityDrawIndex
            );

            const VkCommandBuffer commandBuffer = RenderCommands::GetCommandBuffer();
            const VkBuffer vertexBuffers[] = { shadow.VertexBuffer };
            constexpr VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, shadow.IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(commandBuffer, shadow.Index, 1, 0, 0, 0);

            m_EntityDrawIndex++;

        }

        information.RenderPass_->End();
    }


    void RenderTarget::BeginSpotShadows(ShadowsUpdateInformation& information, std::vector<Light*> lights, glm::vec3 camPos)
    {
        information.RenderPass_->Begin();

        //DEBUG SPOT
        static float Angle = 0.0f;
        Angle += 0.00016 * 1.5f;

        int index = 0;

        for (int i = 0; i < lights.size(); i++)
        {
            if (lights[i] == nullptr)
                continue;

            if (lights[i]->GetType() != static_cast<uint32_t>(Light::LightType::SPOT))
                continue;

            glm::vec3 position  = lights[i]->GetPosition();
            const glm::vec3 direction = lights[i]->GetDirection();

            constexpr float nearPlane = 0.001f;

            glm::mat4 lightView;
            glm::mat4 lightProj;

            glm::vec3 spotDir = glm::normalize(direction);
            //glm::vec3 spotDir = glm::normalize(glm::vec3( cos(Angle), sin(Angle), 0.2f));
            //lights[i]->SetDirection({spotDir.x, spotDir.y, spotDir.z, 1.0f});

            glm::vec3 worldUp = glm::vec3(0, 1, 0);

            if (glm::abs(glm::dot(spotDir, worldUp)) > 0.999f)
                worldUp = glm::vec3(0, 0, 1);

            glm::vec3 right = glm::normalize(glm::cross(worldUp, spotDir));
            glm::vec3 up    = glm::normalize(glm::cross(spotDir, right));

            lightView = glm::lookAt(position, position + spotDir, up);

            const float farPlane = lights[i]->GetRange() * 2.0f;

            const float fov    = lights[i]->GetOuterCone() * 8.0f;
            const float aspect = static_cast<float>(m_shadowMapResolution.width) / static_cast<float>(m_shadowMapResolution.height);

            lightProj = glm::perspective(fov, aspect, nearPlane, farPlane);

            lightProj[1][1] *= -1;

            const glm::mat4 lightSpaceMatrix = lightProj * lightView;

            m_SpotTransformBuffer.Transform[index] = lightSpaceMatrix;

            index++;
        }

        information.Program->Update( information.Program->GetBindingSlot("LightTransform"), &m_SpotTransformBuffer, sizeof(m_SpotTransformBuffer), 0);
    }


    void RenderTarget::EndSpotShadows(const ShadowsUpdateInformation& information)
    {
        m_EntityDrawIndex = 0;

        for (int i = 0; i < m_shadowObjectsCount; i++)
        {
            auto& shadow = m_shadowObjects[i];
            ShadowPushConstant pushConst = { static_cast<uint32_t>(m_EntityDrawIndex), 0 };

            vkCmdPushConstants(
                RenderCommands::GetCommandBuffer(),
                information.Program->GetLayout(),
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
                0,
                sizeof(ShadowPushConstant),
                &pushConst
            );

            information.Program->Update(
                information.Program->GetBindingSlot("transforms"),
                &shadow.Transform,
                sizeof(glm::mat4x4),
                m_EntityDrawIndex
            );

            const VkCommandBuffer commandBuffer = RenderCommands::GetCommandBuffer();
            const VkBuffer vertexBuffers[] = { shadow.VertexBuffer };
            constexpr VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, shadow.IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(commandBuffer, shadow.Index, 1, 0, 0, 0);

            m_EntityDrawIndex++;
        }

        information.RenderPass_->End();
    }


    void RenderTarget::BeginPointShadows(const ShadowsUpdateInformation& information, const std::vector<Light*>& lights, glm::vec3 camPos)
    {
        information.RenderPass_->Begin();

        uint32_t pointIndex = 0;

        for (const auto & light : lights)
        {
            if (light == nullptr)
                continue;

            if (light->GetType() != static_cast<uint32_t>(Light::LightType::POINT))
                continue;

            glm::vec3 pos           = light->GetPosition();
            const float farPlane    = light->GetRange() * 2.0f;

            glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, farPlane);

            glm::mat4 faceViews[6] = {
                glm::lookAt(pos, pos + glm::vec3( 1, 0, 0), glm::vec3(0,-1, 0)),
                glm::lookAt(pos, pos + glm::vec3(-1, 0, 0), glm::vec3(0,-1, 0)),
                glm::lookAt(pos, pos + glm::vec3( 0, 1, 0), glm::vec3(0, 0, 1)),
                glm::lookAt(pos, pos + glm::vec3( 0,-1, 0), glm::vec3(0, 0,-1)),
                glm::lookAt(pos, pos + glm::vec3( 0, 0, 1), glm::vec3(0,-1, 0)),
                glm::lookAt(pos, pos + glm::vec3( 0, 0,-1), glm::vec3(0,-1, 0)),
            };

            for (int f = 0; f < 6; f++)
            {
                m_PointShadowBuffer.Transform[pointIndex * 6 + f] = proj * faceViews[f];
            }

            pointIndex++;
        }

        information.Program->Update(
            information.Program->GetBindingSlot("PointLightPositions"),
            &m_PointShadowBuffer,
            sizeof(m_PointShadowBuffer),
            0
        );
    }


    void RenderTarget::EndPointShadows(const ShadowsUpdateInformation& information, const std::vector<Light*>& lights)
    {
        uint32_t pointIndex = 0;

        for (const auto light : lights)
        {
            if (light == nullptr)
                continue;

            if (light->GetType() != static_cast<uint32_t>(Light::LightType::POINT))
                continue;

            m_EntityDrawIndex = 0;

            for (int i = 0; i < m_shadowObjectsCount; i++)
            {
                auto& shadow = m_shadowObjects[i];
                float farPlane = light->GetRange() * 2.0f;

                ShadowPointPushConstant pushConst = {
                    static_cast<uint32_t>(m_EntityDrawIndex),
                    (pointIndex * 6),
                    farPlane,
                    0.0f,
                    light->GetPosition()
                };

                vkCmdPushConstants(
                    RenderCommands::GetCommandBuffer(),
                    information.Program->GetLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(ShadowPointPushConstant),
                    &pushConst
                );

                information.Program->Update(
                    information.Program->GetBindingSlot("transforms"),
                    &shadow.Transform,
                    sizeof(glm::mat4x4),
                    m_EntityDrawIndex
                );

                const VkCommandBuffer commandBuffer = RenderCommands::GetCommandBuffer();
                const VkBuffer vertexBuffers[] = { shadow.VertexBuffer };
                constexpr VkDeviceSize offsets[] = { 0 };

                vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
                vkCmdBindIndexBuffer(commandBuffer, shadow.IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
                vkCmdDrawIndexed(commandBuffer, shadow.Index, 1, 0, 0, 0);

                m_EntityDrawIndex++;
            }

            pointIndex++;

        }

        information.RenderPass_->End();
    }


    void RenderTarget::BeginDraw2D()
    {
        m_RenderPass2D.Begin();
        m_EntityDrawIndex = 0;

        auto w = static_cast<float>(m_Extent.width) ;
        auto h = static_cast<float>(m_Extent.height);

        m_2DCameraBuffer.View = {
            w, h, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0
        };
        m_2DCameraBuffer.Proj = glm::ortho(w, 0.0f, 0.0f, h, -1.0f, 1.0f);
        m_2DCameraBuffer.ViewProj = glm::mat4(1.0f);

        m_2DSpriteBaseProgram.Update(m_2DSpriteBaseProgram.GetBindingSlot("camera"), &m_2DCameraBuffer, sizeof(m_2DCameraBuffer), 0);
        m_2DTextBaseProgram.Update(m_2DTextBaseProgram.GetBindingSlot("camera"), &m_2DCameraBuffer, sizeof(m_2DCameraBuffer), 0);
    }


    void RenderTarget::EndDraw2D()
    {
        m_DrawTexturesSprite.clear();
        m_DrawTexturesText  .clear();

        if (m_DrawSprite.size() > 0) {

            UseProgram(&m_2DSpriteBaseProgram);

            m_DrawTexturesSprite.resize(m_DrawSprite.size());
            for (auto info : m_DrawSprite)
                m_DrawTexturesSprite[info.second] = info.first;

            m_2DSpriteBaseProgram.SetImages(m_DrawTexturesSprite, m_SpriteSampler);

            VkCommandBuffer commandBuffer = RenderCommands::GetCommandBuffer();
            const uint32_t frame = RenderCommands::GetCurrentFrame();

            memcpy(mp_2DMappedSprite[frame], m_2DDrawSprite.data(), m_2DDrawSpriteCount * sizeof(Instance2D));

            VkBuffer vertexBuffers[] = {
                m_2DGeometry.GetVertexBuffer().GetBuffer(),
                m_2DSpriteBuffer[frame].GetBuffer()
            };

            VkDeviceSize offsets[] = {0, 0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, m_2DGeometry.GetIndexBuffer().GetBuffer(), 0, VK_INDEX_TYPE_UINT16);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_2DSpriteBaseProgram.GetLayout(), 0, 1,
                &m_2DSpriteBaseProgram.GetDescriptorSet(), 0, nullptr);

            vkCmdDrawIndexed(commandBuffer,
                m_2DGeometry.GetIndexCount(),
                m_2DDrawSpriteCount, 0, 0, 0);
        }

        if (m_DrawText.size() > 0) {

            UseProgram(&m_2DTextBaseProgram);

            m_DrawTexturesText.resize(m_DrawText.size());
            for (auto info : m_DrawText)
                m_DrawTexturesText[info.second] = info.first;

            m_2DTextBaseProgram.SetImages(m_DrawTexturesText, m_TextSampler);

            VkCommandBuffer commandBuffer = RenderCommands::GetCommandBuffer();
            const uint32_t frame = RenderCommands::GetCurrentFrame();

            memcpy(mp_2DMappedText[frame], m_2DDrawText.data(), m_2DDrawTextCount * sizeof(Instance2D));

            VkBuffer vertexBuffers[] = {
                m_2DGeometry.GetVertexBuffer().GetBuffer(),
                m_2DTextBuffer[frame].GetBuffer()
            };

            VkDeviceSize offsets[] = {0, 0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, m_2DGeometry.GetIndexBuffer().GetBuffer(), 0, VK_INDEX_TYPE_UINT16);

            vkCmdDrawIndexed(commandBuffer,
                m_2DGeometry.GetIndexCount(),
                m_2DDrawTextCount, 0, 0, 0);
        }

        m_RenderPass2D.End();

        m_2DDrawTextCount = 0;
        m_2DDrawSpriteCount = 0;

        m_DrawSprite.clear();
        m_DrawText.clear();

        m_UsedPrograms.clear();

        m_NextSpriteIndex = 0;
        m_NextTextIndex = 0;
    }


    void RenderTarget::UsePostEffect(GraphicsProgram const *program)
    {
        m_PostEffectProgram.push_back(program);
    }


    void RenderTarget::UseProgram(GraphicsProgram const *program)
    {
        m_UsedPrograms.emplace(program);

        vkCmdBindPipeline(RenderCommands::GetCommandBuffer(),
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            program->GetPipeline()
            );

        vkCmdBindDescriptorSets(RenderCommands::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, program->GetLayout(), 0, 1, &program->GetDescriptorSet(), 0, nullptr);

    }


    void RenderTarget::EndPrograms()
    {
        m_UsedPrograms.clear();
    }


    void RenderTarget::DrawObjects(RenderObject3DBatched const &object)
    {

        UseProgram(object.GraphicProgram);

        VkCommandBuffer commandBuffer = RenderCommands::GetCommandBuffer();

        m_3DConstantBuffer.ObjectID = m_EntityDrawIndex;
        for (auto const* program : m_UsedPrograms) {
            vkCmdPushConstants(
                RenderCommands::GetCommandBuffer(),
                program->GetLayout(),
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(Object3DConstants),
                &m_3DConstantBuffer
            );

            // TODO : Set correct textures array
            program->Update( program->GetBindingSlot("materials"),  object.Material_,    sizeof(Material),      m_EntityDrawIndex );
        }

        memcpy(mp_MappedBatched3DObject, object.BatchedObjects, object.EntityCount * sizeof(Batched3DObject));

        VkBuffer vertexBuffers[] = {
            object.Mesh->GetVertexBuffer().GetBuffer(),
            m_Batched3DObjectBuffer.GetBuffer()
        };

        VkDeviceSize offsets[] = {0, 0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, object.Mesh->GetIndexBuffer().GetBuffer(), 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(commandBuffer, object.Mesh->GetIndexCount(), object.EntityCount, 0, 0, 0);
        m_EntityDrawIndex++;
    }


    void RenderTarget::DrawObject(RenderObject3D const &object)
    {
        UseProgram(object.GraphicProgram);

        if (object.UseShadow)
        {
            DrawObjectInformation info = {};
            info.Transform = *object.Transform;
            info.VertexBuffer = object.Mesh->GetVertexBuffer().GetBuffer();
            info.IndexBuffer = object.Mesh->GetIndexBuffer().GetBuffer();
            info.Index = object.Mesh->GetIndexCount();
            info.Material_ = object.Material_;

            m_shadowObjects[m_shadowObjectsCount] = info;
            if (m_DirTextures.empty()) {
                m_DirTextures = object.GraphicProgram->GetImageInfos();
                m_DirShadowProgram.InitializeWith(m_DirTextures);
            }
            m_shadowObjectsCount++;
        }

        if (object.Material_->EmissiveStrength > 0) {
            DrawObjectInformation info = {};
            info.Transform = *object.Transform;
            info.VertexBuffer = object.Mesh->GetVertexBuffer().GetBuffer();
            info.IndexBuffer = object.Mesh->GetIndexBuffer().GetBuffer();
            info.Index = object.Mesh->GetIndexCount();
            info.Material_ = object.Material_;
            m_HDRObjects[m_HDRObjectsCount] = info;
            m_HDRObjectsCount++;
        }

        m_3DConstantBuffer.ObjectID = m_EntityDrawIndex;
        for (auto const* program : m_UsedPrograms) {
            vkCmdPushConstants(
                RenderCommands::GetCommandBuffer(),
                program->GetLayout(),
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(Object3DConstants),
                &m_3DConstantBuffer
            );

            // TODO : Set correct textures array
            program->Update( program->GetBindingSlot("transforms"),    object.Transform,   sizeof(glm::mat4x4),   m_EntityDrawIndex );
            program->Update( program->GetBindingSlot("materials"),  object.Material_,    sizeof(Material),      m_EntityDrawIndex );
        }
        VkCommandBuffer commandBuffer = RenderCommands::GetCommandBuffer();

        VkBuffer vertexBuffers[] = {
            object.Mesh->GetVertexBuffer().GetBuffer()
        };

        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, object.Mesh->GetIndexBuffer().GetBuffer(), 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(commandBuffer, object.Mesh->GetIndexCount(), 1, 0, 0, 0);

        m_EntityDrawIndex++;
    }


    void RenderTarget::DrawObject(RenderObject2D const &object)
    {

        int32_t index = -1;
        if (!m_DrawSprite.contains(object.Texture_)) {
            index = m_NextSpriteIndex++;
            m_DrawSprite.emplace(object.Texture_, index);
        } else {
            index = m_DrawSprite[object.Texture_];
        }

        glm::vec2 position = object.Position;
        position.x += object.Size.x * 0.5f;
        position.y += object.Size.y;

        m_2DDrawSprite[m_2DDrawSpriteCount++] = {
            object.Tint,
            position,
            object.Size,
            static_cast<uint32_t>(index),
            object.Material_->Diffuse,
            object.Rotation
        };

        m_EntityDrawIndex++;

    }

    std::vector<uint32_t> Utf8ToCodepoints(std::string_view str)
    {
        std::vector<uint32_t> codepoints;
        for (size_t i = 0; i < str.size(); )
        {
            uint32_t cp = 0;
            uint8_t  c  = static_cast<uint8_t>(str[i]);

            if (c < 0x80)
            {
                cp = c;
                i += 1;
            }
            else if ((c & 0xE0) == 0xC0)
            {
                cp = c & 0x1F;
                i++;
                cp = (cp << 6) | (static_cast<uint8_t>(str[i]) & 0x3F);
                i++;
            }
            else if ((c & 0xF0) == 0xE0)
            {
                cp = c & 0x0F;
                i++;
                cp = (cp << 6) | (static_cast<uint8_t>(str[i]) & 0x3F);
                i++;
                cp = (cp << 6) | (static_cast<uint8_t>(str[i]) & 0x3F);
                i++;
            }
            else if ((c & 0xF8) == 0xF0)
            {
                cp = c & 0x07;
                i++;
                cp = (cp << 6) | (static_cast<uint8_t>(str[i]) & 0x3F);
                i++;
                cp = (cp << 6) | (static_cast<uint8_t>(str[i]) & 0x3F);
                i++;
                cp = (cp << 6) | (static_cast<uint8_t>(str[i]) & 0x3F);
                i++;
            }
            else
            {
                i++;
                continue;
            }

            codepoints.push_back(cp);
        }
        return codepoints;
    }

    void RenderTarget::DrawText(RenderText2D const & text)
    {
        auto font = text.Font_;
        m_TextMaterial.TextureArray = font->GetTextureArray().GetUploadId();
        Texture const* pointer = &font->GetTextureArray();

        int32_t index;
        if (!m_DrawText.contains(pointer)) {
            index = m_NextTextIndex++;
            m_DrawText.emplace(pointer, index);
        } else {
            index = m_DrawText[pointer];
        }

        glm::vec2 position = text.Position;

        for (const uint32_t cp : Utf8ToCodepoints(text.Text))
        {
            Character cData = font->GetChar(cp);

            auto h                                      = static_cast<float>(cData.Size.y);
            const float xpos                            = position.x + static_cast<float>(cData.Bearing.x) * text.Scale;
            constexpr float VALEUR_SORTIE_DU_CHAPEAU    = 0.75f;
            float ypos                                  = position.y + (static_cast<float>(cData.Size.y) - static_cast<float>(cData.Bearing.y) + static_cast<float>(font->GetMaxHeight()) * VALEUR_SORTIE_DU_CHAPEAU - h) * text.Scale;

            glm::vec2 size                              = { static_cast<float>(font->GetMaxWidth())  * text.Scale, static_cast<float>(font->GetMaxHeight()) * text.Scale };
            glm::vec2 finalPosition                     = { xpos + size.x * 0.5f, ypos + size.y };

            m_2DDrawText[m_2DDrawTextCount++]        = {
                text.Tint,
                finalPosition,
                size,
                static_cast<uint32_t>(index),
                cData.TextureID_
            };

            position.x += static_cast<float>(cData.AdvanceWidth >> 6) * text.Scale;
        }
    }


    void RenderTarget::BlitTexture(BlitInformation const& informations)
    {

        VkCommandBuffer commandBuffer = RenderCommands::GetCommandBuffer();

        VkImageBlit blit {};
        blit.srcOffsets[0].x    = informations.SourceStart.x;
        blit.srcOffsets[0].y    = informations.SourceStart.y;
        blit.srcOffsets[1].x    = informations.SourceEnd.x;
        blit.srcOffsets[1].y    = informations.SourceEnd.y;

        blit.dstOffsets[0].x    = informations.DestStart.x;
        blit.dstOffsets[0].y    = informations.DestStart.y;
        blit.dstOffsets[1].x    = informations.DestStart.x;
        blit.dstOffsets[1].y    = informations.DestStart.y;

        blit.srcSubresource                 = informations.TextureInformation;
        blit.dstSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.baseArrayLayer  = 0;
        blit.dstSubresource.layerCount      = 1;
        blit.dstSubresource.mipLevel        = 1;

        vkCmdBlitImage(commandBuffer,
            informations.Texture_->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED,
            (*GetImage()).GetImage(), static_cast<VkImageLayout>(m_RenderInformation.Layout), 1, &blit, VK_FILTER_LINEAR);

    }


    void RenderTarget::UpdateCamera(glm::mat4 const& View, glm::mat4 const& Proj)
    {
        m_3DCameraBuffer.Proj = Proj;
        m_3DCameraBuffer.View = View;
        m_3DCameraBuffer.ViewProj = m_3DCameraBuffer.Proj * m_3DCameraBuffer.View;
    }


    void RenderTarget::UpdateLights(const std::vector<Light*>& lights) {


        m_LightBuffer.lightCount = lights.size();

        for (size_t i = 0; i < lights.size(); i++)
        {
            Light* lightObj = lights[i];
            auto& light = m_LightBuffer.LightInfo[i];

            light.Type = lightObj->GetType();
            light.Color = lightObj->GetColor();
            light.Ambient = lightObj->GetAmbient();

            switch (light.Type)
            {
                case static_cast<uint32_t>(Light::LightType::POINT):
                {
                    const auto* point = dynamic_cast<PointLight*>(lightObj);
                    light.Position = point->GetPosition();
                    light.Range = point->GetRange();
                    break;
                }

                case static_cast<uint32_t>(Light::LightType::DIRECTIONAL):
                {
                    const auto* dir = dynamic_cast<DirectionalLight*>(lightObj);
                    light.Direction = dir->GetDirection();
                    break;
                }

                case static_cast<uint32_t>(Light::LightType::SPOT):
                {
                    const auto* spot = dynamic_cast<SpotLight*>(lightObj);
                    light.Position = spot->GetPosition();
                    light.Direction = spot->GetDirection();
                    light.Range = spot->GetRange();
                    light.InnerCone = spot->GetInnerCone();
                    light.OuterCone = spot->GetOuterCone();
                    break;
                }
                default:
                    break;
            }
        }
    }


    void RenderTarget::UpdateLight(const Light* light, const uint32_t id)
    {
        m_LightBuffer.lightCount = light->GetLightCount();

        auto& lightInfo = m_LightBuffer.LightInfo[id];

        lightInfo.Type = light->GetType();
        lightInfo.Color = light->GetColor();
        lightInfo.Ambient = light->GetAmbient();
        lightInfo.Position = light->GetPosition();
        lightInfo.Direction = light->GetDirection();
        lightInfo.Range = light->GetRange();
        lightInfo.InnerCone = light->GetInnerCone();
        lightInfo.OuterCone = light->GetOuterCone();
        lightInfo.Intensity = light->GetIntensity();
    }

    void RenderTarget::UpdateShadows(const std::vector<Light*>& lights, const glm::vec3 camPos, bool isSceneVillage)
    {

        {
            BeginDirShadows(m_shadowUpdateInformations[0], lights, camPos, isSceneVillage);

            vkCmdBindPipeline(RenderCommands::GetCommandBuffer(),
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_shadowUpdateInformations[0].Program->GetPipeline()
                );

            vkCmdBindDescriptorSets(
                RenderCommands::GetCommandBuffer(),
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_shadowUpdateInformations[0].Program->GetLayout(),
                0,
                1,
                &m_shadowUpdateInformations[0].Program->GetDescriptorSet(),
                0,
                nullptr
                );

            EndDirShadows(m_shadowUpdateInformations[0]);
        }

        {
            BeginSpotShadows(m_shadowUpdateInformations[1], lights, camPos);

            vkCmdBindPipeline(RenderCommands::GetCommandBuffer(),
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_shadowUpdateInformations[1].Program->GetPipeline()
                );

            vkCmdBindDescriptorSets(
                RenderCommands::GetCommandBuffer(),
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_shadowUpdateInformations[1].Program->GetLayout(),
                0,
                1,
                &m_shadowUpdateInformations[1].Program->GetDescriptorSet(),
                0,
                nullptr
                );

            EndSpotShadows(m_shadowUpdateInformations[1]);
        }

        if (m_HasDirectionalLight)
        {
            m_DirSpotTransformBuffer.Transform[0] = m_DirTransformBuffer.Transform[0];
        }

        for (int i = m_HasDirectionalLight; i < MAX_LIGHTS; i++)
        {
            if (m_HasDirectionalLight)
            {

                m_DirSpotTransformBuffer.Transform[i] = m_SpotTransformBuffer.Transform[i - 1];
            }
            else
            {
                m_DirSpotTransformBuffer.Transform[i] = m_SpotTransformBuffer.Transform[i];
            }
        }

        {
            BeginPointShadows(m_shadowUpdateInformations[2], lights, camPos);

            vkCmdBindPipeline(RenderCommands::GetCommandBuffer(),
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_shadowUpdateInformations[2].Program->GetPipeline()
                );

            vkCmdBindDescriptorSets(
                RenderCommands::GetCommandBuffer(),
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_shadowUpdateInformations[2].Program->GetLayout(),
                0,
                1,
                &m_shadowUpdateInformations[2].Program->GetDescriptorSet(),
                0,
                nullptr
                );

            EndPointShadows(m_shadowUpdateInformations[2], lights);
        }

        m_shadowObjectsCount = 0;
    }


    void RenderTarget::CreateTextures()
    {

        if (m_RenderInformation.UseMSAA) {
            m_MsaaSampleCount = RenderDevice::GetSampleCountFlag();
            GPC_SUCCESS(CreateMsaaResources());
        }

        if (m_RenderInformation.UseDepthBuffer) {
            m_DepthStencilFormat = RenderDevice::GetDepthStencilFormat();
            GPC_SUCCESS(CreateDepthResources());
        }

        CreateHDRTexture();

    }


    void RenderTarget::DestroyTextures()
    {
        m_DepthStencilTexture->Destroy();
        m_MsaaTexture->Destroy();

        m_RenderPass3D.DestroyFramebuffer();
        m_RenderPass2D.DestroyFramebuffer();

    }


    void RenderTarget::Resize(RenderTargetInformation& information)
    {
        m_RenderInformation = information;
        m_Extent = VkExtent2D( information.Width, information.Height );
        m_Offset = VkOffset2D( information.OffsetX, information.OffsetY );

        CreateTextures();

        if (information.Swapchain_ == nullptr) {
            GPC_SUCCESS(CreateRenderTexture());
        } else {
            for (int i = 0; i < MAX_GENERATED_FRAME; i++) {
                *m_Images[i] = information.Swapchain_->GetTexture(i);
            }
        }

        m_RenderPass3D.Resize(m_Extent);
        m_RenderPass2D.Resize(m_Extent);
        m_RenderPassHDR.Resize(m_Extent);
        m_RenderPassPostEffect.Resize(m_Extent);
    }


    void RenderTarget::DrawHDRObjects()
    {

        m_RenderPassHDR.Begin();
        m_EntityDrawIndex = 0;

        const VkCommandBuffer commandBuffer = RenderCommands::GetCommandBuffer();

        UseProgram(&m_HDRProgram);
        m_HDRProgram.Update(m_HDRProgram.GetBindingSlot("camera"), &m_3DCameraBuffer, sizeof(m_3DCameraBuffer), 0);
        for (int i = 0; i < m_HDRObjectsCount; i++) {
            auto hdrObject = m_HDRObjects[i];

            m_3DConstantBuffer.ObjectID = m_EntityDrawIndex;

            vkCmdPushConstants(
                    commandBuffer,
                    m_HDRProgram.GetLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof(Object3DConstants),
                    &m_3DConstantBuffer
                );

            m_HDRProgram.Update(m_HDRProgram.GetBindingSlot("transforms"), &hdrObject.Transform,
                                   sizeof(glm::mat4x4), m_EntityDrawIndex);
            m_HDRProgram.Update(m_HDRProgram.GetBindingSlot("materials"), hdrObject.Material_,
                       sizeof(Material), m_EntityDrawIndex);

            const VkBuffer vertexBuffers[] = {
                hdrObject.VertexBuffer
            };

            constexpr VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, hdrObject.IndexBuffer, 0, VK_INDEX_TYPE_UINT16);

            vkCmdDrawIndexed(commandBuffer, hdrObject.Index, 1, 0, 0, 0);

            m_EntityDrawIndex++;

        }

        m_RenderPassHDR.End();
        m_UsedPrograms.clear();
        m_HDRObjectsCount = 0;
    }


    void RenderTarget::ApplyPostEffects()
    {

        if (m_RenderInformation.UseBloom) { m_PostEffectProgram.push_back(&m_BloomProgram); }

        if (m_PostEffectProgram.empty()) return;

        m_RenderPassPostEffect.Begin();

        for (auto const* postEffect : m_PostEffectProgram) {
            postEffect->InitializeWith(postEffect->GetBindingSlot("hdr"), { m_HDRTexture }, m_SpriteSampler); // TODO Faire une automatisation

            UseProgram(postEffect);

            vkCmdDraw(RenderCommands::GetCommandBuffer(), 3, 1, 0, 0);
        }

        m_RenderPassPostEffect.End();
        m_PostEffectProgram.clear();

    }


    void RenderTarget::UpdateProgramValues()
    {
        for (auto const* program : m_UsedPrograms) {
            program->Update(program->GetBindingSlot("camera"), &m_3DCameraBuffer, sizeof(m_3DCameraBuffer), 0);

            //LIGHTS
            program->Update(program->GetBindingSlot("lights"), &m_LightBuffer, sizeof(m_LightBuffer), 0);

            //SHADOWS
            program->Update(program->GetBindingSlot("LightTransform"), &m_DirSpotTransformBuffer, sizeof(m_DirSpotTransformBuffer), 0);
        }
        m_UsedPrograms.clear();
    }


    ErrorType RenderTarget::CreateRenderPass()
    {

        Attachment colorAttachment(AttachmentType::COLOR);
        colorAttachment
            .Format(m_RenderInformation.Format)
            .BindTexture(&m_MsaaTexture)
            .PassLayout(ImageLayoutType::UNDEFINED, ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL)
            .UseLayout(ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL)
            .SampleCount(RenderDevice::GetSampleCountFlag())
            .UseClearValue( { .color = { CLEAR_COLOR.r, CLEAR_COLOR.g, CLEAR_COLOR.b, CLEAR_COLOR.a } } )
            .KeepPreviousColor(false);

        Attachment depthAttachment(AttachmentType::DEPTH_STENCIL);
        depthAttachment
            .Format(RenderDevice::GetDepthStencilFormat())
            .BindTexture(&m_DepthStencilTexture)
            .PassLayout(ImageLayoutType::UNDEFINED, ImageLayoutType::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            .UseLayout(ImageLayoutType::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            .SampleCount(RenderDevice::GetSampleCountFlag())
            .UseClearValue( { .depthStencil = {1.0f, 0} } )
            .KeepDepthBuffer(false);

        m_RenderPass3D.BindAttachment(colorAttachment);
        m_RenderPass3D.BindAttachment(depthAttachment);

        const bool useResolver = m_RenderInformation.UseMSAA;

        if (useResolver) {

            Attachment colorResolver(AttachmentType::RESOLVER);
            colorResolver.Format(m_RenderInformation.Format)
                .BindTextures(m_Images)
                .PassLayout(ImageLayoutType::UNDEFINED, ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL)
                .UseLayout(ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL)
                .SampleCount(VK_SAMPLE_COUNT_1_BIT)
                .UseClearValue( { .color = { CLEAR_COLOR.r, CLEAR_COLOR.g, CLEAR_COLOR.b, CLEAR_COLOR.a } } )
                .JustStoreNewColor();

            m_RenderPass3D.BindAttachment(colorResolver);

        }

        RenderPassInformation information{};
        information.Name    = "3D_RENDER_PASS";
        information.Extent  = m_Extent;
        information.Offset  = m_Offset;

        m_RenderPass3D.Create(information);

        return ErrorType::SUCCESS;
    }


    ErrorType RenderTarget::CreateRenderTexture()
    {

        TextureCreationInformation information;
        information.Name = "RENDER_TEXTURE";
        information.ImageInformation.Format     = m_RenderInformation.Format;
        information.ImageInformation.Usage      = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        information.ImageInformation.Width      = m_RenderInformation.Width + m_RenderInformation.OffsetX;
        information.ImageInformation.Height     = m_RenderInformation.Height + m_RenderInformation.OffsetY;
        information.ImageInformation.NumSample  = VK_SAMPLE_COUNT_1_BIT;
        information.ImageInformation.Tiling     = VK_IMAGE_TILING_OPTIMAL;
        information.ImageInformation.Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        information.ImageInformation.Layout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        information.ViewInformation.Format      = m_RenderInformation.Format;
        information.ViewInformation.Flags       = VK_IMAGE_ASPECT_COLOR_BIT;

        ErrorType error = ErrorType::SUCCESS;
        for (size_t i = 0; i < m_Images.size(); i++) {
            *m_Images[i] = new Texture();
            error = (*m_Images[i])->Create(information);
            if (GPC_FAILED(error))
                return error;
        }

        return ErrorType::SUCCESS;
    }


    ErrorType RenderTarget::CreateHDRTexture()
    {

        TextureCreationInformation information;
        information.Name = "HDR_TEXTURE";
        information.ImageInformation.Format     = VK_FORMAT_R16G16B16A16_SFLOAT;
        information.ImageInformation.Usage      = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        information.ImageInformation.Width      = m_Extent.width;
        information.ImageInformation.Height     = m_Extent.height;
        information.ImageInformation.NumSample  = VK_SAMPLE_COUNT_1_BIT;
        information.ImageInformation.Tiling     = VK_IMAGE_TILING_OPTIMAL;
        information.ImageInformation.Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        information.ViewInformation.Format      = VK_FORMAT_R16G16B16A16_SFLOAT;
        information.ViewInformation.Flags       = VK_IMAGE_ASPECT_COLOR_BIT;
        m_HDRTexture = new Texture();
        return m_HDRTexture->Create(information);

    }



    ErrorType RenderTarget::CreateMsaaResources()
    {

        TextureCreationInformation information;
        information.Name    = "MSAA_TEXTURE";
        information.ImageInformation.Format     = m_RenderInformation.Format;
        information.ImageInformation.Usage      = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        information.ImageInformation.Width      = m_RenderInformation.Width;
        information.ImageInformation.Height     = m_RenderInformation.Height;
        information.ImageInformation.NumSample  = m_MsaaSampleCount;
        information.ImageInformation.Tiling     = VK_IMAGE_TILING_OPTIMAL;
        information.ImageInformation.Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        information.ViewInformation.Format      = m_RenderInformation.Format;
        information.ViewInformation.Flags       = VK_IMAGE_ASPECT_COLOR_BIT;

        m_MsaaTexture = new Texture();
        return m_MsaaTexture->Create(information);

    }


    ErrorType RenderTarget::CreateHDRResources()
    {

        Attachment colorAttachment(AttachmentType::COLOR);
        colorAttachment
            .Format(VK_FORMAT_R16G16B16A16_SFLOAT)
            .BindTextures({&m_HDRTexture})
            .PassLayout(ImageLayoutType::UNDEFINED, ImageLayoutType::SHADER_READ_ONLY_OPTIMAL)
            .UseLayout(ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL)
            .UseClearValue( { .color = {0.0f, 0.0f, 0.0f, 0.0f}})
            .SampleCount(VK_SAMPLE_COUNT_1_BIT)
            .KeepPreviousColor(false);

        m_RenderPassHDR.BindAttachment(colorAttachment);

        RenderPassInformation renderPassInfo{};
        renderPassInfo.Name    = "RENDER_PASS_HDR";
        renderPassInfo.Extent  = m_Extent;
        renderPassInfo.Offset  = m_Offset;

        m_RenderPassHDR.Create(renderPassInfo);

        m_HDRShaders[0].Create("hdr.frag", GPC::Shader::FRAGMENT);
        m_HDRShaders[1].Create("hdr.vert", GPC::Shader::VERTEX);

        GPC::ProgramInformation pInformation;
        pInformation.RenderPass_ = &m_RenderPassHDR;
        pInformation.Parameters.CullMode_ = GPC::CullMode::FRONT;
        pInformation.Parameters.PolygoneMode = GPC::PolygonMode::MODE_FILL;
        pInformation.Shaders = {
            &m_HDRShaders[0],
            &m_HDRShaders[1]
        };

        m_HDRProgram.BindLayout(DescriptorSetInformation::ForCamera(0));
        m_HDRProgram.BindLayout(DescriptorSetInformation::ForTransform(1));
        m_HDRProgram.BindLayout(DescriptorSetInformation::ForMaterials(2));

        m_HDRProgram.BindVertexInput( {
            .BindingDescription = Vertex::GetBindingDescription(),
            .AttributeDescriptions = Vertex::GetAttributeDescriptions(),
        } );

        m_HDRProgram.Create(pInformation);

        m_HDRProgram.InitializeWith({}, {});

        return ErrorType::SUCCESS;

    }


    ErrorType RenderTarget::CreatePostEffectResources()
    {

        Attachment colorAttachment(AttachmentType::COLOR);
        colorAttachment
            .Format(m_RenderInformation.Format)
            .BindTextures(m_Images)
            .PassLayout(ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL, ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL)
            .UseLayout(ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL)
            .UseClearValue( { .color = {0.0f, 0.0f, 0.0f, 1.0f}})
            .SampleCount(VK_SAMPLE_COUNT_1_BIT)
            .KeepPreviousColor(true);

        m_RenderPassPostEffect.BindAttachment(colorAttachment);

        RenderPassInformation renderPassInfo{};
        renderPassInfo.Name    = "RENDER_PASS_POST_EFFECT";
        renderPassInfo.Extent  = m_Extent;
        renderPassInfo.Offset  = m_Offset;

        m_RenderPassPostEffect.Create(renderPassInfo);

        if (m_RenderInformation.UseBloom) {
            m_BloomShaders[0].Create("bloom.frag", GPC::Shader::FRAGMENT);
            m_BloomShaders[1].Create("bloom.vert", GPC::Shader::VERTEX);

            GPC::ProgramInformation pInformation;
            pInformation.RenderPass_ = &m_RenderPassPostEffect;
            pInformation.Parameters.CullMode_ = GPC::CullMode::FRONT;
            pInformation.Parameters.PolygoneMode = GPC::PolygonMode::MODE_FILL;
            pInformation.Shaders = {
                &m_BloomShaders[0],
                &m_BloomShaders[1]
            };

            m_BloomProgram.BindLayout(DescriptorSetInformation::ForImages(0, "hdr", 1)); // TODO Si le post effetc marche pas ca viens de ca
            m_BloomProgram.Create(pInformation);

        }

        return ErrorType::SUCCESS;

    }


    ErrorType RenderTarget::CreateDepthResources()
    {

        TextureCreationInformation information;
        information.Name = "DEPTH_TEXTURE";
        information.ImageInformation.Format     = m_DepthStencilFormat;
        information.ImageInformation.Usage      = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        information.ImageInformation.Width      = m_RenderInformation.Width;
        information.ImageInformation.Height     = m_RenderInformation.Height;
        information.ImageInformation.NumSample  = m_MsaaSampleCount;
        information.ImageInformation.Tiling     = VK_IMAGE_TILING_OPTIMAL;
        information.ImageInformation.Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        information.ViewInformation.Format      = m_DepthStencilFormat;
        information.ViewInformation.Flags       = VK_IMAGE_ASPECT_DEPTH_BIT;

        m_DepthStencilTexture = new Texture();
        ErrorType error =  m_DepthStencilTexture->Create(information);
        m_DepthStencilTexture->TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0);

        return error;

    }


    ErrorType RenderTarget::CreateBatchingResources()
    {

        BufferInformation information {
            .Usages = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .Flags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            .TotalSize = MAX_ENTITIES * sizeof(Batched3DObject)
        };
        ErrorType error = m_Batched3DObjectBuffer.Create("BATCHING_BUFFER", information);
        GPC_SUCCESS(error);
        error = m_Batched3DObjectBuffer.Map<Batched3DObject>(&mp_MappedBatched3DObject, 0, MAX_ENTITIES);
        GPC_SUCCESS(error);

        return error;
    }


    void RenderTarget::Create2DFeatures()
    {

        Attachment colorAttachment(AttachmentType::COLOR);
        colorAttachment
            .Format(m_RenderInformation.Format)
            .BindTextures(m_Images)
            .PassLayout(ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL, ImageLayoutType::PRESENT_SRC)
            .UseLayout(ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL)
            .UseClearValue( { .color = {0.0f, 0.0f, 0.0f, 1.0f}})
            .SampleCount(VK_SAMPLE_COUNT_1_BIT)
            .KeepPreviousColor(true);

        m_RenderPass2D.BindAttachment(colorAttachment);

        RenderPassInformation renderPassInfo{};
        renderPassInfo.Name    = "RENDER_PASS_2D";
        renderPassInfo.Extent  = m_Extent;
        renderPassInfo.Offset  = m_Offset;

        m_RenderPass2D.Create(renderPassInfo);

        m_2DBaseShader[0].Create("sprite.frag", GPC::Shader::FRAGMENT);
        m_2DBaseShader[1].Create("sprite.vert", GPC::Shader::VERTEX);
        m_2DBaseShader[2].Create("text.frag", GPC::Shader::FRAGMENT);
        m_2DBaseShader[3].Create("text.vert", GPC::Shader::VERTEX);

        GPC::ProgramInformation pInformation;
        pInformation.RenderPass_ = &m_RenderPass2D;
        pInformation.Parameters.CullMode_ = GPC::CullMode::FRONT;
        pInformation.Parameters.PolygoneMode = GPC::PolygonMode::MODE_FILL;
        pInformation.Shaders = {
            &m_2DBaseShader[0],
            &m_2DBaseShader[1]
        };

        m_2DSpriteBaseProgram.BindLayout( DescriptorSetInformation::ForCamera( 0 ) );
        m_2DSpriteBaseProgram.BindLayout( DescriptorSetInformation::ForImages(1, "images", 50));
        m_2DSpriteBaseProgram.BindVertexInput( {
            .BindingDescription = Vertex::GetBindingDescription(),
            .AttributeDescriptions = Vertex::GetAttributeDescriptions(),
        } );
        m_2DSpriteBaseProgram.BindVertexInput( {
            .BindingDescription = Instance2D::GetBindingDescription(),
            .AttributeDescriptions = Instance2D::GetAttributeDescriptions(),
        } );
        m_2DSpriteBaseProgram.Create(pInformation);


        GPC::ProgramInformation pTextInformation;
        pTextInformation.RenderPass_ = &m_RenderPass2D;
        pTextInformation.Parameters.CullMode_ = GPC::CullMode::FRONT;
        pTextInformation.Parameters.PolygoneMode = GPC::PolygonMode::MODE_FILL;
        pTextInformation.Shaders = {
            &m_2DBaseShader[2],
            &m_2DBaseShader[3]
        };

        m_2DTextBaseProgram.BindLayout( DescriptorSetInformation::ForCamera( 0 ) );
        m_2DTextBaseProgram.BindLayout( DescriptorSetInformation::ForImages(1, "images", 50));
        m_2DTextBaseProgram.BindVertexInput( {
            .BindingDescription = Vertex::GetBindingDescription(),
            .AttributeDescriptions = Vertex::GetAttributeDescriptions(),
        } );
        m_2DTextBaseProgram.BindVertexInput( {
            .BindingDescription = Instance2D::GetBindingDescription(),
            .AttributeDescriptions = Instance2D::GetAttributeDescriptions(),
        } );
        m_2DTextBaseProgram.Create(pTextInformation);

        GeometryData* geo = new GeometryData();
        Primitive::CreateRectangle({1, 1}, geo);
        m_2DGeometry.Create(geo);

        SamplerInformation samplerInfo = {};
        m_TextSampler.Create(samplerInfo);

        samplerInfo.MagFilter = VK_FILTER_NEAREST;
        m_SpriteSampler.Create(samplerInfo);

        m_TextObject.Material_ = &m_TextMaterial;
        m_TextObject.Size = {64, 64};
        m_TextObject.ZIndex = 10;

        BufferInformation information {
            .Usages = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .Flags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            .TotalSize = MAX_ENTITIES * sizeof(Instance2D)
        };
        for (uint8_t i = 0; i < MAX_GENERATED_FRAME; ++i)
        {
            m_2DSpriteBuffer[i].Create("INSTANCE_SPRITE_BUFFER_" + std::to_string(i), information);
            m_2DSpriteBuffer[i].Map<Instance2D>(&mp_2DMappedSprite[i], 0, MAX_ENTITIES);

            m_2DTextBuffer[i].Create("INSTANCE_TEXT_BUFFER_" + std::to_string(i), information);
            m_2DTextBuffer[i].Map<Instance2D>(&mp_2DMappedText[i], 0, MAX_ENTITIES);
        }

    }


    ErrorType RenderTarget::CreateShadowRenderPass()
    {
        // DIR
        {
            TextureCreationInformation info;

            info.Name = "SHADOW_TEXTURE";
            //info.ImageInformation.Format = VK_FORMAT_D16_UNORM;
            info.ImageInformation.Format = VK_FORMAT_D32_SFLOAT;
            info.ImageInformation.Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            info.ImageInformation.Width  = m_shadowMapDirResolution.width;
            info.ImageInformation.Height = m_shadowMapDirResolution.height;
            info.ImageInformation.NumSample = VK_SAMPLE_COUNT_1_BIT;
            info.ImageInformation.Tiling = VK_IMAGE_TILING_OPTIMAL;
            info.ImageInformation.Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            info.ImageInformation.LayerCount = 2;

            info.ViewInformation.Format = info.ImageInformation.Format;
            info.ViewInformation.ViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            info.ViewInformation.Flags = VK_IMAGE_ASPECT_DEPTH_BIT;
            info.ViewInformation.LayerCount = 2;

            m_DepthDirShadowTexture = new Texture();
            m_DepthDirShadowTexture->Create(info);


            Attachment depthAttachment(AttachmentType::DEPTH_STENCIL);
            depthAttachment
                .Format(info.ImageInformation.Format)
                .BindTexture(&m_DepthDirShadowTexture)
                .PassLayout(ImageLayoutType::UNDEFINED, ImageLayoutType::SHADER_READ_ONLY_OPTIMAL)
                .UseLayout(ImageLayoutType::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                .SampleCount(VK_SAMPLE_COUNT_1_BIT)
                .UseClearValue( { .depthStencil = {1.0f, 0} } )
                .KeepDepthBuffer(false);


            m_DirShadowRenderPass.BindAttachment(depthAttachment);

            RenderPassInformation information{};
            information.Name    = "DIR_LIGHT_SHADOW_RENDER_PASS";
            information.Extent  = m_shadowMapDirResolution;
            information.Offset  = m_Offset;
            information.NumberOfFrameBufferLayer = 2;

            m_DirShadowRenderPass.Create(information);
            m_DirShadowRenderPass.Resize(m_shadowMapDirResolution);

            m_DirShadowBaseShader[0].Create("shadow.frag", GPC::Shader::FRAGMENT);
            m_DirShadowBaseShader[1].Create("shadow.vert", GPC::Shader::VERTEX);
            m_DirShadowBaseShader[2].Create("shadow_dir.geom", GPC::Shader::GEOMETRY);

            GPC::ProgramInformation pInformation;
            pInformation.RenderPass_ = &m_DirShadowRenderPass;
            pInformation.Parameters.CullMode_ = GPC::CullMode::FRONT;
            pInformation.Parameters.PolygoneMode = GPC::PolygonMode::MODE_FILL;
            pInformation.Parameters.DepthBiasEnable = true;
            pInformation.Parameters.DepthBiasConstantFactor = 1.5f;
            pInformation.Parameters.DepthBiasSlopeFactor = 2.0f;
            pInformation.Shaders = {
                &m_DirShadowBaseShader[0],
                &m_DirShadowBaseShader[1],
                &m_DirShadowBaseShader[2]
            };

            m_DirShadowProgram.BindLayout({
                .Name = "transforms",
                .Type = DescriptorType::STORAGE_BUFFER,
                .Usages = STORAGE_BUFFER_BIT,
                .BindingSlot = 0,
                .DescriptorCount = 1,
                .ElementCount = 1,
                .ElementStride = sizeof(TransformData),
                .Flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
                .Sampler_ = nullptr
            });

            m_DirShadowProgram.BindLayout( {
                .Name = "LightTransform",
                .Type = DescriptorType::UNIFORM_BUFFER,
                .Usages = UNIFORM_BUFFER_BIT,
                .BindingSlot = 1,
                .DescriptorCount = 1,
                .ElementCount = 1,
                .ElementStride = sizeof(TransformLight),
                .Flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
                .Sampler_ = nullptr,
            });

            m_DirShadowProgram.BindLayout( DescriptorSetInformation::ForImages(2, "textures", 50) );

            m_DirShadowProgram.BindVertexInput( {
                .BindingDescription = Vertex::GetBindingDescription(),
                .AttributeDescriptions = Vertex::GetAttributeDescriptions(),
            } );

            pInformation.LayoutInformation.Offset       = 0;
            pInformation.LayoutInformation.Size         = sizeof(ShadowPushConstant);
            pInformation.LayoutInformation.StageFlags   = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;

            m_DirShadowProgram.Create(pInformation);

            Sampler sampler {};
            sampler.Create({});
            m_DirShadowProgram.InitializeWith({m_DepthDirShadowTexture}, {sampler});

            ShadowsUpdateInformation updateInformation =
            {
                .Program = &m_DirShadowProgram,
                .RenderPass_ = &m_DirShadowRenderPass
            };

            m_shadowUpdateInformations[0] = updateInformation;

        }

        //SPOT
        {
            TextureCreationInformation info;

            info.Name = "SHADOW_TEXTURE";
            info.ImageInformation.Format = VK_FORMAT_D16_UNORM;
            //info.ImageInformation.Format = VK_FORMAT_D32_SFLOAT;
            info.ImageInformation.Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            info.ImageInformation.Width = m_shadowMapResolution.width;
            info.ImageInformation.Height = m_shadowMapResolution.height;
            info.ImageInformation.NumSample = VK_SAMPLE_COUNT_1_BIT;
            info.ImageInformation.Tiling = VK_IMAGE_TILING_OPTIMAL;
            info.ImageInformation.Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            info.ImageInformation.LayerCount = MAX_SHADOWS_LIGHT;

            info.ViewInformation.Format = info.ImageInformation.Format;
            info.ViewInformation.ViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            info.ViewInformation.Flags = VK_IMAGE_ASPECT_DEPTH_BIT;
            info.ViewInformation.LayerCount = MAX_SHADOWS_LIGHT;

            m_DepthSpotShadowTexture = new Texture();
            m_DepthSpotShadowTexture->Create(info);


            Attachment depthAttachment(AttachmentType::DEPTH_STENCIL);
            depthAttachment
                .Format(info.ImageInformation.Format)
                .BindTexture(&m_DepthSpotShadowTexture)
                .PassLayout(ImageLayoutType::UNDEFINED, ImageLayoutType::SHADER_READ_ONLY_OPTIMAL)
                .UseLayout(ImageLayoutType::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                .SampleCount(VK_SAMPLE_COUNT_1_BIT)
                .UseClearValue( { .depthStencil = {1.0f, 0} } )
                .KeepDepthBuffer(false);

            m_SpotShadowRenderPass.BindAttachment(depthAttachment);

            RenderPassInformation information{};
            information.Name    = "SPOT_SHADOW_RENDER_PASS";
            information.Extent  = m_shadowMapResolution;
            information.Offset  = m_Offset;
            information.NumberOfFrameBufferLayer = MAX_SHADOWS_LIGHT;

            m_SpotShadowRenderPass.Create(information);
            m_SpotShadowRenderPass.Resize(m_shadowMapResolution);

            m_SpotShadowBaseShader[0].Create("shadow.frag", GPC::Shader::FRAGMENT);
            m_SpotShadowBaseShader[1].Create("shadow.vert", GPC::Shader::VERTEX);
            m_SpotShadowBaseShader[2].Create("shadow.geom", GPC::Shader::GEOMETRY);

            GPC::ProgramInformation pInformation;
            pInformation.RenderPass_ = &m_SpotShadowRenderPass;
            pInformation.Parameters.CullMode_ = GPC::CullMode::FRONT;
            pInformation.Parameters.PolygoneMode = GPC::PolygonMode::MODE_FILL;
            pInformation.Parameters.DepthBiasEnable = true;
            pInformation.Parameters.DepthBiasConstantFactor = 1.5f;
            pInformation.Parameters.DepthBiasSlopeFactor = 2.0f;
            pInformation.Shaders = {
                &m_SpotShadowBaseShader[0],
                &m_SpotShadowBaseShader[1],
                &m_SpotShadowBaseShader[2]
            };

            m_SpotShadowProgram.BindLayout({
                .Name = "transforms",
                .Type = DescriptorType::STORAGE_BUFFER,
                .Usages = STORAGE_BUFFER_BIT,
                .BindingSlot = 0,
                .DescriptorCount = 1,
                .ElementCount = 1,
                .ElementStride = sizeof(TransformData),
                .Flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
                .Sampler_ = nullptr
            });

            m_SpotShadowProgram.BindLayout( {
                .Name = "LightTransform",
                .Type = DescriptorType::UNIFORM_BUFFER,
                .Usages = UNIFORM_BUFFER_BIT,
                .BindingSlot = 1,
                .DescriptorCount = 1,
                .ElementCount = 1,
                .ElementStride = sizeof(TransformLight),
                .Flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
                .Sampler_ = nullptr,
            });

            m_SpotShadowProgram.BindLayout( DescriptorSetInformation::ForImages(2, "textures", 50) );

            m_SpotShadowProgram.BindVertexInput( {
                .BindingDescription = Vertex::GetBindingDescription(),
                .AttributeDescriptions = Vertex::GetAttributeDescriptions(),
            } );

            pInformation.LayoutInformation.Offset       = 0;
            pInformation.LayoutInformation.Size         = sizeof(ShadowPushConstant);
            pInformation.LayoutInformation.StageFlags   = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;

            m_SpotShadowProgram.Create(pInformation);

            Sampler sampler {};
            sampler.Create({});
            m_SpotShadowProgram.InitializeWith({m_DepthDirShadowTexture}, {sampler});

            ShadowsUpdateInformation updateInformation =
            {
                .Program = &m_SpotShadowProgram,
                .RenderPass_ = &m_SpotShadowRenderPass
            };

            m_shadowUpdateInformations[1] = updateInformation;

        }

        // POINT
        {
            TextureCreationInformation cubeInfo;
            cubeInfo.Name = "SHADOW_CUBE_TEXTURE";
            cubeInfo.ImageInformation.Format      = VK_FORMAT_D16_UNORM;
            cubeInfo.ImageInformation.Usage       = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            cubeInfo.ImageInformation.Width       = m_shadowMapResolution.width;
            cubeInfo.ImageInformation.Height      = m_shadowMapResolution.height;
            cubeInfo.ImageInformation.NumSample   = VK_SAMPLE_COUNT_1_BIT;
            cubeInfo.ImageInformation.Tiling      = VK_IMAGE_TILING_OPTIMAL;
            cubeInfo.ImageInformation.Properties  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            cubeInfo.ImageInformation.LayerCount  = MAX_POINT_LIGHT * 6;
            cubeInfo.ImageInformation.Flags       = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

            cubeInfo.ViewInformation.Format       = VK_FORMAT_D16_UNORM;
            cubeInfo.ViewInformation.ViewType     = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            cubeInfo.ViewInformation.Flags        = VK_IMAGE_ASPECT_DEPTH_BIT;
            cubeInfo.ViewInformation.LayerCount   = MAX_POINT_LIGHT * 6;

            m_DepthShadowCubeTexture = new Texture();
            m_DepthShadowCubeTexture->Create(cubeInfo);

            Attachment depthAttachment(AttachmentType::DEPTH_STENCIL);
            depthAttachment
                .Format(cubeInfo.ImageInformation.Format)
                .BindTexture(&m_DepthShadowCubeTexture)
                .PassLayout(ImageLayoutType::UNDEFINED, ImageLayoutType::SHADER_READ_ONLY_OPTIMAL)
                .UseLayout(ImageLayoutType::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                .SampleCount(VK_SAMPLE_COUNT_1_BIT)
                .UseClearValue( { .depthStencil = {1.0f, 0} } )
                .KeepDepthBuffer(false);


            m_PointShadowRenderPass.BindAttachment(depthAttachment);

            RenderPassInformation information{};
            information.Name    = "POINT_LIGHT_SHADOW_RENDER_PASS";
            information.Extent  = m_shadowMapResolution;
            information.Offset  = m_Offset;
            information.NumberOfFrameBufferLayer = MAX_POINT_LIGHT * 6;

            m_PointShadowRenderPass.Create(information);
            m_PointShadowRenderPass.Resize(m_shadowMapResolution);

            m_PointShadowShader[0].Create("shadow_point.frag", GPC::Shader::FRAGMENT);
            m_PointShadowShader[1].Create("shadow_point.vert", GPC::Shader::VERTEX);
            m_PointShadowShader[2].Create("shadow_point.geom", GPC::Shader::GEOMETRY);

            GPC::ProgramInformation pInformation;
            pInformation.RenderPass_ = &m_PointShadowRenderPass;
            pInformation.Parameters.CullMode_ = GPC::CullMode::BACK;
            pInformation.Parameters.PolygoneMode = GPC::PolygonMode::MODE_FILL;
            pInformation.Parameters.DepthBiasEnable = true;
            pInformation.Parameters.DepthBiasConstantFactor = 1.5f;
            pInformation.Parameters.DepthBiasSlopeFactor = 2.0f;
            pInformation.Shaders = {
                &m_PointShadowShader[0],
                &m_PointShadowShader[1],
                &m_PointShadowShader[2]
            };

            m_PointShadowProgram.BindLayout({
                .Name = "transforms",
                .Type = DescriptorType::STORAGE_BUFFER,
                .Usages = STORAGE_BUFFER_BIT,
                .BindingSlot = 0,
                .DescriptorCount = 1,
                .ElementCount = 1,
                .ElementStride = sizeof(TransformData),
                .Flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
                .Sampler_ = nullptr
            });

            m_PointShadowProgram.BindLayout( {
                .Name = "PointLightPositions",
                .Type = DescriptorType::UNIFORM_BUFFER,
                .Usages = UNIFORM_BUFFER_BIT,
                .BindingSlot = 1,
                .DescriptorCount = 1,
                .ElementCount = 1,
                .ElementStride = sizeof(TransformLight),
                .Flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
                .Sampler_ = nullptr,
            });

            m_PointShadowProgram.BindVertexInput( {
                .BindingDescription = Vertex::GetBindingDescription(),
                .AttributeDescriptions = Vertex::GetAttributeDescriptions(),
            } );

            pInformation.LayoutInformation.Offset       = 0;
            pInformation.LayoutInformation.Size         = sizeof(ShadowPointPushConstant);
            pInformation.LayoutInformation.StageFlags   = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

            pInformation.DepthTestEnable  = true;
            pInformation.DepthWriteEnable = true;

            m_PointShadowProgram.Create(pInformation);

            m_PointShadowProgram.InitializeWith({}, {});

            ShadowsUpdateInformation updateInformation =
            {
                .Program = &m_PointShadowProgram,
                .RenderPass_ = &m_PointShadowRenderPass
            };

            m_shadowUpdateInformations[2] = updateInformation;
        }

        return ErrorType::SUCCESS;
    }

}