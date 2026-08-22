#pragma once
#include "NativeWrapper.h"
#include "ComPtr.h"
#include "AutoPointer.h"
#include "CString.h"
#include "Check.h"
#include "CRegistryKey.h"

#include <intrin.h>
#include <wbemcli.h>

ECK_NAMESPACE_BEGIN
const CStringW& GetRunningPath() noexcept;// ECK.cpp

inline COLORREF GetCursorPositionColor() noexcept
{
    POINT pt;
    GetCursorPos(&pt);
    const auto hDC = GetDC(nullptr);
    const auto cr = GetPixel(hDC, pt.x, pt.y);
    ReleaseDC(nullptr, hDC);
    return cr;
}

inline HRESULT WmiConnectNamespace(
    _Out_ IWbemServices*& pWbemServices,
    _Out_ IWbemLocator*& pWbemLocator,
    _In_opt_z_ PCWSTR pszNamespace = L"ROOT\\CIMV2") noexcept
{
    pWbemServices = nullptr;
    pWbemLocator = nullptr;
    HRESULT hr;

    ComPtr<IWbemLocator> pLocator;
    hr = pLocator.CreateInstance(CLSID_WbemLocator);
    if (FAILED(hr))
        return hr;

    ComPtr<IWbemServices> pServices;
    const UniquePtr<DelBStr> bstrNamespace{ SysAllocString(pszNamespace) };
    if (!bstrNamespace)
        return E_OUTOFMEMORY;
    hr = pLocator->ConnectServer(bstrNamespace.get(),
        nullptr, nullptr, 0, 0, 0, 0, &pServices);
    if (FAILED(hr))
        return hr;

    hr = CoSetProxyBlanket(
        pServices.Get(),
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        nullptr,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE);
    if (FAILED(hr))
        return hr;

    pWbemServices = pServices.Detach();
    pWbemLocator = pLocator.Detach();
    return S_OK;
}

/// <summary>
/// WMI查询类属性
/// </summary>
/// <param name="pszWql">WQL语句</param>
/// <param name="pszProp">属性名</param>
/// <param name="Var">
/// 查询结果，如果失败，函数使用VariantClear清除此结构，
/// 调用方不再使用此结构时也应调用VariantClear
/// </param>
/// <param name="pWbemServices">IWbemServices指针，使用此接口执行查询</param>
/// <returns>HRESULT</returns>
inline HRESULT WmiQueryClassProperty(
    _In_ PCWSTR pszWql,
    _In_ PCWSTR pszProp,
    _Inout_ VARIANT& Var,
    _In_ IWbemServices* pWbemServices) noexcept
{
    HRESULT hr;

    ComPtr<IEnumWbemClassObject> pEnum;
    const UniquePtr<DelBStr> bstrLang{ SysAllocString(L"WQL") };
    const UniquePtr<DelBStr> bstrWql{ SysAllocString(pszWql) };
    if (!bstrLang || !bstrWql)
        return E_OUTOFMEMORY;
    hr = pWbemServices->ExecQuery(
        bstrLang.get(),
        bstrWql.get(),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnum);
    if (FAILED(hr))
    {
        VariantClear(&Var);
        return hr;
    }

    ComPtr<IWbemClassObject> pClassObject;
    ULONG cReturned;
    hr = pEnum->Next(WBEM_INFINITE, 1, &pClassObject, &cReturned);
    if (FAILED(hr) || cReturned != 1)
    {
        VariantClear(&Var);
        return hr;
    }

    return pClassObject->Get(pszProp, 0, &Var, nullptr, nullptr);
}

/// <summary>
/// WMI查询类属性
/// </summary>
/// <param name="pszWql">WQL语句</param>
/// <param name="pszProp">属性</param>
/// <param name="Var">
/// 查询结果，如果失败，函数使用VariantClear清除此结构，
/// 调用方不再使用此结构时也应调用VariantClear
/// </param>
/// <returns>HRESULT</returns>
inline HRESULT WmiQueryClassProperty(
    _In_ PCWSTR pszWql,
    _In_ PCWSTR pszProp,
    _Inout_ VARIANT& Var) noexcept
{
    HRESULT hr;

    ComPtr<IWbemServices> pServices;
    ComPtr<IWbemLocator> pLocator;
    if (FAILED(hr = WmiConnectNamespace(pServices.AtSelf(), pLocator.AtSelf())))
        return hr;
    return WmiQueryClassProperty(pszWql, pszProp, Var, pServices.Get());
}

struct CPUINFO
{
    CStringW rsVendor;
    CStringW rsBrand;
    CStringW rsSerialNum;
    CStringW rsDescription;
    UINT uL2Cache;// 千字节
    UINT uL3Cache;// 千字节
    UINT uDataWidth;
    UINT cCore;
    UINT cThread;
    UINT uMaxClockSpeed;// 兆赫兹
};

inline HRESULT GetCpuInfomation(CPUINFO& ci) noexcept
{
#if !defined(_M_ARM64) && !defined(_M_ARM)
    int Register[4];
    // 取制造商
    __cpuid(Register, 0);
    std::swap(Register[2], Register[3]);
    ci.rsVendor = EcdMultiByteToWide((PCSTR)&Register[1], 12);
    /* */if (ci.rsVendor == L"GenuineIntel")
        ci.rsVendor = L"Intel Corporation.";
    else if (ci.rsVendor == L"AuthenticAMD" || ci.rsVendor == L"AMD ISBETTER")
        ci.rsVendor = L"Advanced Micro Devices.";
    else if (ci.rsVendor == L"Geode By NSC")
        ci.rsVendor = L"National Semiconductor.";
    else if (ci.rsVendor == L"CyrixInstead")
        ci.rsVendor = L"Cyrix Corp., VIA Inc.";
    else if (ci.rsVendor == L"NexGenDriven")
        ci.rsVendor = L"NexGen Inc., Advanced Micro Devices.";
    else if (ci.rsVendor == L"CentaurHauls")
        ci.rsVendor = L"IDT\\Centaur, Via Inc.";
    else if (ci.rsVendor == L"UMC UMC UMC ")
        ci.rsVendor = L"United Microelectronics Corp.";
    else if (ci.rsVendor == L"RiseRiseRise")
        ci.rsVendor = L"Rise.";
    else if (ci.rsVendor == L"GenuineTMx86" || ci.rsVendor == L"TransmetaCPU")
        ci.rsVendor = L"Transmeta.";
    // 取商标
    char szBrand[49];
    for (int i = 0x80000002; i <= 0x80000004; ++i)
    {
        __cpuid(Register, i);
        memcpy(szBrand + (i - 0x80000002) * 16, Register, sizeof(Register));
    }
    szBrand[48] = '\0';
    ci.rsBrand = EcdMultiByteToWide(szBrand);
    // 取序列号
    ci.rsSerialNum.ReSize(Int32StringBufferSize * 2);
    __cpuid(Register, 1);
    int cch = swprintf(ci.rsSerialNum.Data(), L"%08X%08X", Register[3], Register[0]);
    ci.rsSerialNum.ReSize(cch);

    HRESULT hr;

    ComPtr<IWbemServices> pServices;
    ComPtr<IWbemLocator> pLocator;
    if (FAILED(hr = WmiConnectNamespace(pServices.AtSelf(), pLocator.AtSelf())))
        return hr;

    VARIANT Var{};
    ComPtr<IEnumWbemClassObject> pEnum;
    const UniquePtr<DelBStr> bstrLang{ SysAllocString(L"WQL") };
    const UniquePtr<DelBStr> bstrWql{ SysAllocString(L"Select * From Win32_Processor") };
    if (!bstrLang || !bstrWql)
        return E_OUTOFMEMORY;
    hr = pServices->ExecQuery(
        bstrLang.get(),
        bstrWql.get(),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnum);
    if (FAILED(hr))
        return hr;
    ComPtr<IWbemClassObject> pClassObject;
    ULONG cReturned;
    hr = pEnum->Next(WBEM_INFINITE, 1, &pClassObject, &cReturned);
    if (cReturned == 1)
    {
        if (SUCCEEDED(pClassObject->Get(L"Description", 0, &Var, nullptr, nullptr)))
        {
            ci.rsDescription.AssignBSTR(Var.bstrVal);
            VariantClear(&Var);
        }
        if (SUCCEEDED(pClassObject->Get(L"L2CacheSize", 0, &Var, nullptr, nullptr)))
            ci.uL2Cache = Var.uintVal;
        if (SUCCEEDED(pClassObject->Get(L"L3CacheSize", 0, &Var, nullptr, nullptr)))
            ci.uL3Cache = Var.uintVal;
        if (SUCCEEDED(pClassObject->Get(L"DataWidth", 0, &Var, nullptr, nullptr)))
            ci.uDataWidth = Var.uiVal;
        if (SUCCEEDED(pClassObject->Get(L"NumberOfCores", 0, &Var, nullptr, nullptr)))
            ci.cCore = Var.uintVal;
        if (SUCCEEDED(pClassObject->Get(L"ThreadCount", 0, &Var, nullptr, nullptr)))
            ci.cThread = Var.uintVal;
        if (SUCCEEDED(pClassObject->Get(L"MaxClockSpeed", 0, &Var, nullptr, nullptr)))
            ci.uMaxClockSpeed = Var.uintVal;
    }
    return S_OK;
#else
    return E_NOTIMPL;
#endif// __arm__
}

inline BOOL GetDesktopPartHandle(
    _Out_ HWND& hPmOrWorkerW,
    _Out_ HWND& hDefView,
    _Out_ HWND& hLV) noexcept
{
    hDefView = hLV = nullptr;
    if (!(hPmOrWorkerW = FindWindowW(L"Progman", L"Program Manager")))
        return FALSE;
    if (hDefView = FindWindowExW(hPmOrWorkerW, nullptr, L"SHELLDLL_DefView", nullptr))
        if (hLV = FindWindowExW(hDefView, nullptr, L"SysListView32", nullptr))
            return TRUE;
    hPmOrWorkerW = nullptr;
    EckCounterNV(1024)
    {
        if (!(hPmOrWorkerW = FindWindowExW(0, hPmOrWorkerW, L"WorkerW", nullptr)))
            break;
        if (!(hDefView = FindWindowExW(hPmOrWorkerW, nullptr, L"SHELLDLL_DefView", nullptr)))
            continue;
        if (hLV = FindWindowExW(hDefView, nullptr, L"SysListView32", nullptr))
            return TRUE;
    }
    return FALSE;
}

inline BOOL ShowDesktop(BOOL bShow, BOOL bIgnoreProgman = TRUE) noexcept
{
    HWND hPmOrWorkerW, hDefView, hLV;
    if (!GetDesktopPartHandle(hPmOrWorkerW, hDefView, hLV))
        return FALSE;
    const int sw = (bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
    ShowWindowAsync(hLV, sw);
    ShowWindowAsync(hDefView, sw);
    if (!bIgnoreProgman)
        ShowWindowAsync(hPmOrWorkerW, sw);
    return TRUE;
}

inline BOOL GetTaskBarPartHandle(
    _Out_ HWND& hTaskBar,
    _Out_writes_opt_(*pcSecondary) HWND* phSecondary = nullptr,
    _Inout_opt_ size_t* pcSecondary = nullptr) noexcept
{
    hTaskBar = FindWindowW(L"Shell_TrayWnd", nullptr);
    if (!hTaskBar)
    {
        if (pcSecondary) *pcSecondary = 0u;
        return FALSE;
    }
    if (phSecondary && pcSecondary)
    {
        HWND hLast{};
        size_t c{};
        while (hLast = FindWindowExW(nullptr, hLast,
            L"Shell_SecondaryTrayWnd", nullptr))
        {
            *phSecondary++ = hLast;
            if (++c == *pcSecondary)
                break;
        }
        *pcSecondary = c;
    }
    return TRUE;
}

inline BOOL ShowTaskBar(BOOL bShow, BOOL bIgnoreSecondary = FALSE) noexcept
{
    HWND hTaskBar, hSecondary[16];
    size_t cSecondary = ARRAYSIZE(hSecondary);
    if (!GetTaskBarPartHandle(
        hTaskBar,
        hSecondary,
        bIgnoreSecondary ? nullptr : &cSecondary))
        return FALSE;
    const int sw = (bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
    if (bIgnoreSecondary)
        ShowWindowAsync(hTaskBar, sw);
    else// 使用ShowWindowAsync会导致副屏任务栏隐藏后再次显示
    {
        ShowWindow(hTaskBar, sw);
        EckCounter(cSecondary, i)
            ShowWindow(hSecondary[i], sw);
    }
    return TRUE;
}

struct FILEVERINFO
{
    CStringW Comment;
    CStringW InternalName;
    CStringW ProductName;
    CStringW CompanyName;
    CStringW LegalCopyright;
    CStringW ProductVersion;
    CStringW FileDescription;
    CStringW LegalTrademarks;
    CStringW PrivateBuild;
    CStringW FileVersion;
    CStringW OriginalFilename;
    CStringW SpecialBuild;
};

inline BOOL GetFileVersionInformation(
    _In_z_ PCWSTR pszFile,
    Eck_Out_buffer_ FILEVERINFO& fvi) noexcept
{
    const DWORD cbBuf = GetFileVersionInfoSizeW(pszFile, nullptr);
    if (!cbBuf)
        return FALSE;
    const UniquePtr<DelMA<void>> pBuf{ malloc(cbBuf) };
    CheckPointer(pBuf.get());
    if (!GetFileVersionInfoW(pszFile, 0, cbBuf, pBuf.get()))
        return FALSE;

    struct
    {
        WORD wLanguage;
        WORD wCodePage;
    }*pLangCp;
    UINT cbLangCp;
    if (!VerQueryValueW(
        pBuf.get(),
        LR"(\VarFileInfo\Translation)",
        (void**)&pLangCp,
        &cbLangCp))
        return FALSE;

    WCHAR szLangCp[9];
    _swprintf(szLangCp, L"%04X%04X", pLangCp[0].wLanguage, pLangCp[0].wCodePage);

    constexpr WCHAR StringBlock[]{ LR"(\StringFileInfo\)" };
    WCHAR szSub[ARRAYSIZE(StringBlock) + ARRAYSIZE(szLangCp) + 24];
    auto p = szSub;
    EckCopyConstStringW(p, StringBlock);
    p += (ARRAYSIZE(StringBlock) - 1);
    EckCopyConstStringW(p, szLangCp);
    p += (ARRAYSIZE(szLangCp) - 1);
    *p++ = L'\\';

    void* pStr;
    UINT cchStr;
    EckCopyConstStringW(p, L"Comment");
    VerQueryValueW(pBuf.get(), szSub, &pStr, &cchStr);
    fvi.Comment.Assign((PCWSTR)pStr, (int)cchStr);

    EckCopyConstStringW(p, L"InternalName");
    VerQueryValueW(pBuf.get(), szSub, &pStr, &cchStr);
    fvi.InternalName.Assign((PCWSTR)pStr, (int)cchStr);

    EckCopyConstStringW(p, L"ProductName");
    VerQueryValueW(pBuf.get(), szSub, &pStr, &cchStr);
    fvi.ProductName.Assign((PCWSTR)pStr, (int)cchStr);

    EckCopyConstStringW(p, L"CompanyName");
    VerQueryValueW(pBuf.get(), szSub, &pStr, &cchStr);
    fvi.CompanyName.Assign((PCWSTR)pStr, (int)cchStr);

    EckCopyConstStringW(p, L"LegalCopyright");
    VerQueryValueW(pBuf.get(), szSub, &pStr, &cchStr);
    fvi.LegalCopyright.Assign((PCWSTR)pStr, (int)cchStr);

    EckCopyConstStringW(p, L"ProductVersion");
    VerQueryValueW(pBuf.get(), szSub, &pStr, &cchStr);
    fvi.ProductVersion.Assign((PCWSTR)pStr, (int)cchStr);

    EckCopyConstStringW(p, L"FileDescription");
    VerQueryValueW(pBuf.get(), szSub, &pStr, &cchStr);
    fvi.FileDescription.Assign((PCWSTR)pStr, (int)cchStr);

    EckCopyConstStringW(p, L"LegalTrademarks");
    VerQueryValueW(pBuf.get(), szSub, &pStr, &cchStr);
    fvi.LegalTrademarks.Assign((PCWSTR)pStr, (int)cchStr);

    EckCopyConstStringW(p, L"PrivateBuild");
    VerQueryValueW(pBuf.get(), szSub, &pStr, &cchStr);
    fvi.PrivateBuild.Assign((PCWSTR)pStr, (int)cchStr);

    EckCopyConstStringW(p, L"FileVersion");
    VerQueryValueW(pBuf.get(), szSub, &pStr, &cchStr);
    fvi.FileVersion.Assign((PCWSTR)pStr, (int)cchStr);

    EckCopyConstStringW(p, L"OriginalFilename");
    VerQueryValueW(pBuf.get(), szSub, &pStr, &cchStr);
    fvi.OriginalFilename.Assign((PCWSTR)pStr, (int)cchStr);

    EckCopyConstStringW(p, L"SpecialBuild");
    VerQueryValueW(pBuf.get(), szSub, &pStr, &cchStr);
    fvi.SpecialBuild.Assign((PCWSTR)pStr, (int)cchStr);
    return TRUE;
}

inline void InputChar(WCHAR ch, BOOL bReplaceEndOfLine = TRUE) noexcept
{
    INPUT input[2]{ {.type = INPUT_KEYBOARD } };
    if (bReplaceEndOfLine && (ch == L'\r' || ch == L'\n'))
        input[0].ki.wVk = VK_RETURN;
    else
    {
        input[0].ki.wScan = ch;
        input[0].ki.dwFlags = KEYEVENTF_UNICODE;
    }

    input[1] = input[0];
    input[1].ki.dwFlags |= KEYEVENTF_KEYUP;
    SendInput(ARRAYSIZE(input), input, sizeof(INPUT));
}

inline void InputChar(
    _In_reads_or_z_(cchText) PCWSTR pszText,
    int cchText = -1,
    BOOL bReplaceEndOfLine = TRUE) noexcept
{
    if (cchText < 0)
        cchText = (int)TcsLength(pszText);
    if (bReplaceEndOfLine)
        for (int i{}; i < cchText;)
        {
            auto ch = pszText[i++];
            if (ch == L'\r' && i < cchText && pszText[i] == L'\n')
            {
                ch = L'\n';
                ++i;
            }
            InputChar(ch, bReplaceEndOfLine);
        }
    else
    {
        EckCounter(cchText, i)
            InputChar(pszText[i], bReplaceEndOfLine);
    }
}

inline NTSTATUS RestartExplorer() noexcept
{
    const auto hWnd = FindWindowW(L"Shell_TrayWnd", nullptr);
    if (!hWnd)
        return STATUS_NOT_FOUND;
    DWORD dwProcessId;
    GetWindowThreadProcessId(hWnd, &dwProcessId);
    NTSTATUS nts;
    const auto hProcess = NaOpenProcess(
        PROCESS_TERMINATE, FALSE, dwProcessId, &nts);
    if (hProcess)
    {
        NtTerminateProcess(hProcess, 2);
        NtClose(hProcess);
    }
    return nts;
}

inline SIZE GetCursorSize(int iDpi) noexcept
{
    HKEY hKey;
    DWORD dwBaseSize{}, cb{ sizeof(DWORD) };
    RegOpenKeyExW(HKEY_CURRENT_USER, LR"(Control Panel\Cursors)",
        0, KEY_READ, &hKey);
    if (hKey)
    {
        RegQueryValueExW(hKey, L"CursorBaseSize", nullptr,
            nullptr, (PBYTE)&dwBaseSize, &cb);
        RegCloseKey(hKey);
    }
    return { (int)dwBaseSize,(int)dwBaseSize };
}

inline NTSTATUS ExpandEnvironmentString(
    Eck_Append_buffer_ CStringW& rsDst,
    _In_reads_or_z_(cchSrc) PCWCH pszSrc,
    int cchSrc = -1,
    int cchInitialBuf = 80) noexcept
{
    if (cchSrc < 0)
        cchSrc = (int)TcsLength(pszSrc);
    const auto cbIn = USHORT(cchSrc * sizeof(WCHAR));
    UNICODE_STRING usIn{ cbIn,cbIn,(PWCH)pszSrc };
    ULONG cbOut = ULONG(cchInitialBuf * sizeof(WCHAR));
    rsDst.Reserve(rsDst.Size() + cchInitialBuf);
    UNICODE_STRING usOut{ (USHORT)cbOut,(USHORT)cbOut,rsDst.Data() + rsDst.Size() };
    auto nts = RtlExpandEnvironmentStrings_U(nullptr, &usIn, &usOut, &cbOut);
    if (NT_SUCCESS(nts))
    {
        rsDst.ReSize(rsDst.Size() + int(cbOut / sizeof(WCHAR)));
        return nts;
    }
    else if (nts == STATUS_BUFFER_TOO_SMALL)
    {
        usOut.Length = usOut.MaximumLength = (USHORT)cbOut;
        usOut.Buffer = rsDst.PushBackNoExtra(int(cbOut / sizeof(WCHAR)));
        return RtlExpandEnvironmentStrings_U(nullptr, &usIn, &usOut, &cbOut);
    }
    else
        return nts;
}

inline ARGB GetDwmColorizationColor() noexcept
{
    CRegistryKey Key{};
    Key.Open(HKEY_CURRENT_USER, LR"(Software\Microsoft\Windows\DWM)", KEY_READ);
    return (ARGB)Key.QueryValueDword(L"ColorizationColor");
}
ECK_NAMESPACE_END