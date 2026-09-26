#include "pch.h"

#include "CWindowMain.h"


void CWindowMain::OnDestory()
{
}

LRESULT CWindowMain::OnCreate()
{
    eck::PtcCurrent()->UpdateDefaultColor();
    return 0;
}

LRESULT CWindowMain::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (uMsg)
    {
    case WM_SIZE:
    {
        const auto lResult = __super::OnMessage(uMsg, wParam, lParam);
        return lResult;
    }

    case WM_CREATE:
    {
        const auto lResult = __super::OnMessage(uMsg, wParam, lParam);
        OnCreate();
        return lResult;
    }
    case WM_DESTROY:
    {
        const auto lResult = __super::OnMessage(uMsg, wParam, lParam);
        OnDestory();
        PostQuitMessage(0);
        return lResult;
    }

    case WM_SYSCOLORCHANGE:
        eck::MsgOnSystemColorChangeMainWindow(Handle, wParam, lParam);
        break;
    case WM_SETTINGCHANGE:
        if (eck::MsgOnSettingChangeMainWindow(Handle, wParam, lParam))
        {
            Redraw();
        }
        break;
    case WM_DPICHANGED:
        SetUserDpi(LOWORD(wParam));
        break;
    case WM_DWMCOLORIZATIONCOLORCHANGED:
        Redraw();
        break;
    }
    return __super::OnMessage(uMsg, wParam, lParam);
}