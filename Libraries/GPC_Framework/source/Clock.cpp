#include "../include/Clock.h"

namespace GPC {

    void Clock::Restart(double t) {
        m_DeltaTime = 0.0;
        m_UnscaleDeltaTime = 0.0;
        m_Time = 0.0;
        m_UnscaleTime = 0.0;
        m_LastUpdateTime = clock::now();
    }

    void Clock::Update() {
        auto t = clock::now();
        auto time = t - m_LastUpdateTime;
        uint64_t time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time).count();
        constexpr double NANOSECONDS_TO_SECOND = 0.000000001;
        Seconds time_s = static_cast<double>(time_ns) * NANOSECONDS_TO_SECOND;

        m_UnscaleTime += time_s;
        m_Time += time_s * m_TimeScale;

        m_UnscaleDeltaTime = time_s;
        m_DeltaTime = time_s * m_TimeScale;

        m_LastUpdateTime = t;
    }

    double Clock::GetDeltaTime() const {
        return m_DeltaTime;
    }

    double Clock::GetDeltaTimeUnscaled() const {
        return m_UnscaleDeltaTime;
    }

    double Clock::GetTime() const {
        return m_Time;
    }

    double Clock::GetTimeUnscaled() const {
        return m_UnscaleTime;
    }

    double Clock::GetTimeScale() const {
        return m_TimeScale;
    }

    void Clock::SetTimeScale(double scale) {
        m_TimeScale = scale;
    }
} // GPC