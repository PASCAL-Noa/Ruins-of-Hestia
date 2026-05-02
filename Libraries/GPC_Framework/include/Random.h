#pragma once
#include <random>

#include "Object.h"

namespace GPC
{

    class Random
    {

    public:
        template <class T = float> requires std::is_floating_point_v<T>
        static T Float(T const& min = 0.0f, T const& max = 1.0f);

        template <class T = float> requires std::is_floating_point_v<T>
        static glm::vec<3, T> Vector3(T const& min = 0.0f, T const& max = 1.0f);

        static glm::vec2 Vector2(float const& min = 0.0f, float const& max = 1.0f);

        template <class T> requires std::is_integral_v<T>
        static T Integer(T const& min, T const& max);

        static uint32_t WeightedDistribution(const float* weights, uint32_t count);

        // Same as WeightedDistribution but the sum of all weights is already equal to 1
        static uint32_t NormalizedWeightedDistribution(const float* weights, uint32_t count);

    private:
        static std::default_random_engine& GetRandomEngine();
    };

    template<class T> requires std::is_floating_point_v<T>
    T Random::Float(T const &min, T const &max)
    {
        std::uniform_real_distribution<T> rndDist(min, max);
        return rndDist(GetRandomEngine());
    }

    template<class T> requires std::is_floating_point_v<T>
    glm::vec<3, T> Random::Vector3(T const &min, T const &max)
    {
        return glm::vec<3, T>(Random(min, max), Random(min, max), Random(min, max));
    }

    template<class T> requires std::is_integral_v<T>
    T Random::Integer(T const &min, T const &max)
    {
        std::uniform_int_distribution<T> rndDist(min, max);
        return rndDist(GetRandomEngine());
    }

}
