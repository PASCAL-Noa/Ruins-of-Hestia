#include "SettingsGame.h"
#include "AudioMath.h"
#include "SerialFile.h"
#include "File.h"
#include "IAudioVoice.h"
#include "SerialTree.h"
#include "StaticVoice.h"
#include "StreamVoice.h"

namespace GPC
{

    SettingsGame& SettingsGame::Get()
    {
        static SettingsGame instance;
        return instance;
    }

    float SettingsGame::GetVolume(AudioType type) const
    {
        if (type == AudioType::Music) {
            return m_Audio.MasterVolume * m_Audio.MusicVolume;
        } else {
            return m_Audio.MasterVolume * m_Audio.SFXVolume;
        }
    }

    void SettingsGame::ApplyWindow(RenderWindow* window)
    {
        if (!window) return;

        // Logique d'application
    }

    void SettingsGame::ApplyAudio(const std::vector<IAudioVoice*>& audio, bool music)
    {
        float multiplier = m_Audio.MasterVolume * (music ? m_Audio.SFXVolume : m_Audio.MusicVolume);
        AudioMath::StereoVolume vol = { multiplier, multiplier };
        for (auto voice : audio){
            if (voice) voice->SetVolume(vol);
        }
    }

    void SettingsGame::BindKey(PlayerAction action, Inputs::KeyCode newKey)
    {
        switch(action)
        {
        case PlayerAction::MOVE_FORWARD:    m_Controls.MoveForward = newKey; break;
        case PlayerAction::MOVE_LEFT:       m_Controls.MoveLeftward = newKey; break;
        case PlayerAction::MOVE_BACKWARD:   m_Controls.MoveBackward = newKey; break;
        case PlayerAction::MOVE_RIGHT:      m_Controls.MoveRightward = newKey; break;
        case PlayerAction::INVENTORY:       m_Controls.OpenInventory = newKey; break;
        case PlayerAction::BUILD:           m_Controls.OpenBuildMode = newKey; break;
        }
    }

    void SettingsGame::Save()
    {
        GPC::SerialTree root;

        root["Audio"]["Master"].Set(&m_Audio.MasterVolume);
        root["Audio"]["Music"].Set(&m_Audio.MusicVolume);
        root["Audio"]["SFX"].Set(&m_Audio.SFXVolume);

        root["Video"]["FPS"].Set(&m_Video.FPS);
        root["Video"]["Fullscreen"].Set(&m_Video.Fullscreen);

        GPC::SerialFile::Save("settingsGame.bin", root);
    }

    void SettingsGame::Load()
    {
        GPC::SerialTree root;

        if (GPC::SerialFile::Load("settingsGame.bin", root) == ErrorType::SUCCESS) {

            if (root.HasKey("Audio")) {
                m_Audio.MasterVolume = *root["Audio"]["Master"].Get<float>();
                m_Audio.MusicVolume  = *root["Audio"]["Music"].Get<float>();
                m_Audio.SFXVolume    = *root["Audio"]["SFX"].Get<float>();
            }

            if (root.HasKey("Video")) {
                m_Video.FPS        = *root["Video"]["FPS"].Get<int>();
                m_Video.Fullscreen = *root["Video"]["Fullscreen"].Get<bool>();
            }
        }
    }
}
