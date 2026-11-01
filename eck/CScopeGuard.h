#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
template<class F>
class CScopeGuard
{
private:
    F m_Fn;
    bool m_bActive;
public:
    template<class I>
    explicit CScopeGuard(I&& f) noexcept
        : m_Fn{ std::forward<I>(f) }, m_bActive{ true } {}

    CScopeGuard(CScopeGuard&& x) noexcept
        : m_Fn{ std::move(x.m_Fn) }, m_bActive{ x.m_bActive }
    {
        x.m_bActive = false;
    }
    CScopeGuard& operator=(CScopeGuard&& x) noexcept
    {
        if (&x == this)
            return *this;
        m_Fn = std::move(x.m_Fn);
        x.m_bActive = false;
    }

    CScopeGuard(const CScopeGuard&) = delete;
    CScopeGuard& operator=(const CScopeGuard&) = delete;

    ~CScopeGuard()
    {
        if (m_bActive)
            m_Fn();
    }

    EckInlineCe void Cancel() noexcept { m_bActive = false; }
};

template<class F>
CScopeGuard(F) -> CScopeGuard<std::decay_t<F>>;
ECK_NAMESPACE_END