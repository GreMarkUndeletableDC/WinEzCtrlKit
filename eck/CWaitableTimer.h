#pragma once
#include "CWaitableObject.h"
#include "Utility.h"

ECK_NAMESPACE_BEGIN
class CWaitableTimer : public CWaitableObject
{
public:
    explicit CWaitableTimer(std::nullopt_t) noexcept {}

    CWaitableTimer() noexcept { Create(); }

    CWaitableTimer(
        std::wstring_view svName,
        BOOL bManualReset = FALSE,
        BOOL bInheritHandle = FALSE) noexcept
    {
        Create(svName, bManualReset, bInheritHandle);
    }

    NTSTATUS Create() noexcept
    {
        return NtCreateTimer(
            AtClear(),
            TIMER_ALL_ACCESS,
            nullptr,
            SynchronizationTimer);
    }

    NTSTATUS Create(
        std::wstring_view svName,
        BOOL bManualReset = FALSE,
        BOOL bInheritHandle = FALSE) noexcept
    {
        const auto us{ StringViewToNtString(svName) };
        OBJECT_ATTRIBUTES oa;
        InitializeObjectAttributes(
            &oa,
            (svName.empty() ? nullptr : &us),
            (bInheritHandle ? OBJ_INHERIT : 0),
            nullptr,
            nullptr);
        return NtCreateTimer(
            AtClear(),
            TIMER_ALL_ACCESS,
            &oa,
            bManualReset ? NotificationTimer : SynchronizationTimer);
    }

    EckInline NTSTATUS SetDueTime(_In_ const LARGE_INTEGER* pliDueTime) noexcept
    {
        return NtSetTimer(
            m_hObject,
            (LARGE_INTEGER*)pliDueTime,
            nullptr,
            nullptr,
            FALSE,
            0,
            nullptr);
    }

    EckInline NTSTATUS SetDueTimeAndPeriod(
        _In_ const LARGE_INTEGER* pliDueTime,
        LONG msPeriod) noexcept
    {
        return NtSetTimer(
            m_hObject,
            (LARGE_INTEGER*)pliDueTime,
            nullptr,
            nullptr,
            FALSE,
            msPeriod,
            nullptr);
    }

    EckInline NTSTATUS SetDueTimeMs(LONGLONG ms) noexcept
    {
        const LARGE_INTEGER liDueTime{ .QuadPart = ms * -10000ll };
        return SetDueTime(&liDueTime);
    }

    EckInline NTSTATUS SetDueTimeAndPeriodMs(LONGLONG ms, LONG msPeriod) noexcept
    {
        const LARGE_INTEGER liDueTime{ .QuadPart = ms * -10000ll };
        return SetDueTimeAndPeriod(&liDueTime, msPeriod);
    }

    EckInline NTSTATUS Cancel(_Out_opt_ BOOLEAN* pbPrevState = nullptr) noexcept
    {
        return NtCancelTimer(m_hObject, pbPrevState);
    }

    NTSTATUS QueryState(
        _Out_ BOOLEAN& bState,
        _Out_ LONGLONG& llRemainingTime) noexcept
    {
        TIMER_BASIC_INFORMATION tbi;
        ULONG cbRet;
        const auto nts = NtQueryTimer(m_hObject, TimerBasicInformation,
            &tbi, sizeof(tbi), &cbRet);
        if (NT_SUCCESS(nts))
        {
            bState = tbi.TimerState;
            llRemainingTime = tbi.RemainingTime.QuadPart;
        }
        else
        {
            bState = FALSE;
            llRemainingTime = 0;
        }
        return nts;
    }
};
ECK_NAMESPACE_END