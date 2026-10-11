#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CConditionVariable
{
private:
    RTL_CONDITION_VARIABLE m_cv;
public:
    ECK_DISABLE_COPY_MOVE(CConditionVariable)
public:
    CConditionVariable() noexcept
    {
        RtlInitializeConditionVariable(&m_cv);
    }

    EckInline NTSTATUS Sleep(
        _In_ RTL_CRITICAL_SECTION* pcs,
        _In_opt_ const LARGE_INTEGER* pli) noexcept
    {
        return RtlSleepConditionVariableCS(&m_cv, pcs, (LARGE_INTEGER*)pli);
    }
    EckInline NTSTATUS Sleep(
        _In_ RTL_SRWLOCK* psrw,
        BOOL bReadLock,
        _In_opt_ const LARGE_INTEGER* pli) noexcept
    {
        return RtlSleepConditionVariableSRW(
            &m_cv,
            psrw,
            (LARGE_INTEGER*)pli,
            bReadLock ? CONDITION_VARIABLE_LOCKMODE_SHARED : 0);
    }

    EckInline NTSTATUS SleepMs(_In_ RTL_CRITICAL_SECTION* pcs, LONGLONG ms) noexcept
    {
        const LARGE_INTEGER li{ .QuadPart = ms * -10000ll };
        return Sleep(pcs, &li);
    }
    EckInline NTSTATUS SleepMs(_In_ RTL_SRWLOCK* psrw, BOOL bReadLock, LONGLONG ms) noexcept
    {
        const LARGE_INTEGER li{ .QuadPart = ms * -10000ll };
        return Sleep(psrw, bReadLock, &li);
    }

    EckInline void Wake() noexcept
    {
        RtlWakeConditionVariable(&m_cv);
    }

    EckInline void WakeAll() noexcept
    {
        RtlWakeAllConditionVariable(&m_cv);
    }

    EckInline auto GetPointer() noexcept { return &m_cv; }
};
ECK_NAMESPACE_END