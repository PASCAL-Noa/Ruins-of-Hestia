#pragma once

#include "vector"
#include "AudioSystem.h"

namespace GPC
{
    struct RandomAudio
    {
        std::vector<AudioSource*> Sources;

        int LastIndex = -1;

        void Play()
        {
            if (Sources.empty()) return;
            if (Sources.size() == 1) { Sources[0]->Play(); return; }

            int idx;
            do { idx = rand() % Sources.size(); } while (idx == LastIndex);
            LastIndex = idx;
            Sources[idx]->Play();
        }
    };
}