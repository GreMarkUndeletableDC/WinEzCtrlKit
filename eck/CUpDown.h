#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
class CUpDown : public CWindow
{
public:
    ECK_RTTI(CUpDown, CWindow);
    ECK_W_ATTACHABLE(CUpDown);
    ECK_W_CREATE_CLASS(UPDOWN_CLASSW);

    ECK_W_STYLE(AlignmentLeft, UDS_ALIGNLEFT);
    ECK_W_STYLE(AlignmentRight, UDS_ALIGNRIGHT);
    ECK_W_STYLE(ArrowKeys, UDS_ARROWKEYS);
    ECK_W_STYLE(AutoBuddy, UDS_AUTOBUDDY);
    ECK_W_STYLE(Horizontal, UDS_HORZ);
    ECK_W_STYLE(NoThousands, UDS_NOTHOUSANDS);
    ECK_W_STYLE(SetBuddyInt, UDS_SETBUDDYINT);
    ECK_W_STYLE(Wrap, UDS_WRAP);

    EckInline int GetAcceleration(
        _Out_writes_opt_(cBuf) UDACCEL* pBuf, int cBuf) const noexcept
    {
        return (int)SendMessageW(UDM_GETACCEL, cBuf, (LPARAM)pBuf);
    }

    EckInline void GetAcceleration(std::vector<UDACCEL>& vAccel) const noexcept
    {
        int cAccel = (int)GetAcceleration(nullptr, 0);
        if (!cAccel)
            return;
        vAccel.resize(cAccel);
        GetAcceleration(vAccel.data(), cAccel);
    }

    EckInline int GetBase() const noexcept
    {
        return (int)SendMessageW(UDM_GETBASE, 0, 0);
    }

    EckInline HWND GetBuddy() const noexcept
    {
        return (HWND)SendMessageW(UDM_GETBUDDY, 0, 0);
    }

    EckInline int GetPosition(_Out_opt_ BOOL* pbSuccess = nullptr) const noexcept
    {
        return (int)SendMessageW(UDM_GETPOS32, 0, (LPARAM)pbSuccess);
    }

    EckInline void GetRange(
        _Out_opt_ int* piMin = nullptr,
        _Out_opt_ int* piMax = nullptr) const noexcept
    {
        SendMessageW(UDM_GETRANGE32, (WPARAM)piMin, (LPARAM)piMax);
    }

    EckInline BOOL SetAcceleration(_In_reads_(c) const UDACCEL* puda, int c) const noexcept
    {
        return (BOOL)SendMessageW(UDM_SETACCEL, c, (LPARAM)puda);
    }

    EckInline BOOL SetBase(int iBase) const noexcept
    {
        return (BOOL)SendMessageW(UDM_SETBASE, iBase, 0);
    }

    EckInline HWND SetBuddy(HWND hBuddy) const noexcept
    {
        return (HWND)SendMessageW(UDM_SETBUDDY, (WPARAM)hBuddy, 0);
    }

    EckInline int SetPosition(int iPos) const noexcept
    {
        return (int)SendMessageW(UDM_SETPOS, 0, iPos);
    }

    EckInline void SetRange(int iMin, int iMax) const noexcept
    {
        SendMessageW(UDM_SETRANGE32, iMin, iMax);
    }

    EckInline void SetMinimum(int iMin) const noexcept
    {
        int iMax;
        GetRange(nullptr, &iMax);
        SetRange(iMin, iMax);
    }

    EckInline void SetMaximum(int iMax) const noexcept
    {
        int iMin;
        GetRange(&iMin, nullptr);
        SetRange(iMin, iMax);
    }
};
ECK_NAMESPACE_END