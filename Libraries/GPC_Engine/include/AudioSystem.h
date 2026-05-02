#pragma once

#include "AudioMixer.h"
#include "AudioResource.h"
#include "AudioType.h"
#include "ECS_Defines.h"
#include "System.h"
#include "IAudioVoice.h"
#include "Audio.h"
#include <vector>
#include <string>

#include "TransformComponents.h"

namespace GPC
{
    struct AudioResourceAsset;
    class Scene;

    struct AudioSource : public Component
    {
        BIND_COMPONENT(AUDIOSOURCE);
        AUTO_COMPONENT_CONSTRUCTOR(AudioSource);

        std::string     Path{};
        AudioResourceAsset*  Resource = nullptr;
        bool            Looping = false;
        AudioType       TypeVoice = AudioType::SFX;
        float           Volume = 1.0f;

        bool            Is3D = false;
        float           MinDistance = 1.0f;
        float           MaxDistance = 50.0f;

        void            Play() { m_PlayRequested = true; m_StopRequested = false; }
        void            Stop() { m_StopRequested = true; m_PlayRequested = false; }
        bool            IsPlaying() const { return m_IsPlaying; }
    private:
        friend class AudioSystem;

        bool            m_PlayRequested = false;
        bool            m_StopRequested = false;
        bool            m_IsPlaying = false;

        IAudioVoice*    Voice = nullptr;
    };

    class AudioSystem : public System
    {
    public:
        BIND_SYSTEM(AUDIO);

        AudioSystem();
        ~AudioSystem() override;

        void    OnUpdate() override;

    private:
        void                ProcessCommands();
        void                ProcessStop(AudioSource& src);
        void                ProcessPlay(AudioSource& src, EntityID entity);
        void                ProcessActive(AudioSource& src, EntityID entity);

        void                ApplySpatialization(AudioSource &src, EntityID entity);
        void                PushAudioData();
        Transform3D*        GetListenerTransform();

        // AudioResourceAsset* GetOrLoadResource(const std::string& path);
        AudioResourceAsset*      GetOrLoadResource(const std::string& path);

        AudioMixer          m_Mixer;
        Audio               m_AudioEngine;
        AudioConfig         m_Config;
        std::vector<float>  m_ChunkBuffer;

        // Todo : AssetEngine
        // std::unordered_map<std::string, AudioResource*> m_SFXCache;

        bool                m_HasPendingAudio{false};
    };
}
