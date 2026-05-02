#include "Random.h"

namespace GPC
{
    uint32_t Random::NormalizedWeightedDistribution(const float *weights, uint32_t count) {
        float random = Float();
        for (uint32_t i = 0; i < count; ++i) {
            random -= weights[i];
            if (random <= 0.0f) return i;
        }
        return count - 1;
    }

    glm::vec2 Random::Vector2(float const &min, float const &max)
    {
        return glm::vec2(Float(min, max), Float(min, max));
    }

    uint32_t Random::WeightedDistribution(const float *weights, uint32_t count) {
        float sum = 0;
        for (uint32_t i = 0; i < count; ++i) {
            sum += weights[i];
        }
        float random = Float(0.0f, sum);
        for (uint32_t i = 0; i < count; ++i) {
            random -= weights[i];
            if (random <= 0.0f) return i;
        }
        return count - 1;
    }

    std::default_random_engine & Random::GetRandomEngine()
    {
        static std::default_random_engine random_engine = std::default_random_engine(static_cast<unsigned>(time(nullptr)));
        static float f = std::uniform_real_distribution(0.f, 1.f)(random_engine);
        return random_engine;
    }

}
