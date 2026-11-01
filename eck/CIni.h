#pragma once
#include "FileHelper.h"
#include "LocaleStringUtility.h"

ECK_NAMESPACE_BEGIN
class CIni
{
private:
    CStringW m_rsText{};
    WCHAR m_szLineBreak[3]{ L'\r',L'\n',L'\0' };
    short m_cchLineBreak = 2;

    BITBOOL m_bParseEscape : 1 = false;//          \[  \]  \=  \r  \n  \\  \;  \"
    BITBOOL m_bParseComment : 1 = false;

    int FindValuePosition(PCWSTR pszSection, PCWSTR pszKey,
        int* pposSectionEnd = nullptr, int* pposKeyBegin = nullptr) const noexcept
    {
        if (pposSectionEnd)
            *pposSectionEnd = StrNPos;
        if (pposKeyBegin)
            *pposKeyBegin = StrNPos;
        if (m_rsText.IsEmpty())
            return StrNPos;

        CStringW rsUnescaping{};
        if (m_bParseEscape)
        {
            rsUnescaping = pszSection;
            UnEscape(rsUnescaping);
            pszSection = rsUnescaping.Data();
        }

        int pos;
        CStringW rsSection{};
        rsSection.PushBack(m_szLineBreak);
        rsSection.PushBackChar(L'[');
        rsSection.PushBack(pszSection);
        rsSection.PushBackChar(L']');
        if (m_rsText.Size() < rsSection.Size() - m_cchLineBreak)
            return StrNPos;
        if (wcscmp(m_rsText.Data(), rsSection.Data() + m_cchLineBreak) == 0)
        {
            pos = 0;
            pos += (rsSection.Size() - m_cchLineBreak);
        }
        else
        {
            pos = m_rsText.Find(rsSection);
            if (pos == StrNPos)
                return StrNPos;
            pos += rsSection.Size();
        }

        if (pposSectionEnd)
            *pposSectionEnd = pos;

        if (m_bParseEscape)
        {
            rsUnescaping = pszKey;
            UnEscape(rsUnescaping);
            pszKey = rsUnescaping.Data();
        }

        CStringW rsKey{};
        rsKey.PushBack(m_szLineBreak);
        rsKey.PushBack(pszKey);
        rsKey.PushBackChar(L'=');

        pos = m_rsText.Find(rsKey, pos);
        if (pos == StrNPos)
            return StrNPos;
        if (pposKeyBegin)
            *pposKeyBegin = pos;

        pos += rsKey.Size();

        return pos;
    }

    int FindValueEndPosition(int posValueBegin) noexcept
    {
        const int posEnd = m_rsText.Find(m_szLineBreak, posValueBegin);
        if (posEnd == StrNPos)
            return m_rsText.Size();
        if (m_bParseComment)
        {
            m_rsText[posEnd] = L'\0';// 限位
            int posComment;

            if (m_bParseEscape)
            {
                posComment = posValueBegin;
                while ((posComment = m_rsText.Find(L";", posComment)) != StrNPos)
                {
                    if (m_rsText[posComment - 1] != L'\\')
                        break;
                    else
                        ++posComment;
                }
            }
            else
                posComment = m_rsText.Find(L";", posValueBegin);

            m_rsText[posEnd] = m_szLineBreak[0];// 恢复
            if (posComment != StrNPos)
                return posComment;
        }
        return posEnd;
    }

    bool WriteValue(PCWSTR pszSection, PCWSTR pszKey, PCWSTR pszValue,
        bool bMustExist = false, bool bUnEscapeChar = true) noexcept
    {
        int pos1, posSectionEnd;
        CStringW rsNew{}, rsUnescaping{};
        int pos0 = FindValuePosition(pszSection, pszKey, &posSectionEnd);
        if (pos0 == StrNPos)
        {
            if (bMustExist)
                return false;
            else if (posSectionEnd == StrNPos)// 没有节
            {
                //-----------制节
                //-----检查换行
                if (m_rsText.Size() >= m_cchLineBreak &&
                    wcsncmp(
                        m_rsText.Data() + m_rsText.Size() - m_cchLineBreak,
                        m_szLineBreak,
                        m_cchLineBreak) != 0)
                    rsNew.PushBack(m_szLineBreak, m_cchLineBreak);
                if (m_bParseEscape && bUnEscapeChar)
                {
                    rsUnescaping = pszSection;
                    UnEscape(rsUnescaping);
                    pszSection = rsUnescaping.Data();
                }
                rsNew.PushBackChar(L'[');
                rsNew.PushBack(pszSection);
                rsNew.PushBackChar(L']');
                //-----------制键
                if (m_bParseEscape && bUnEscapeChar)
                {
                    rsUnescaping = pszKey;
                    UnEscape(rsUnescaping);
                    pszKey = rsUnescaping.Data();
                }
                rsNew.PushBack(m_szLineBreak, m_cchLineBreak);
                rsNew.PushBack(pszKey);
                rsNew.PushBackChar(L'=');
                //-----------制值
                if (m_bParseEscape && bUnEscapeChar)
                {
                    rsUnescaping = pszValue;
                    UnEscape(rsUnescaping);
                    pszValue = rsUnescaping.Data();
                }
                rsNew.PushBack(pszValue);
                rsNew.PushBack(m_szLineBreak, m_cchLineBreak);
                //-----------添加
                m_rsText.PushBack(rsNew);
                return true;
            }
            else// 没有键
            {
                //-----------寻找下一个节
                WCHAR szNextSection[4];
                wcscpy(szNextSection, m_szLineBreak);
                wcscat(szNextSection, L"[");
                int posNextSection = m_rsText.Find(szNextSection, posSectionEnd);
                //-----------制键
                //-----检查换行
                if (posNextSection == StrNPos)
                {
                    if (m_rsText.Size() >= m_cchLineBreak &&
                        wcsncmp(
                            m_rsText.Data() + m_rsText.Size() - m_cchLineBreak,
                            m_szLineBreak,
                            m_cchLineBreak) != 0)
                        rsNew.PushBack(m_szLineBreak, m_cchLineBreak);
                }
                if (m_bParseEscape && bUnEscapeChar)
                {
                    rsUnescaping = pszKey;
                    UnEscape(rsUnescaping);
                    pszKey = rsUnescaping.Data();
                }
                rsNew.PushBack(pszKey);
                rsNew.PushBackChar(L'=');
                //-----------制值
                if (m_bParseEscape && bUnEscapeChar)
                {
                    rsUnescaping = pszValue;
                    UnEscape(rsUnescaping);
                    pszValue = rsUnescaping.Data();
                }
                rsNew.PushBack(pszValue);
                rsNew.PushBack(m_szLineBreak, m_cchLineBreak);
                //-----------添加
                if (posNextSection == StrNPos)
                    m_rsText.PushBack(rsNew);
                else
                    m_rsText.Insert(posNextSection, rsNew);
                return true;
            }
        }
        pos1 = FindValueEndPosition(pos0);
        m_rsText.Replace(pos0, pos1 - pos0, pszValue, (int)wcslen(pszValue));
        return true;
    }
public:
    static void Escape(CStringW& rsText, int posBegin = 0) noexcept
    {
        for (int i = posBegin; i < rsText.Size(); i++)
        {
            if (rsText[i] != L'\\')
                continue;
            if (i + 1 >= rsText.Size())
            {
                rsText.PopBack();
                break;
            }
            rsText.Erase(i, 1);
            if (rsText[i] == L'r')
                rsText[i] = L'\r';
            else if (rsText[i] == L'n')
                rsText[i] = L'\n';
            ++i;
        }
    }

    static void UnEscape(CStringW& rsText, int posBegin = 0) noexcept
    {
        for (int i = posBegin; i < rsText.Size(); i++)
        {
            if (rsText[i] == L'[')
            {
                *rsText.Insert(i, 1) = L'\\';
                ++i;
            }
            else if (rsText[i] == L']')
            {
                *rsText.Insert(i, 1) = L'\\';
                ++i;
            }
            else if (rsText[i] == L'=')
            {
                *rsText.Insert(i, 1) = L'\\';
                ++i;
            }
            else if (rsText[i] == L'\r')
            {
                *rsText.Insert(i, 1) = L'\\';
                rsText[i + 1] = L'r';
                ++i;
            }
            else if (rsText[i] == L'\n')
            {
                *rsText.Insert(i, 1) = L'\\';
                rsText[i + 1] = L'n';
                ++i;
            }
            else if (rsText[i] == L'\\')
            {
                *rsText.Insert(i, 1) = L'\\';
                ++i;
            }
            else if (rsText[i] == L';')
            {
                *rsText.Insert(i, 1) = L'\\';
                ++i;
            }
            else if (rsText[i] == L',')
            {
                *rsText.Insert(i, 1) = L'\\';
                ++i;
            }
        }
    }

    CIni() = default;
    CIni(PCWSTR pszFileName, UINT uCp = CP_UTF8) noexcept
    {
        Load(pszFileName, uCp);
    }

    int ReadInt(PCWSTR pszSection, PCWSTR pszKey,
        int Default = 0) noexcept
    {
        const int pos = FindValuePosition(pszSection, pszKey);
        if (pos == StrNPos)
            return Default;
        return _wtoi(m_rsText.Data() + pos);
    }

    double ReadDouble(PCWSTR pszSection, PCWSTR pszKey,
        double Default = 0.0) noexcept
    {
        const int pos = FindValuePosition(pszSection, pszKey);
        if (pos == StrNPos)
            return Default;
        return _wtof(m_rsText.Data() + pos);
    }

    bool ReadBool(PCWSTR pszSection, PCWSTR pszKey,
        bool Default = false) noexcept
    {
        const int pos0 = FindValuePosition(pszSection, pszKey);
        if (pos0 == StrNPos)
            return Default;
        const int pos1 = FindValueEndPosition(pos0);
        const int cch = pos1 - pos0;
        if (cch == 4 && wcsnicmp(m_rsText.Data() + pos0, L"True", 4) == 0)
            return true;
        else if (cch == 5 && wcsnicmp(m_rsText.Data() + pos0, L"False", 5) == 0)
            return false;
        else
            return !!_wtoi(m_rsText.Data() + pos0);
    }

    CStringW ReadString(PCWSTR pszSection, PCWSTR pszKey,
        PCWSTR Default = L"") noexcept
    {
        int pos0 = FindValuePosition(pszSection, pszKey);
        if (pos0 == StrNPos)
            return Default;
        int pos1 = FindValueEndPosition(pos0);
        auto rs = CStringW(m_rsText.Data() + pos0, pos1 - pos0);
        if (m_bParseEscape)
            Escape(rs);
        return rs;
    }

    bool ReadIntArray(std::vector<int>& v, PCWSTR pszSection, PCWSTR pszKey) noexcept
    {
        const int pos0 = FindValuePosition(pszSection, pszKey);
        if (pos0 == StrNPos)
            return false;
        const int pos1 = FindValueEndPosition(pos0);
        const int cch = pos1 - pos0;
        SplitString(m_rsText.Data() + pos0, L",", 0, cch, 1,
            [&v](PCWSTR pszStart, int cchSub) noexcept
            {
                v.emplace_back(_wtoi(pszStart));
                return false;
            });
        return true;
    }

    bool ReadDoubleArray(std::vector<double>& v, PCWSTR pszSection, PCWSTR pszKey) noexcept
    {
        const int pos0 = FindValuePosition(pszSection, pszKey);
        if (pos0 == StrNPos)
            return false;
        const int pos1 = FindValueEndPosition(pos0);
        const int cch = pos1 - pos0;
        SplitString(m_rsText.Data() + pos0, L",", 0, cch, 1,
            [&v](PCWSTR pszStart, int cchSub) noexcept
            {
                v.emplace_back(_wtof(pszStart));
                return false;
            });
        return true;
    }

    template<class TBool = bool>
    bool ReadBoolArray(std::vector<TBool>& v, PCWSTR pszSection, PCWSTR pszKey) noexcept
    {
        const int pos0 = FindValuePosition(pszSection, pszKey);
        if (pos0 == StrNPos)
            return false;
        const int pos1 = FindValueEndPosition(pos0);
        const int cch = pos1 - pos0;
        SplitString(m_rsText.Data() + pos0, L",", 0, cch, 1,
            [&v](PCWSTR pszStart, int cchSub)
            {
                if (cchSub == 4 && wcsnicmp(pszStart, L"True", 4) == 0)
                    v.emplace_back(1);
                else if (cchSub == 5 && wcsnicmp(pszStart, L"False", 5) == 0)
                    v.emplace_back(0);
                else
                    v.emplace_back(!!_wtoi(pszStart));
                return false;
            });
        return true;
    }

    bool ReadStringArray(std::vector<CStringW>& v, PCWSTR pszSection, PCWSTR pszKey) noexcept
    {
        const int pos0 = FindValuePosition(pszSection, pszKey);
        if (pos0 == StrNPos)
            return false;
        const int pos1 = FindValueEndPosition(pos0);
        if (pos1 <= pos0)
            return true;
        if (!m_bParseEscape)
        {
            SplitString(m_rsText.Data() + pos0, L",", 0, pos1 - pos0, 1,
                [&v](PCWSTR pszStart, int cchSub)
                {
                    v.emplace_back(pszStart, cchSub);
                    return false;
                });
            return true;
        }

        int posElem = pos0;
        for (int pos = pos0; pos < pos1;)
        {
            if (m_rsText[pos] == L'\\' && pos + 1 < pos1)
                pos += 2;
            else if (m_rsText[pos] == L',')
            {
                Escape(v.emplace_back(m_rsText.Data() + posElem, pos - posElem));
                posElem = ++pos;
            }
            else
                ++pos;
        }
        Escape(v.emplace_back(m_rsText.Data() + posElem, pos1 - posElem));
        return true;
    }

    EckInline bool WriteInt(PCWSTR pszSection, PCWSTR pszKey,
        int Value, bool bMustExist = false) noexcept
    {
        WCHAR szValue[Int32StringBufferSize];
        _itow(Value, szValue, 10);
        return WriteValue(pszSection, pszKey, szValue, bMustExist, false);
    }

    EckInline bool WriteDouble(PCWSTR pszSection, PCWSTR pszKey,
        double Value, bool bMustExist = false) noexcept
    {
        auto rsValue = ToString(Value, 15);
        return WriteValue(pszSection, pszKey, rsValue.Data(), bMustExist, false);
    }

    EckInline bool WriteBool(PCWSTR pszSection, PCWSTR pszKey,
        bool Value, bool bMustExist = false) noexcept
    {
        return WriteValue(pszSection, pszKey, Value ? L"True" : L"False", bMustExist, false);
    }

    EckInline bool WriteString(PCWSTR pszSection, PCWSTR pszKey,
        PCWSTR pszValue, bool bMustExist = false) noexcept
    {
        return WriteValue(pszSection, pszKey, pszValue, bMustExist);
    }

    bool WriteIntArray(PCWSTR pszSection, PCWSTR pszKey,
        const int* Value, int cValue, bool bMustExist = false) noexcept
    {
        CStringW rs{};
        EckCounter(cValue, i)
            rs.PushBackFormat(L"%d,", Value[i]);
        if (cValue)
            rs.PopBack();
        return WriteValue(pszSection, pszKey, rs.Data(), bMustExist, false);
    }

    bool WriteDoubleArray(PCWSTR pszSection, PCWSTR pszKey,
        const double* Value, int cValue, bool bMustExist = false) noexcept
    {
        CStringW rs{};
        EckCounter(cValue, i)
            rs.PushBackFormat(L"%f,", Value[i]);
        if (cValue)
            rs.PopBack();
        return WriteValue(pszSection, pszKey, rs.Data(), bMustExist, false);
    }

    template<class TBool = bool>
    bool WriteIntArray(PCWSTR pszSection, PCWSTR pszKey,
        const TBool* Value, int cValue, bool bMustExist = false) noexcept
    {
        CStringW rs{};
        rs.Reserve(6 * cValue);
        EckCounter(cValue, i)
        {
            if (Value[i])
                rs.PushBack(L"True,");
            else
                rs.PushBack(L"False,");
        }
        if (cValue)
            rs.PopBack();
        return WriteValue(pszSection, pszKey, rs.Data(), bMustExist, false);
    }

    bool WriteStringArray(PCWSTR pszSection, PCWSTR pszKey,
        const CStringW* Value, int cValue, bool bMustExist = false) noexcept
    {
        CStringW rs{};
        EckCounter(cValue, i)
        {
            if (m_bParseEscape)
            {
                const int posBegin = rs.Size();
                rs.PushBack(Value[i]);
                if (!Value[i].IsEmpty())
                    UnEscape(rs, posBegin);
            }
            else
                rs.PushBack(Value[i]);
            rs.PushBackChar(L',');
        }
        if (cValue)
            rs.PopBack();
        return WriteValue(pszSection, pszKey, rs.Data(), bMustExist, false);
    }

    bool DeleteSection(PCWSTR pszSection) noexcept
    {
        int pos0;
        CStringW rsUnescaping{};
        if (m_bParseEscape)
        {
            rsUnescaping = pszSection;
            UnEscape(rsUnescaping);
            pszSection = rsUnescaping.Data();
        }

        CStringW rsSection{};
        rsSection.PushBack(m_szLineBreak);
        rsSection.PushBackChar(L'[');
        rsSection.PushBack(pszSection);
        rsSection.PushBackChar(L']');
        if (m_rsText.Size() < rsSection.Size() - m_cchLineBreak)
            return false;
        if (wcscmp(m_rsText.Data(), rsSection.Data() + m_cchLineBreak) == 0)
            pos0 = 0;
        else
        {
            pos0 = m_rsText.Find(rsSection);
            if (pos0 == StrNPos)
                return false;
        }

        //-----------寻找下一个节
        WCHAR szNextSection[4];
        wcscpy(szNextSection, m_szLineBreak);
        wcscat(szNextSection, L"[");
        const int pos1 = m_rsText.Find(szNextSection, pos0 + rsSection.Size());
        if (pos1 == StrNPos)
            m_rsText.PopBack(m_rsText.Size() - pos0);
        else
            m_rsText.Erase(pos0, pos1 - pos0);
        return true;
    }

    bool DeleteKey(PCWSTR pszSection, PCWSTR pszKey) noexcept
    {
        int posKeyBegin;
        const int pos0 = FindValuePosition(pszSection, pszKey, nullptr, &posKeyBegin);
        if (pos0 == StrNPos)
            return false;
        const int pos1 = FindValueEndPosition(pos0);
        m_rsText.Erase(posKeyBegin, pos1 - posKeyBegin);
        return true;
    }

    bool Load(PCWSTR pszFileName, UINT uCp = CP_UTF8) noexcept
    {
        auto rbFile = ReadInFile(pszFileName);
        if (rbFile.IsEmpty())
            return false;

        if (uCp == CP_UTF16LE)
            m_rsText.Assign((PCWCH)rbFile.Data(), int(rbFile.Size() / sizeof(WCHAR)));
        else if (uCp == CP_UTF16BE)
        {
            m_rsText.Assign((PCWCH)rbFile.Data(), int(rbFile.Size() / sizeof(WCHAR)));
            LcsUtf16ReverseByteOrder(m_rsText.Data(), m_rsText.Size());
        }
        else
            m_rsText = EcdMultiByteToWide((PCSTR)rbFile.Data(), (int)rbFile.Size(), uCp);
        return true;
    }

    bool Save(PCWSTR pszFileName, UINT uCp = CP_UTF8, bool bAddBom = true) noexcept
    {
        CByteBuffer rb{};
        if (uCp == CP_UTF16LE)
        {
            const HANDLE hFile = CreateFileW(pszFileName, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hFile == INVALID_HANDLE_VALUE)
                return false;

            DWORD cbWritten;
            if (bAddBom)
                WriteFile(hFile, BOM_UTF16LE, ARRAYSIZE(BOM_UTF16LE), &cbWritten, nullptr);
            const bool b = WriteFile(hFile, m_rsText.Data(), (DWORD)m_rsText.ByteSize(), &cbWritten, nullptr);
            CloseHandle(hFile);
            return b;
        }
        else if (uCp == CP_UTF16BE)
        {
            const int cchBuf = LCMapStringEx(LOCALE_NAME_INVARIANT, LCMAP_BYTEREV,
                m_rsText.Data(), m_rsText.Size(), nullptr, 0, nullptr, nullptr, 0);
            if (cchBuf == 0)
                return false;
            rb.ReSize((cchBuf + 1) * sizeof(WCHAR));
            int cbSkip = (bAddBom ? 2 : 0);
            memcpy(rb.Data(), BOM_UTF16BE, cbSkip);
            LCMapStringEx(LOCALE_NAME_INVARIANT, LCMAP_BYTEREV,
                m_rsText.Data(), m_rsText.Size(), (PWSTR)(rb.Data() + cbSkip), cchBuf, nullptr, nullptr, 0);
        }
        else
        {
            const int cchBuf = WideCharToMultiByte(uCp, 0, m_rsText.Data(), m_rsText.Size(),
                nullptr, 0, nullptr, nullptr);
            if (cchBuf == 0)
                return false;
            rb.ReSize(cchBuf + 3);
            int cbSkip = ((bAddBom && uCp == CP_UTF8) ? 3 : 0);
            memcpy(rb.Data(), BOM_UTF8, cbSkip);
            WideCharToMultiByte(uCp, 0, m_rsText.Data(), m_rsText.Size(),
                (PSTR)rb.Data() + cbSkip, cchBuf, nullptr, nullptr);
        }
        return WriteToFile(pszFileName, rb);
    }

    EckInline const CStringW& GetText() const noexcept { return m_rsText; }

    void SetParseEscape(bool bParseEscapeChar) noexcept { m_bParseEscape = bParseEscapeChar; }

    void SetParseComment(bool bParseComment) noexcept { m_bParseComment = bParseComment; }

    bool SetLineBreak(PCWSTR pszLineBreak) noexcept
    {
        const int cch = (int)wcslen(pszLineBreak);
        if (cch > 2)
            return false;
        m_cchLineBreak = (short)cch;
        wcscpy(m_szLineBreak, pszLineBreak);
        return true;
    }

    EckInline void SetText(PCWSTR pszText) noexcept { m_rsText = pszText; }
    EckInline void SetText(const CStringW& rsText) noexcept { m_rsText = rsText; }
    EckInline void SetText(CStringW&& rsText) noexcept { m_rsText = std::move(rsText); }

    EckInline void Clear() noexcept { m_rsText.Clear(); }
};
ECK_NAMESPACE_END