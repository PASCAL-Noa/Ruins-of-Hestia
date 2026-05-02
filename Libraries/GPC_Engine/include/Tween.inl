namespace GPC
{
    template<typename T>
    Tween<T>::Tween(const TweenConfig<T> &config) : m_Config(config)
    {
        m_EaseFunc = Tweening::GetFunction(m_Config.Ease);
    }

    template<typename T>
    void Tween<T>::Update(float dt)
    {
        if (IsFinished())
            return;

        m_Elapsed += dt;

        if (IsFinished())
        {
            m_Config.Setter(m_Config.End);
            return;
        }

        float t = m_Elapsed / m_Config.Duration;
        float easedT = m_EaseFunc(t);

        T currentValue = m_Config.Start + (m_Config.End - m_Config.Start) * easedT;
        m_Config.Setter(currentValue);
    }
}