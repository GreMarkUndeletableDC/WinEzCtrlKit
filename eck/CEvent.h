#pragma once
#include "CWaitableObject.h"
#include "Utility.h"

ECK_NAMESPACE_BEGIN
class CEvent : public CWaitableObject
{
public:
    explicit CEvent(std::nullopt_t) noexcept {}

    CEvent() noexcept { Create(); }

    CEvent(
        std::wstring_view svName,
        BOOL bManualReset,
        BOOL bInitialState,
        BOOL bInheritHandle = FALSE) noexcept
    {
        Create(svName, bManualReset, bInitialState, bInheritHandle);
    }

    NTSTATUS Create() noexcept
    {
        return NtCreateEvent(
            AtClear(),
            EVENT_ALL_ACCESS,
            nullptr,
            SynchronizationEvent,
            FALSE);
    }

    NTSTATUS Create(
        std::wstring_view svName,
        BOOL bManualReset,
        BOOL bInitialState,
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
        return NtCreateEvent(
            AtClear(),
            EVENT_ALL_ACCESS, &oa,
            bManualReset ? NotificationEvent : SynchronizationEvent,
            bInitialState);
    }

    EckInline NTSTATUS Signal(_Out_opt_ LONG* plPrevState = nullptr) noexcept
    {
        return NtSetEvent(m_hObject, plPrevState);
    }
    EckInline NTSTATUS NoSignal() noexcept
    {
        return NtClearEvent(m_hObject);
    }
    EckInline NTSTATUS NoSignal(_Out_opt_ LONG* plPrevState) noexcept
    {
        return NtResetEvent(m_hObject, plPrevState);
    }
};
ECK_NAMESPACE_END