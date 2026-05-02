#pragma once  
  
#include "IAudioVoice.h"  
#include <span>  
#include <vector>  
  
namespace GPC  
{  
    class AudioMixer  
    {  
    public:  
        void                        Mix(std::span<float> destinationMix);
        void                        AddVoice(IAudioVoice* voice);
        void                        RemoveVoice(IAudioVoice* voice);

        [[nodiscard]] float         GetMasterVolume() const {return m_MasterVolume;}
        void                        SetMasterVolume(float volume);
  
    private:  
        std::vector<IAudioVoice*>   m_Voices;
        std::vector<float>          m_VoiceBuffer;
        float                       m_MasterVolume = 1.0f;
    };
}