#pragma once
#include "FileHelper.h"

#include <DbgHelp.h>

ECK_NAMESPACE_BEGIN
// 微软符号服务器
constexpr inline std::wstring_view SymbolServerMicrosoft{ L"http://msdl.microsoft.com/download/symbols"sv };

struct CV_INFO_PDB70
{
    DWORD CvSignature;
    GUID Signature;
    DWORD Age;
    char PdbFileName[1];
};

struct PDB_INFO
{
    CStringA rsPdbFile{};
    CV_INFO_PDB70 Cv{};
};

inline W32ERR DshQueryPePdb(
    Eck_Out_buffer_ PDB_INFO& PdbInfo,
    _In_reads_bytes_(cbPe) PCVOID pPe,
    size_t cbPe) noexcept
{
    const auto pDosHeader = (IMAGE_DOS_HEADER*)pPe;
    const auto pNtHeaders = (IMAGE_NT_HEADERS*)((BYTE*)pPe + pDosHeader->e_lfanew);
    const IMAGE_DATA_DIRECTORY* pDbgDataDir;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE ||
        pNtHeaders->Signature != IMAGE_NT_SIGNATURE)
        return ERROR_BAD_EXE_FORMAT;

    switch (pNtHeaders->FileHeader.Machine)
    {
    case IMAGE_FILE_MACHINE_I386:
    {
        const auto pOptHdr = (IMAGE_OPTIONAL_HEADER32*)(&pNtHeaders->OptionalHeader);
        pDbgDataDir = &pOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
    }
    break;
    case IMAGE_FILE_MACHINE_AMD64:
    {
        const auto pOptHdr = (IMAGE_OPTIONAL_HEADER64*)(&pNtHeaders->OptionalHeader);
        pDbgDataDir = &pOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
    }
    break;
    default:
        return ERROR_NOT_SUPPORTED;
    }
    if (pDbgDataDir->VirtualAddress == 0 || pDbgDataDir->Size == 0)
        return ERROR_NOT_FOUND;

    for (DWORD i = 0; i < pDbgDataDir->Size / sizeof(IMAGE_DEBUG_DIRECTORY); ++i)
    {
        const auto pDbgData = (IMAGE_DEBUG_DIRECTORY*)
            ((BYTE*)pPe + pDbgDataDir->VirtualAddress);
        if (pDbgData[i].Type == IMAGE_DEBUG_TYPE_CODEVIEW)
        {
            const auto pCvInfo = (CV_INFO_PDB70*)(
                (BYTE*)pPe + pDbgData[i].PointerToRawData);
            if (pCvInfo->CvSignature == 'SDSR')
            {
                PdbInfo.rsPdbFile.Assign(pCvInfo->PdbFileName);
                PdbInfo.Cv = *pCvInfo;
                return ERROR_SUCCESS;
            }
        }
    }
    return ERROR_NOT_FOUND;
}


inline W32ERR DshQueryPePdb(Eck_Out_buffer_ PDB_INFO& PdbInfo, _In_ PCWSTR pszPeFile) noexcept
{
    NTSTATUS nts;
    CFile File{};
    nts = File.Create(
        pszPeFile,
        FILE_OPEN,
        FILE_GENERIC_READ,
        FILE_SHARE_READ,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
    if (!NT_SUCCESS(nts))
        return WIN32_FROM_NTSTATUS(nts);

    CFileSectionMap Map{};
    Map.Create(File.Get(), PAGE_READONLY, SEC_COMMIT, &nts);
    if (!NT_SUCCESS(nts))
        return WIN32_FROM_NTSTATUS(nts);
    size_t cbView;
    const auto pViewBase = Map.Map(PAGE_READONLY, &cbView, &nts);
    if (!NT_SUCCESS(nts))
        return WIN32_FROM_NTSTATUS(nts);

    return DshQueryPePdb(PdbInfo, pViewBase, cbView);
}

inline W32ERR DshMakeSymbolUrl(
    Eck_Out_buffer_ CStringW& rsSymbolUrl,
    const PDB_INFO& PdbInfo,
    std::wstring_view svSymbolSrv = SymbolServerMicrosoft) noexcept
{
    const auto rsPdbW = EcdMultiByteToWide(PdbInfo.rsPdbFile.Data(),
        PdbInfo.rsPdbFile.Size(), CP_ACP);
    rsSymbolUrl.Clear();
    if (PdbInfo.rsPdbFile.IsEmpty())
        return ERROR_INVALID_PARAMETER;
    rsSymbolUrl.PushBack(svSymbolSrv);
    if (rsSymbolUrl.Back() != L'/')
        rsSymbolUrl.PushBackChar(L'/');
    rsSymbolUrl.PushBack(rsPdbW);
    rsSymbolUrl.PushBackChar(L'/');
    GuidToString(PdbInfo.Cv.Signature, rsSymbolUrl.PushBack(32));
    rsSymbolUrl.PushBackFormat(L"%u/", PdbInfo.Cv.Age);
    rsSymbolUrl.PushBack(rsPdbW);
    return ERROR_SUCCESS;
}

inline W32ERR DshInit(
    _Out_ HANDLE& hProcess,
    UINT uOptions = SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_ANYTHING,
    PCWSTR pszUserSearchPath = nullptr,
    BOOL bInvadeProcess = FALSE) noexcept
{
    NTSTATUS nts;
    hProcess = NaOpenProcess(SYNCHRONIZE, FALSE, NtCurrentProcessId32(), &nts);
    if (!NT_SUCCESS(nts))
        return WIN32_FROM_NTSTATUS(nts);
    if (!SymInitializeW(hProcess, nullptr, bInvadeProcess))
        return NaGetLastError();
    SymSetOptions(uOptions);
    return ERROR_SUCCESS;
}

inline W32ERR DshUnInit(HANDLE hProcess) noexcept
{
    if (!hProcess || hProcess == INVALID_HANDLE_VALUE)
        return ERROR_INVALID_PARAMETER;
    W32ERR u;
    if (SymCleanup(hProcess))
        u = ERROR_SUCCESS;
    else
        u = NaGetLastError();
    NtClose(hProcess);
    return u;
}

inline W32ERR DshLoadPdb(HANDLE hProcess, _In_ PCWSTR pszPdbFile,
    DWORD64 DllBase = 0x00401000) noexcept
{
    NTSTATUS nts;
    const auto cbPdb = (UINT)FileGetSizeByPath(pszPdbFile, &nts);
    if (!NT_SUCCESS(nts))
        return WIN32_FROM_NTSTATUS(nts);
    if (!SymLoadModuleExW(hProcess, nullptr, pszPdbFile,
        nullptr, DllBase, cbPdb, nullptr, 0))
        return NaGetLastError();
    return ERROR_SUCCESS;
}
ECK_NAMESPACE_END