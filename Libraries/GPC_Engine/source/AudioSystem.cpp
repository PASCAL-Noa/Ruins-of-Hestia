#include "AudioSystem.h"

#include "Assets.h"
#include "Scene.h"
#include "StaticVoice.h"
#include "StreamVoice.h"
#include "AudioListener.h"
#include "Transform.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

namespace GPC
{
    AudioSystem::AudioSystem()
    {
        if (m_AudioEngine.Create(m_Config))
        {
            m_AudioEngine.Start();
            m_ChunkBuffer.resize(m_Config.BufferFrames * m_Config.Channels, 0.0f);
        }
    }

    AudioSystem::~AudioSystem()
    {
        m_AudioEngine.Stop();

        if (m_Ctx.pScene)
        {
            CBlockPtr<AudioSource> sourceBlock = m_Ctx.pScene->GetComponentBlock<AudioSource>();
            if (sourceBlock)
            {
                AudioSource* sources = sourceBlock->ComponentData();
                for (uint64_t i = 0; i < sourceBlock->GetEntityCount(); ++i)
                {
                    if (sources[i].Voice)
                    {
                        delete sources[i].Voice;
                        sources[i].Voice = nullptr;
                    }
                }
            }
        }

        // for (auto& pair : m_SFXCache)
        // {
        //     delete pair.second;
        // }
        // m_SFXCache.clear();
    }

    void AudioSystem::OnUpdate()
    {
        System::OnUpdate();
        if (m_ChunkBuffer.empty() || !m_Ctx.pScene)
            return;

        ProcessCommands();
        PushAudioData();
    }

    void AudioSystem::ProcessCommands()
    {
        CBlockPtr<AudioSource> sourceBlock = m_Ctx.pScene->GetComponentBlock<AudioSource>();
        if (!sourceBlock) return;

        AudioSource* sources = sourceBlock->ComponentData();
        EntityID* entityIds = sourceBlock->EntitiesData();
        for (uint64_t i = 0; i < sourceBlock->GetEntityCount(); ++i)
        {
            EntityID entity = entityIds[i];

            ProcessStop(sources[i]);
            ProcessPlay(sources[i], entity);
            ProcessActive(sources[i], entity);
        }
    }

    void AudioSystem::ProcessStop(AudioSource& src)
    {
        if (!src.m_StopRequested) return;

        if (src.Voice)
        {
            m_Mixer.RemoveVoice(src.Voice);
            delete src.Voice;
            src.Voice = nullptr;
        }
        src.m_IsPlaying = false;
        src.m_StopRequested = false;
    }

    void AudioSystem::ProcessPlay(AudioSource& src, EntityID entity)
    {
        if (!src.m_PlayRequested)
            return;

        if (src.Voice)
        {
            m_Mixer.RemoveVoice(src.Voice);
            delete src.Voice;
            src.Voice = nullptr;
        }

        if (src.TypeVoice == AudioType::SFX)
        {
            if (!src.Resource)
                src.Resource = GetOrLoadResource(src.Path);

            if (src.Resource)
                src.Voice = new StaticVoice(src.Resource->Samples, src.Resource->Channels);
        }
        else if (src.TypeVoice == AudioType::Music && !src.Path.empty())
        {
            src.Voice = new StreamVoice(src.Path);
        }

        if (!src.Voice)
        {
            src.m_PlayRequested = false;
            return;
        }

        ApplySpatialization(src, entity);
        src.Voice->SetLooping(src.Looping);
        m_Mixer.AddVoice(src.Voice);

        src.m_IsPlaying = true;
        src.m_PlayRequested = false;
    }

    void AudioSystem::ProcessActive(AudioSource& src, EntityID entity)
    {
        if (!src.Voice || !src.m_IsPlaying) return;

        ApplySpatialization(src, entity);
        src.Voice->SetLooping(src.Looping);

        if (src.Voice->IsFinished())
        {
            m_Mixer.RemoveVoice(src.Voice);
            delete src.Voice;
            src.Voice = nullptr;
            src.m_IsPlaying = false;
        }
    }

    void AudioSystem::PushAudioData()
    {
        std::span<float> span(m_ChunkBuffer);

        while (true)
        {
            if (!m_HasPendingAudio)
            {
                m_Mixer.Mix(span);
                m_HasPendingAudio = true;
            }

            if (m_AudioEngine.PushSamples(span))
                m_HasPendingAudio = false;
            else
                break;
        }
    }

    Transform3D* AudioSystem::GetListenerTransform()
    {
        CBlockPtr<AudioListener> listenerBlock = m_Ctx.pScene->GetComponentBlock<AudioListener>();
        if (!listenerBlock || listenerBlock->GetEntityCount() == 0)
            return nullptr;

        EntityID listenerEntity = listenerBlock->EntitiesData()[0];
        return m_Ctx.pScene->GetComponent<Transform3D>(listenerEntity);
    }

    void AudioSystem::ApplySpatialization(AudioSource& src, EntityID entity)
    {
        if (!src.Is3D)
        {
            src.Voice->SetVolume({src.Volume, src.Volume});
            return;
        }

        Transform3D* listenerComp = GetListenerTransform();
        Transform3D* sourceComp = m_Ctx.pScene->GetComponent<Transform3D>(entity);

        if (listenerComp && sourceComp)
        {
            glm::vec3 dirToSource = sourceComp->LocalTransform.GetPosition() - listenerComp->LocalTransform.GetPosition();
            float distance = glm::length(dirToSource);
            float attenuation = 1.0f;

            if (distance > src.MinDistance && src.MaxDistance > src.MinDistance)
            {
                attenuation = 1.0f - std::clamp((distance - src.MinDistance) / (src.MaxDistance - src.MinDistance), 0.0f, 1.0f);
            }

            float pan = 0.0f;
            if (distance > 0.001f)
            {
                glm::vec3 normDir = dirToSource / distance;
                glm::vec3 listenerRight = glm::normalize(listenerComp->LocalTransform.GetRight());
                pan = std::clamp(glm::dot(listenerRight, normDir), -1.0f, 1.0f);
            }

            float normalizedPan = (pan + 1.0f) * 0.5f;
            float leftVol = src.Volume * attenuation * std::sqrt(1.0f - normalizedPan);
            float rightVol = src.Volume * attenuation * std::sqrt(normalizedPan);

            src.Voice->SetVolume({leftVol, rightVol});
        }
        else
        {
            src.Voice->SetVolume({src.Volume, src.Volume});
        }
    }

    AudioResourceAsset* AudioSystem::GetOrLoadResource(const std::string &path)
    {
        if (path.empty())
            return nullptr;
        auto cacheIterator = Assets::GetInstance()->m_Assets.find(path);
        if (cacheIterator != Assets::GetInstance()->m_Assets.end())
            return cacheIterator->second->ToAudioResource();

        AudioResourceAsset *resource = new AudioResourceAsset();
        if (WavParser::LoadToMemory(path, *resource))
        {
            Assets::GetInstance()->m_Assets[path] = resource;
            return resource;
        }

        delete resource;
        return nullptr;
    }
}