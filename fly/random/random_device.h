#pragma once

#include <chrono>
#include <cstdlib>
#include <random>

#include <fly/fly.h>

namespace fly {

/**
 * Wrapper around STL random number generator to provide simpler construction
 * of RNGs for various random engines and distributions.
 *
 * Examples:
 *
 *     UniformIntegerDevice<int, std::default_random_engine> device(10, 20);
 *     int randomInt = device();
 *
 *     BernoulliDevice<std::mt19937> device(0.5);
 *     bool randomBool = device();
 *
 *     NormalDevice<float, std::minstd_real> device(20.0, 5.0);
 *     float randomFloat = device();
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version February 3, 2016
 */
template <typename T, typename E = std::default_random_engine>
class RandomDevice
{
public:
    /**
     * Construct the random engine and seed it with a value based on the system
     * clock time.
     */
    FLY_API RandomDevice();

    /**
     * Seed the random engine.
     */
    FLY_API void Seed(T);

    /**
     * Generate a random number.
     */
    FLY_API virtual T operator()() = 0;

protected:
    E m_engine;
};

template <typename T, typename E>
RandomDevice<T, E>::RandomDevice() : m_engine()
{
    auto now = std::chrono::system_clock::now();
    time_t seed = std::chrono::system_clock::to_time_t(now);

    m_engine.seed(static_cast<typename E::result_type>(seed));
}

template <typename T, typename E>
void RandomDevice<T, E>::Seed(T seed)
{
    m_engine.seed(seed);
}

//==============================================================================
// Normal distribution
//==============================================================================
template <typename T, typename E = std::default_random_engine>
class NormalDevice : public RandomDevice<T, E>
{
public:
    FLY_API NormalDevice();
    FLY_API NormalDevice(T, T);
    FLY_API T operator()();

private:
    std::normal_distribution<T> m_distribution;
};

template <typename T, typename E>
NormalDevice<T, E>::NormalDevice() : RandomDevice<T, E>(), m_distribution()
{
}

template <typename T, typename E>
NormalDevice<T, E>::NormalDevice(T mean, T stddev) : RandomDevice<T, E>(), m_distribution(mean, stddev)
{
}

template <typename T, typename E>
T NormalDevice<T, E>::operator()()
{
    return m_distribution(RandomDevice<T, E>::m_engine);
}

//==============================================================================
// Uniform integer distribution
//==============================================================================
template <typename T, typename E = std::default_random_engine>
class UniformIntegerDevice : public RandomDevice<T, E>
{
public:
    FLY_API UniformIntegerDevice();
    FLY_API UniformIntegerDevice(T, T);
    FLY_API T operator()();

private:
    std::uniform_int_distribution<T> m_distribution;
};

template <typename T, typename E>
UniformIntegerDevice<T, E>::UniformIntegerDevice() : RandomDevice<T, E>(), m_distribution()
{
}

template <typename T, typename E>
UniformIntegerDevice<T, E>::UniformIntegerDevice(T min, T max) : RandomDevice<T, E>(), m_distribution(min, max)
{
}

template <typename T, typename E>
T UniformIntegerDevice<T, E>::operator()()
{
    return m_distribution(RandomDevice<T, E>::m_engine);
}

//==============================================================================
// Uniform real distribution
//==============================================================================
template <typename T, typename E = std::default_random_engine>
class UniformRealDevice : public RandomDevice<T, E>
{
public:
    FLY_API UniformRealDevice();
    FLY_API UniformRealDevice(T, T);
    FLY_API T operator()();

private:
    std::uniform_real_distribution<T> m_distribution;
};

template <typename T, typename E>
UniformRealDevice<T, E>::UniformRealDevice() : RandomDevice<T, E>(), m_distribution()
{
}

template <typename T, typename E>
UniformRealDevice<T, E>::UniformRealDevice(T min, T max) : RandomDevice<T, E>(), m_distribution(min, max)
{
}

template <typename T, typename E>
T UniformRealDevice<T, E>::operator()()
{
    return m_distribution(RandomDevice<T, E>::m_engine);
}

//==============================================================================
// Bernoulli distribution
//==============================================================================
template <typename E = std::default_random_engine>
class BernoulliDevice : public RandomDevice<bool, E>
{
public:
    FLY_API BernoulliDevice();
    FLY_API BernoulliDevice(double);
    FLY_API bool operator()();

private:
    std::bernoulli_distribution m_distribution;
};

template <typename E>
BernoulliDevice<E>::BernoulliDevice() : RandomDevice<bool, E>(), m_distribution()
{
}

template <typename E>
BernoulliDevice<E>::BernoulliDevice(double trueProbability) : RandomDevice<bool, E>(), m_distribution(trueProbability)
{
}

template <typename E>
bool BernoulliDevice<E>::operator()()
{
    return m_distribution(RandomDevice<bool, E>::m_engine);
}

//==============================================================================
// Binomial distribution
//==============================================================================
template <typename T, typename E = std::default_random_engine>
class BinomialDevice : public RandomDevice<T, E>
{
public:
    FLY_API BinomialDevice();
    FLY_API BinomialDevice(T, double);
    FLY_API T operator()();

private:
    std::binomial_distribution<T> m_distribution;
};

template <typename T, typename E>
BinomialDevice<T, E>::BinomialDevice() : RandomDevice<T, E>(), m_distribution()
{
}

template <typename T, typename E>
BinomialDevice<T, E>::BinomialDevice(T max, double probability) : RandomDevice<T, E>(), m_distribution(max, probability)
{
}

template <typename T, typename E>
T BinomialDevice<T, E>::operator()()
{
    return m_distribution(RandomDevice<T, E>::m_engine);
}

//==============================================================================
// Geometric distribution
//==============================================================================
template <typename T, typename E = std::default_random_engine>
class GeometricDevice : public RandomDevice<T, E>
{
public:
    FLY_API GeometricDevice();
    FLY_API GeometricDevice(double);
    FLY_API T operator()();

private:
    std::geometric_distribution<T> m_distribution;
};

template <typename T, typename E>
GeometricDevice<T, E>::GeometricDevice() : RandomDevice<T, E>(), m_distribution()
{
}

template <typename T, typename E>
GeometricDevice<T, E>::GeometricDevice(double probability) : RandomDevice<T, E>(), m_distribution(probability)
{
}

template <typename T, typename E>
T GeometricDevice<T, E>::operator()()
{
    return m_distribution(RandomDevice<T, E>::m_engine);
}

//==============================================================================
// Exponential distribution
//==============================================================================
template <typename T, typename E = std::default_random_engine>
class ExponentialDevice : public RandomDevice<T, E>
{
public:
    FLY_API ExponentialDevice();
    FLY_API ExponentialDevice(T);
    FLY_API T operator()();

private:
    std::exponential_distribution<T> m_distribution;
};

template <typename T, typename E>
ExponentialDevice<T, E>::ExponentialDevice() : RandomDevice<T, E>(), m_distribution()
{
}

template <typename T, typename E>
ExponentialDevice<T, E>::ExponentialDevice(T lambda) : RandomDevice<T, E>(), m_distribution(lambda)
{
}

template <typename T, typename E>
T ExponentialDevice<T, E>::operator()()
{
    return m_distribution(RandomDevice<T, E>::m_engine);
}

}
