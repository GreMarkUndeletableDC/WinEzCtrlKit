#pragma once
#include "CRegistryKey.h"
#include "NativeWrapper.h"
#include "Check.h"

#include <RegStr.h>
#include <Msi.h>

ECK_NAMESPACE_BEGIN
enum class AppwizFlags : UINT
{
    None = 0u,
    NoRemove = 1u << 0, // 未提供卸载选项
    NoModify = 1u << 1, // 未提供修改选项
    NoRepair = 1u << 2, // 未提供修复选项
    WindowsInstaller = 1u << 3,     // 由MSI安装程序安装，且从注册表获取
    SystemComponent = 1u << 4,      // 系统组件
    RegistryCurrentUser = 1u << 5,  // 注册表在HKEY_CURRENT_USER下
    RegistryLocalMachine = 1u << 6, // 注册表在HKEY_LOCAL_MACHINE下
    RegistryWow64 = 1u << 7,        // 仅_WIN64平台有效，表示该卸载项为Wow64程序
    Patch = 1u << 8,                // 补丁
    Msi = 1u << 9,                  // MSI安装程序安装，且从MSIAPI获取
    NoRegistry = 1u << 10,          // 仅用于MSI安装程序，指示查询注册表失败。调用方一般不使用此值
};
ECK_ENUM_BIT_FLAGS(AppwizFlags);

enum class AppwizString
{
    ProductId,      // 手动获取
    RegistryKeyName,// 手动获取

    MinimumImportant,

    DisplayName = MinimumImportant,
    InstallLocation,
    UninstallString,
    Publisher,
    KbNumber,

    MaximumImportant,

    DisplayIcon = MaximumImportant,
    DisplayVersion,
    HelpLink,
    HelpTelephone,
    InstallDate,
    InstallSource,
    UrlInformationAbout,
    UrlUpdateInformation,
    Contact,
    Readme,
    RegisterOwner,
    RegisterCompany,
    Comment,
    QuietUninstallString,
    ModifyPath,

    Maximum
};

namespace Detail
{
    constexpr inline PCWSTR AppwizStringList[size_t(AppwizString::Maximum)]
    {
        L"ProductID",
        L"RegKeyName",

        L"DisplayName",
        L"InstallLocation",
        L"UninstallString",
        L"Publisher",
        L"KBNumber",

        L"DisplayIcon",
        L"DisplayVersion",
        L"HelpLink",
        L"HelpTelephone",
        L"InstallDate",
        L"InstallSource",
        L"URLInfoAbout",
        L"URLUpdateInfo",
        L"Contact",
        L"Readme",
        L"RegOwner",
        L"RegCompany",
        L"Comments",
        L"QuietUninstallString",
        L"ModifyPath",
    };

    constexpr inline PCWSTR AppwizStringListMsi[size_t(AppwizString::Maximum)]
    {
        L"ProductID",
        L"RegKeyName",// 与ProductID相同

        INSTALLPROPERTY_PRODUCTNAME,// 注意：INSTALLPROPERTY_DISPLAYNAME用于补丁
        INSTALLPROPERTY_INSTALLLOCATION,
        L"/UninstallString",
        INSTALLPROPERTY_PUBLISHER,
        L"/KBNumber",

        INSTALLPROPERTY_PRODUCTICON,
        INSTALLPROPERTY_VERSIONSTRING,
        INSTALLPROPERTY_HELPLINK,
        INSTALLPROPERTY_HELPTELEPHONE,
        INSTALLPROPERTY_INSTALLDATE,
        INSTALLPROPERTY_INSTALLSOURCE,
        INSTALLPROPERTY_URLINFOABOUT,
        INSTALLPROPERTY_URLUPDATEINFO,
        L"/Contact",
        L"/Readme",
        L"RegOwner",
        L"RegCompany",
        L"/Comments",
        L"/QuietUninstallString",
        L"/ModifyPath",
    };

    const inline struct
    {
        HKEY hRoot;
        PCWSTR pszSubKey;
        int cchSubKey;
    }
    UninstallSource[]
    {
        { HKEY_LOCAL_MACHINE, EckArgString(REGSTR_PATH_UNINSTALL) },
        { HKEY_CURRENT_USER,  EckArgString(REGSTR_PATH_UNINSTALL) },
#ifdef _WIN64
        { HKEY_LOCAL_MACHINE, EckArgString(L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall") },
        { HKEY_CURRENT_USER,  EckArgString(L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall") },
#endif
    };
}

class CUninstallEnumerator
{
private:
    CRegistryKey m_Reg{};
    CStringW m_rsBuffer{};
    DWORD m_idxCurr{};
    BYTE m_idxCurrSource{};
    BITBOOL m_bAutoCompleteAppInfo : 1{ TRUE };
    BITBOOL m_bSkipWindowsInstaller : 1{ TRUE };
    BITBOOL m_bEnumMsi : 1{ 1 };

    LSTATUS NextSource() noexcept
    {
        if (m_idxCurrSource < ARRAYSIZE(Detail::UninstallSource))
        {
            auto& Source = Detail::UninstallSource[m_idxCurrSource++];
            auto ls = m_Reg.Open(Source.hRoot, Source.pszSubKey, KEY_READ);
            if (ls == ERROR_SUCCESS)
            {
                DWORD cchMaxSub;
                ls = m_Reg.QueryInfomation(nullptr, nullptr, nullptr, &cchMaxSub);
                if (ls != ERROR_SUCCESS)
                    return ls;
                m_rsBuffer.Reserve(cchMaxSub);
                m_idxCurr = 0;
                return ERROR_SUCCESS;
            }
            return NextSource();
        }
        return ERROR_NO_MORE_ITEMS;
    }

    constexpr static AppwizFlags GetRegistryFlags(size_t idxSrc) noexcept
    {
        switch (idxSrc)
        {
        case 0:  return AppwizFlags::RegistryLocalMachine;
        case 1:  return AppwizFlags::RegistryCurrentUser;
        case 2:  return AppwizFlags::RegistryWow64 | AppwizFlags::RegistryLocalMachine;
        case 3:  return AppwizFlags::RegistryWow64 | AppwizFlags::RegistryCurrentUser;
        default: return AppwizFlags::None;
        }
    }
public:
    struct Application
    {
        friend class CUninstallEnumerator;

        CRegistryKey Key{};
        CStringW StringBuffer{};
        DWORD EstimatedSize{};
        AppwizFlags Flags{};
        DWORD MajorVersion{};
        DWORD MinorVersion{};
        struct StringSpan
        {
            int idx;
            int cch;
        } Str[size_t(AppwizString::Maximum)]{};
    private:
        EckInlineNdCe StringSpan GetString(AppwizString e) const noexcept { return Str[size_t(e)]; }

        LSTATUS GetStringValueRegistry(PCWSTR pszValue, _Out_ StringSpan& spResult) noexcept
        {
            LSTATUS ls;
            DWORD cbBuf{};
            if ((ls = Key.QueryValue(pszValue, nullptr, &cbBuf)) != ERROR_SUCCESS)
            {
                spResult = {};
                return ls;
            }
            if (cbBuf)
            {
                const auto cchAdded = cbBuf / sizeof(WCHAR) + 1;
                const auto psBuf = StringBuffer.PushBack((int)cchAdded);
                if ((ls = Key.QueryValue(pszValue, psBuf, &cbBuf)) != ERROR_SUCCESS ||
                    !cbBuf)
                {
                    StringBuffer.PopBack((int)cchAdded);
                    spResult = {};
                    return ls;
                }
                *(psBuf + cchAdded - 1) = 0;
                spResult = { int(psBuf - StringBuffer.Data()),int(cchAdded - 1) };
            }
            else
                spResult = {};
            return ERROR_SUCCESS;
        }

        LSTATUS GetStringValueMsi(PCWSTR pszValue, _Out_ StringSpan& spResult) noexcept
        {
            LSTATUS ls;
            DWORD cchBuf{};// 不含结尾NULL
            ls = (LSTATUS)MsiGetProductInfoW(StringBuffer.Data()/*首部总为ProductID*/,
                pszValue, nullptr, &cchBuf);
            if (!cchBuf)
            {
                spResult = {};
                return ls;
            }
            ++cchBuf;
            const auto pszBuf = StringBuffer.PushBack(cchBuf);
            const auto cchAdded = cchBuf;
            ls = (LSTATUS)MsiGetProductInfoW(StringBuffer.Data()/*首部总为ProductID*/,
                pszValue, pszBuf, &cchBuf);
            if (ls != ERROR_SUCCESS || !cchBuf)
            {
                StringBuffer.PopBack(cchAdded);
                spResult = {};
                return ls;
            }
            spResult = { int(pszBuf - StringBuffer.Data()),int(cchAdded - 1) };
            return ERROR_SUCCESS;
        }

        BOOL AcquireBasicInfomation(BOOL bSkipWindowsInstaller,
            PCWSTR pszKeyOrId, int cchKeyOrId, BOOL bMsi) noexcept
        {
            DWORD dwBuf{}, cbBuf{ sizeof(dwBuf) };
            if (bMsi)
            {
                Flags = AppwizFlags::Msi;// Certainly...
                // 测试位于注册表的哪个源
                EckCounter(ARRAYSIZE(Detail::UninstallSource), i)
                {
                    const auto& Src = Detail::UninstallSource[i];
                    StringBuffer.Assign(Src.pszSubKey, Src.cchSubKey);
                    StringBuffer.PushBackChar(L'\\');
                    StringBuffer.PushBack(pszKeyOrId, cchKeyOrId);
                    if (Key.Open(Src.hRoot, StringBuffer.Data(), KEY_READ) == ERROR_SUCCESS)
                    {
                        Flags |= GetRegistryFlags(i);
                        break;
                    }
                }
                if (!Key.GetHKey())
                    Flags |= AppwizFlags::NoRegistry;

                StringBuffer.Assign(pszKeyOrId, cchKeyOrId + 1/* For null terminator */);
                Str[size_t(AppwizString::ProductId)] = { 0,38 };
            }
            else
            {
                Key.QueryValue(L"WindowsInstaller", &dwBuf, &cbBuf);
                if (dwBuf)
                {
                    // 若有需要，调用方使用此标志判断是否跳过Windows Installer应用
                    Flags |= AppwizFlags::WindowsInstaller;
                    if (bSkipWindowsInstaller)
                        return TRUE;// 停止获取信息
                }
                dwBuf = 0;
            }
            const auto posKey = StringBuffer.Size();
            StringBuffer.PushBack(pszKeyOrId, cchKeyOrId + 1);
            Str[size_t(AppwizString::RegistryKeyName)] = { posKey,StringBuffer.Size() - posKey - 1 };

            Key.QueryValue(L"NoRemove", &dwBuf, &cbBuf);
            if (dwBuf)
                Flags |= AppwizFlags::NoRemove;
            dwBuf = 0;
            Key.QueryValue(L"NoModify", &dwBuf, &cbBuf);
            if (dwBuf)
                Flags |= AppwizFlags::NoModify;
            dwBuf = 0;
            Key.QueryValue(L"NoRepair", &dwBuf, &cbBuf);
            if (dwBuf)
                Flags |= AppwizFlags::NoRepair;
            dwBuf = 0;
            Key.QueryValue(L"SystemComponent", &dwBuf, &cbBuf);
            if (dwBuf)
                Flags |= AppwizFlags::SystemComponent;

            if (bMsi)
            {
                for (size_t i = size_t(AppwizString::MinimumImportant);
                    i < size_t(AppwizString::MaximumImportant); ++i)
                {
                    if (Detail::AppwizStringListMsi[i][0] == '/')
                        GetStringValueRegistry(Detail::AppwizStringListMsi[i] + 1, Str[i]);
                    else
                        GetStringValueMsi(Detail::AppwizStringListMsi[i], Str[i]);
                }
            }
            else
            {
                for (size_t i = size_t(AppwizString::MinimumImportant);
                    i < size_t(AppwizString::MaximumImportant); ++i)
                {
                    GetStringValueRegistry(Detail::AppwizStringList[i], Str[i]);
                }

                if (GetString(AppwizString::KbNumber).cch)
                    Flags |= AppwizFlags::Patch;
            }
            return FALSE;
        }

        static DWORD VersionFromString(PCWSTR pszVer, DWORD dwType) noexcept
        {
            if (dwType == REG_DWORD)
                return *(DWORD*)pszVer;
            if (dwType == REG_SZ)
                return _wtoi(pszVer);
            return 0;
        }

        W32ERR CreateProcessForCommand(std::wstring_view svCmd) noexcept
        {
            if (svCmd.empty())
                return ERROR_NOT_SUPPORTED;
            STARTUPINFO si{};
            PROCESS_INFORMATION pi{};
            si.cb = sizeof(si);
            const auto pszBuf = (PWSTR)_malloca((svCmd.size() + 1) * sizeof(WCHAR));
            CheckPointer(pszBuf);
            TcsCopyLength(pszBuf, svCmd.data(), svCmd.size() + 1);
            const auto b = CreateProcessW(nullptr, pszBuf,
                nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
            _freea(pszBuf);
            if (b)
            {
                NtClose(pi.hProcess);
                NtClose(pi.hThread);
                return ERROR_SUCCESS;
            }
            return NaGetLastError();
        }
    public:
        EckInlineNdCe BOOL TestFlag(AppwizFlags f) const noexcept
        {
            return (Flags & f) != AppwizFlags::None;
        }

        EckInlineNdCe BOOL IsWindowsInstaller() const noexcept
        {
            return TestFlag(AppwizFlags::WindowsInstaller);
        }

        // 测试是否为一般意义上的可操作项
        EckInlineNdCe BOOL IsValid() const noexcept
        {
            if (!Str[size_t(AppwizString::DisplayName)].cch ||
                Str[size_t(AppwizString::KbNumber)].cch)
                return FALSE;
            if (TestFlag(AppwizFlags::Msi))
                return !TestFlag(AppwizFlags::NoRegistry);
            else
                return !!Str[size_t(AppwizString::UninstallString)].cch;
        }

        // 提供与 控制面板/程序和功能 相似的过滤条件
        EckInlineNdCe BOOL IsNormalApp() const noexcept
        {
            return IsValid() && !TestFlag(
                AppwizFlags::WindowsInstaller | AppwizFlags::SystemComponent);
        }

        EckInlineNdCe std::wstring_view GetStringView(AppwizString e) const noexcept
        {
            const auto f = GetString(e);
            if (f.cch)
                return { StringBuffer.Data() + f.idx,(size_t)f.cch };
            else
                return {};
        }

        void Clear() noexcept
        {
            Key.Close();
            StringBuffer.Clear();
            EstimatedSize = 0;
            Flags = AppwizFlags::None;
            MajorVersion = 0;
            MinorVersion = 0;
            ZeroMemory(Str, sizeof(Str));
        }

        void AcquireAllInfomation() noexcept
        {
            for (size_t i = size_t(AppwizString::MaximumImportant);
                i < size_t(AppwizString::Maximum); ++i)
                GetStringValueRegistry(Detail::AppwizStringList[i], Str[i]);
            // 补全非字符串信息
            EstimatedSize = 0;
            DWORD cbBuf{ sizeof(EstimatedSize) };
            Key.QueryValue(L"EstimatedSize", &EstimatedSize, &cbBuf);

            // MajorVersion、MinorVersion、VersionMajor、VersionMinor
            // 某些程序填写为字符串，因此使用字符串缓冲区接收
            WCHAR szBuf[Int32StringBufferSize];
            DWORD dwType;
            BOOL bMajorRead{}, bMinorRead{};
            cbBuf = sizeof(szBuf);
            if (Key.QueryValue(L"MajorVersion", szBuf, &cbBuf, &dwType) == ERROR_SUCCESS)
            {
                MajorVersion = VersionFromString(szBuf, dwType);
                bMajorRead = !MajorVersion;
            }
            cbBuf = sizeof(szBuf);
            if (Key.QueryValue(L"MinorVersion", szBuf, &cbBuf, &dwType) == ERROR_SUCCESS)
            {
                MinorVersion = VersionFromString(szBuf, dwType);
                bMinorRead = !MinorVersion;
            }
            if (!bMajorRead)
            {
                cbBuf = sizeof(szBuf);
                if (Key.QueryValue(L"VersionMajor", szBuf, &cbBuf, &dwType) == ERROR_SUCCESS)
                    MajorVersion = VersionFromString(szBuf, dwType);
            }
            if (!bMinorRead)
            {
                cbBuf = sizeof(szBuf);
                if (Key.QueryValue(L"VersionMinor", szBuf, &cbBuf, &dwType) == ERROR_SUCCESS)
                    MinorVersion = VersionFromString(szBuf, dwType);
            }
        }

        W32ERR Repair() noexcept
        {
            if (TestFlag(AppwizFlags::NoRepair))
                return ERROR_NOT_SUPPORTED;
            if (TestFlag(AppwizFlags::Msi))
            {
                return MsiReinstallProductW(GetStringView(AppwizString::ProductId).data(),
                    REINSTALLMODE_USERDATA | REINSTALLMODE_MACHINEDATA |
                    REINSTALLMODE_SHORTCUT | REINSTALLMODE_FILEOLDERVERSION |
                    REINSTALLMODE_FILEVERIFY | REINSTALLMODE_PACKAGE);
            }
            return ERROR_NOT_SUPPORTED;
        }

        W32ERR Uninstall() noexcept
        {
            if (TestFlag(AppwizFlags::NoRemove))
                return ERROR_NOT_SUPPORTED;
            if (TestFlag(AppwizFlags::Msi))
                return MsiConfigureProductW(GetStringView(AppwizString::ProductId).data(),
                    INSTALLSTATE_ABSENT, INSTALLSTATE_ABSENT);
            return CreateProcessForCommand(GetStringView(AppwizString::UninstallString));
        }

        W32ERR Modify() noexcept
        {
            if (TestFlag(AppwizFlags::NoModify))
                return ERROR_NOT_SUPPORTED;
            if (TestFlag(AppwizFlags::Msi))
            {
                const auto uOld = MsiSetInternalUI(INSTALLUILEVEL_FULL, nullptr);
                const auto r = MsiConfigureProductW(
                    GetStringView(AppwizString::ProductId).data(),
                    INSTALLSTATE_DEFAULT, INSTALLSTATE_DEFAULT);
                MsiSetInternalUI(uOld, nullptr);
                return r;
            }
            return CreateProcessForCommand(GetStringView(AppwizString::ModifyPath));
        }
    };

    LSTATUS Initialize() noexcept
    {
        m_idxCurrSource = 0;
        return NextSource();
    }

    LSTATUS Next(_Inout_ Application& App) noexcept
    {
        LSTATUS ls;
        if (m_idxCurrSource < ARRAYSIZE(Detail::UninstallSource))
        {
            DWORD cbBuf{ (DWORD)m_rsBuffer.ByteCapacity() };
            // 枚举注册表
            if ((ls = m_Reg.EnumerateKey(m_idxCurr++, m_rsBuffer.Data(), &cbBuf)) != ERROR_SUCCESS)
            {
                if (ls == ERROR_NO_MORE_ITEMS)
                {
                    if ((ls = NextSource()) != ERROR_SUCCESS)
                    {
                        if (m_idxCurrSource >= ARRAYSIZE(Detail::UninstallSource))
                        {
                            m_idxCurr = 0;
                            m_rsBuffer.Reserve(40);
                        }
                        return ls;
                    }
                    return Next(App);
                }
                return ls;
            }
            if ((ls = App.Key.Open(m_Reg.GetHKey(), m_rsBuffer.Data(), KEY_READ)) != ERROR_SUCCESS)
                return ls;
            App.StringBuffer.Clear();
            App.StringBuffer.Reserve(MAX_PATH);
            App.Flags = GetRegistryFlags(m_idxCurrSource);
            if (App.AcquireBasicInfomation(m_bSkipWindowsInstaller,
                m_rsBuffer.Data(), cbBuf / sizeof(WCHAR), FALSE))
                return ERROR_SUCCESS;
        }
        else
        {
            if (!m_bEnumMsi)
                return ERROR_NO_MORE_ITEMS;
            // 枚举MSI
            if ((ls = MsiEnumProductsW(m_idxCurr++, m_rsBuffer.Data())) != ERROR_SUCCESS)
                return ls;
            App.AcquireBasicInfomation(FALSE/* Not used */, m_rsBuffer.Data(), 38, TRUE);
        }
        if (m_bAutoCompleteAppInfo)
            App.AcquireAllInfomation();
        return ERROR_SUCCESS;
    }

    // 是否在获取每个应用的基本信息时自动获取所有信息，
    // 若仅需基本信息，可设置为FALSE以提高性能
    EckInlineCe void SetAutoCompleteAppInfomation(BOOL bAutoComplete) noexcept
    {
        m_bAutoCompleteAppInfo = bAutoComplete;
    }
    EckInlineNdCe BOOL GetAutoCompleteAppInfomation() const noexcept
    {
        return m_bAutoCompleteAppInfo;
    }

    // 若调用方使用MsiEnumProductsW枚举MSI应用信息，
    // 则可设为TRUE以跳过这些应用，此时仅获取WindowsInstaller标志
    EckInlineCe void SetSkipWindowsInstaller(BOOL bSkip) noexcept
    {
        m_bSkipWindowsInstaller = bSkip;
    }
    EckInlineNdCe BOOL GetSkipWindowsInstaller() const noexcept
    {
        return m_bSkipWindowsInstaller;
    }

    // 是否使用MSIAPI枚举应用信息
    EckInlineCe void SetEnumerateMsi(BOOL bEnum) noexcept { m_bEnumMsi = bEnum; }
    EckInlineNdCe BOOL GetEnumerateMsi() const noexcept { return m_bEnumMsi; }
};
ECK_NAMESPACE_END