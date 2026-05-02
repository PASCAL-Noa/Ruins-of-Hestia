#pragma once
#include "RenderWindow.h"
#include "Inputs.h"
#include "AudioType.h"

namespace GPC {
    struct VideoData
    {
        uint32_t FPS = 240;
        bool Fullscreen = false;
        uint32_t Width = 1280;
        uint32_t Height = 720;
    };

    struct AudioData
    {
        float MasterVolume = 0.1f;
        float MusicVolume = 0.1f;
        float SFXVolume = 1.0f;
    };

    struct Controls {
        Inputs::KeyCode MoveForward = Inputs::KeyCode::Z;
        Inputs::KeyCode MoveLeftward = Inputs::KeyCode::Q;
        Inputs::KeyCode MoveBackward = Inputs::KeyCode::S;
        Inputs::KeyCode MoveRightward = Inputs::KeyCode::D;
        Inputs::KeyCode OpenInventory = Inputs::KeyCode::E; //TODO: A voir avec les autres
        Inputs::KeyCode OpenBuildMode = Inputs::KeyCode::B; //TODO: A voir avec les autres
        float MouseSensitivity = 1.0f;
    };

    enum class PlayerAction {
        MOVE_FORWARD,
        MOVE_LEFT,
        MOVE_BACKWARD,
        MOVE_RIGHT,
        INVENTORY,
        BUILD
    };

    class IAudioVoice;
    class SettingsGame {
    public:

        static SettingsGame& Get();
        const VideoData& GetVideo() const { return m_Video; }
        const AudioData& GetAudio() const { return m_Audio; }
        const Controls& GetControls() const { return m_Controls; }

        float GetVolume(AudioType type) const;
        void SetMasterVolume(float volume) { m_Audio.MasterVolume = volume; }
        void SetMusicVolume(float volume) { m_Audio.MusicVolume = volume; }
        void SetSFXVolume(float volume) { m_Audio.SFXVolume = volume; }

        void SetFullscreen(bool fullscreen) { m_Video.Fullscreen = fullscreen; }
        void SetFPS(int fps) {m_Video.FPS = fps; }

        void ApplyWindow(RenderWindow* window);
        void ApplyAudio(const std::vector<IAudioVoice*>& audio,  bool music);
        void BindKey(PlayerAction action, Inputs::KeyCode newKey);

        void Save();
        void Load();


        private:
        SettingsGame() = default;
        ~SettingsGame() = default;

        VideoData m_Video;
        AudioData m_Audio;
        Controls m_Controls;

    };
}