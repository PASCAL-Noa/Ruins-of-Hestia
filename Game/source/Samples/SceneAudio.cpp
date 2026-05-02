#include "Samples/SceneAudio.h"

#include <algorithm>

#include "Assets.h"
#include "AudioListener.h"
#include "AudioSystem.h"
#include "CameraSystem.h"
#include "FreeCamera.h"
#include "Inputs.h"

namespace GPC
{
    ErrorType SceneAudio::OnAssetsLoad()
    {
        SceneDefault::OnAssetsLoad();
        GPC_ASSETS->AddAudio("ROH", "Resources/Audio/Music/Ecran_titre.wav");
        GPC_ASSETS->AddAudio("Zemmour", "Resources/Audio/SFX/zemmour-tousse.wav");
        return ErrorType::SUCCESS;
    }

    ErrorType SceneAudio::OnCreate(SceneInformation &info)
    {
        auto camera  = CreateEntityAs3D();
        auto cTransform     = GetComponent<Transform3D>(camera);
        auto cCamera        = AddComponent<CameraComponent>(camera);
        auto cController    = AddBehavior<FreeCamera>(camera);
        auto cAudioListener = AddComponent<AudioListener>(camera);

        m_MusicEntity = CreateEntityAs3D();
        auto audioSource = AddComponent<AudioSource>(m_MusicEntity);
        audioSource->Path = "Resources/Audio/Music/Ecran_titre.wav";
        audioSource->Resource = GPC_AUDIO("ROH");
        audioSource->TypeVoice = AudioType::Music;
        audioSource->Volume = 1.0f;
        audioSource->Looping = true;
        audioSource->Play();

        m_SfxEntity = CreateEntityAs3D();
        auto sfxSource = AddComponent<AudioSource>(m_SfxEntity);
        sfxSource->Path = "Resources/Audio/SFX/sfx.wav";
        sfxSource->Resource = GPC_AUDIO("Zemmour");
        sfxSource->TypeVoice = AudioType::SFX;
        sfxSource->Volume = 0.8f;
        sfxSource->Looping = false;

        return ErrorType::SUCCESS;
    }
    void SceneAudio::OnSceneUpdate()
    {
        SceneDefault::OnSceneUpdate();

        if (m_MusicEntity == 0) return;

        auto audioSource = GetComponent<AudioSource>(m_MusicEntity);
        if (!audioSource) return;

        if (Inputs::IsKeyPress(Inputs::KeyCode::J))
        {
            audioSource->Volume = std::min(1.0f, audioSource->Volume + 0.1f);
        }

        if (Inputs::IsKeyPress(Inputs::KeyCode::K))
        {
            audioSource->Volume = std::max(0.0f, audioSource->Volume - 0.1f);
        }

        if (Inputs::IsKeyPress(Inputs::KeyCode::SPACE))
        {
            if (m_SfxEntity != 0)
            {
                auto sfxSource = GetComponent<AudioSource>(m_SfxEntity);
                if (sfxSource)
                {
                    sfxSource->Play();
                }
            }
        }
    }
}
