#pragma once
#include "CString.h"

#include <set>
#include <unordered_set>

ECK_NAMESPACE_BEGIN
enum class IniResult
{
    Ok,                     // 成功
    SecRBracketNotFound,    // 节右括号"]"未找到
    SecEmptyName,           // 节名为空
    SecIllegalChar,         // 方括号周围存在非法字符
    SecContainerNotMatch,   // 容器未闭合
    SecDuplicate,           // 节名重复
    KvSepNotFound,          // 键值分隔符"="未找到
    KvEmptyKey,             // 键名为空
    EscapeAtEnd,            // 转义字符"\"后面没有字符

    Max,
};

enum : UINT
{
    INIE_EF_NONE = 0,
    INIE_EF_HAS_COMMENTS = 1u << 0, // 条目后面存在注释
    INIE_EF_IS_CONTAINER = 1u << 1, // 容器（仅用于节）
};

enum : UINT
{
    INIE_IF_NONE = 0,                   // 无特殊选项
    INIE_IF_IGNORE_COMMENTS = 1u << 0,  // 待解析内容中没有注释
    INIE_IF_DISABLE_EXT = 1u << 1,      // 禁用扩展语法
    INIE_IF_ESCAPE = 1u << 2,           // 启用转义
    INIE_IF_KEEP_SPACE = 1u << 3,       // 保留等号周围的空白符

    INIE_IF_EOL_BEFORE_SECTION = 1u << 4,       // 节之前有换行符
    INIE_IF_END_CONTAINER_WITH_NAME = 1u << 5,  // 容器结束时标注名称
};

namespace Detail
{
    template<class TIterator>
    struct IniContext
    {
        TIterator it;

        EckInlineNdCe auto& GetIterator() const noexcept { return it; }

        EckInlineNdCe auto operator->() const noexcept { return &*it; }
        EckInlineNd auto& operator*() const noexcept { return *it; }
        EckInlineNdCe auto& Data() noexcept { return *it; }

        EckInlineNdCe BOOL IsValid() const noexcept { return it != TIterator{}; }
        EckInlineNdCe operator BOOL() const noexcept { return IsValid(); }
    };

    template<class TIterator>
    struct IniContextKeyValue : IniContext<TIterator>
    {
        EckInlineNdCe const CStringW& GetKey() const noexcept { return this->Data().rsKey; }
        EckInlineNdCe const CStringW& GetValue() const noexcept { return this->Data().rsValue; }

        EckInlineNd BOOL IsEmpty() const noexcept
        {
            return !this->IsValid() || GetValue().IsEmpty();
        }

        EckInline std::wstring_view GetString(std::wstring_view svDef = {}) const noexcept
        {
            if (IsEmpty())
                return svDef;
            else
                return GetValue().ToStringView();
        }

        template<std::integral T>
        T GetInt(T nDef = 0, BOOL bHex = FALSE) const noexcept
        {
            if (IsEmpty())
                return nDef;
            else
            {
                T r;
                TcvToInt(GetValue().Data(), GetValue().Size(), r, bHex ? 16 : 0);
                return r;
            }
        }

        template<std::floating_point T>
        EckInline T GetFloat(T dDef = 0.0) const noexcept
        {
            if (IsEmpty())
                return dDef;
            else
            {
                T i;
                TcvToFloat(GetValue().Data(), GetValue(), i);
                return i;
            }
        }

        EckInline BOOL GetBool(BOOL bDef = FALSE) const noexcept
        {
            if (IsEmpty())
                return bDef;
            else
            {
                const auto& rs = GetValue();
                return (
                    rs.CompareI(L"true"sv) == 0 ||
                    rs.Compare(L"1"sv) == 0 ||
                    rs.Compare(L"真"sv) == 0);
            }
        }

        template<CcpEnumeration T>
        EckInline T GetEnumeration(T eDef = T{}) noexcept
        {
            using TUnderlying = std::underlying_type_t<T>;
            return (T)GetInt<TUnderlying>((TUnderlying)eDef);
        }
    };

    struct IniEntry
    {
        friend class CIniExt;
    private:
        CStringW rsName{};      // 【禁止外部修改】名称，对于节，为节名，对于键值对，为键名
        mutable UINT uId{};             // 【禁止外部修改】ID
        mutable UINT uFlags{};          // INIE_EF_常量
        mutable CStringW rsComment{};   // 条目后方的注释
    public:
        IniEntry() = default;
        IniEntry(
            std::wstring_view svName,
            UINT uId,
            UINT uFlags,
            std::wstring_view svComment) noexcept :
            rsName{ svName }, uId{ uId }, uFlags{ uFlags }, rsComment{ svComment } {}
        IniEntry(
            CStringW&& svName,
            UINT uId,
            UINT uFlags,
            CStringW&& svComment) noexcept :
            rsName{ std::move(svName) }, uId{ uId },
            uFlags{ uFlags }, rsComment{ std::move(svComment) } {}

        constexpr std::strong_ordering operator<=>(const IniEntry& x) const noexcept
        {
            return uId <=> x.uId;
        }

        EckInlineNdCe auto& GetName() const noexcept { return rsName; }
        EckInlineNdCe auto GetId() const noexcept { return uId; }
    };

    struct IniValue : IniEntry
    {
        friend class CIniExt;
    private:
        mutable CStringW rsValue{};
    public:
        IniValue() = default;
        IniValue(
            std::wstring_view svName,
            UINT uId,
            UINT uFlags,
            std::wstring_view svComment,
            std::wstring_view svValue) noexcept :
            IniEntry{ svName, uId, uFlags, svComment }, rsValue{ svValue } {}
        IniValue(
            CStringW&& svName,
            UINT uId,
            UINT uFlags,
            CStringW&& svComment,
            CStringW&& svValue) noexcept :
            IniEntry{ std::move(svName), uId, uFlags, std::move(svComment) },
            rsValue{ std::move(svValue) } {}
    };
}

constexpr inline bool IsOrderedMap = false;
constexpr inline bool IsAllowMultiKeys = false;
constexpr inline bool IsCaseSensitive = true;
class CIniExt
{
public:
    using Entry = Detail::IniEntry;
    using Value = Detail::IniValue;

    struct FCompare
    {
        using is_transparent = void;

        EckInlineNd bool operator()(std::wstring_view x1, std::wstring_view x2) const noexcept
        {
            if constexpr (IsCaseSensitive)
                return x1 < x2;
            else
                return TcsCompareLength2I(x1.data(), x1.size(), x2.data(), x2.size()) < 0;
        }
    };

    struct FHash
    {
        using is_transparent = void;

        size_t operator()(std::wstring_view sv) const noexcept
        {
            return std::hash<std::wstring_view>{}(sv);
        }
    };
    struct FEqual
    {
        using is_transparent = void;

        bool operator()(std::wstring_view x1, std::wstring_view x2) const noexcept
        {
            if constexpr (IsCaseSensitive)
                return x1 == x2;
            else
                return TcsCompareLength2I(x1.data(), x1.size(), x2.data(), x2.size()) == 0;
        }
    };

    template<class T>
    using TSet = std::conditional_t<IsAllowMultiKeys,
        std::multiset<T, FCompare>,
        std::set<T, FCompare>>;
    template<class T>
    using TUnorderedSet = std::conditional_t<IsAllowMultiKeys,
        std::unordered_multiset<T, FHash, FEqual>,
        std::unordered_set<T, FHash, FEqual>>;

    using TValueSet = std::conditional_t<IsOrderedMap || !IsCaseSensitive,
        TSet<Detail::IniValue>,
        TUnorderedSet<Detail::IniValue>>;

    struct Section;

    using TSectionSet = std::conditional_t<IsOrderedMap || !IsCaseSensitive,
        TSet<Section>,
        TUnorderedSet<Section>>;

    struct Section : Detail::IniEntry
    {
        friend class CIniExt;
    private:
        mutable TValueSet Val{};    // 【禁止外部修改】所有值
        mutable TSectionSet Child{};// 【禁止外部修改】子节
    public:
        using Detail::IniEntry::IniEntry;

        EckInline void ForEachKeyValue(auto&& Fn) noexcept
        {
            for (auto& e : Val)
                Fn(e.second);
        }
        EckInline void ForEachChild(auto&& Fn) noexcept
        {
            for (auto& e : Child)
                Fn(e.second);
        }
    };

    struct Comment : Detail::IniEntry
    {
    public:
        using Detail::IniEntry::IniEntry;
    };

    using TSectionIterator = typename TSectionSet::iterator;
    using TSectionConstIterator = typename TSectionSet::const_iterator;

    using TKeyValueIterator = typename TValueSet::iterator;
    using TKeyValueConstIterator = typename TValueSet::const_iterator;

    using SectionContext = Detail::IniContext<TSectionIterator>;
    using SectionConstContext = Detail::IniContext<TSectionConstIterator>;
    using KeyValueContext = Detail::IniContextKeyValue<TKeyValueIterator>;
    using CtxKvCst = Detail::IniContextKeyValue<TKeyValueConstIterator>;
private:
    TSectionSet m_Root{};
    std::vector<Comment> m_vComment{};// uFlags和rsComment无效
    EolType m_eEolType{ EolType::CRLF };
    UINT m_uId{};

    constexpr static BOOL EscapeChar(_Inout_ WCHAR& ch) noexcept
    {
        switch (ch)
        {
        case 'n':	ch = '\n';	break;
        case 'r':	ch = '\r';	break;
        case 't':	ch = '\t';	break;
        case '0':	ch = '\0';	break;
        case '\\':	break;
        case ';':	break;
        case '[':	break;
        case ']':	break;
        case '=':	break;
        default:	return FALSE;
        }
        return TRUE;
    }

    EckInlineNdCe static BOOL IsBreakLineChar(WCHAR ch) noexcept
    {
        return ch == '\n' || ch == '\r';
    }

    EckInlineNdCe static BOOL IsSpaceChar(WCHAR ch) noexcept
    {
        return IsBreakLineChar(ch) || ch == '\t' || ch == ' ' || ch == '\0';
    }

    EckInlineNdCe static BOOL IsCommentChar(WCHAR ch) noexcept
    {
        return ch == ';';
    }

    EckInlineNdCe static BOOL IsSpaceOrCommentChar(WCHAR ch) noexcept
    {
        return IsSpaceChar(ch) || ch == ';';
    }

    EckInlineNdCe static BOOL IsBreakLineOrCommentChar(WCHAR ch) noexcept
    {
        return ch == '\n' || ch == '\r' || ch == ';';
    }
    EckInlineNdCe static BOOL IsBomChar(WCHAR ch) noexcept
    {
        return ch == 0xFEFF;
    }
    EckInlineNdCe static BOOL IsSpaceCharHeader(WCHAR ch) noexcept
    {
        return IsSpaceChar(ch) || IsBomChar(ch);
    }

    // 调用前：psz指向[
    // 调用后：psz指向]的下一个位置
    static IniResult UnescapeSectionName(CStringW& rs, PCWCH& psz, size_t cch) noexcept
    {
        const auto pszEnd = psz + cch;
        for (; psz != pszEnd; ++psz)
        {
            const auto ch = *psz;
            if (ch == '\\')// 转义字符
            {
                if (psz + 1 >= pszEnd)
                    return IniResult::EscapeAtEnd;
                auto ch2 = *++psz;
                if (EscapeChar(ch2))
                    rs.PushBackChar(ch2);
                else
                    rs.PushBackChar(ch).PushBackChar(ch2);
            }
            else if (ch == ']')// 节末尾
            {
                if (psz + 1 < pszEnd && !IsBreakLineOrCommentChar(*(psz + 1)))
                    return IniResult::SecIllegalChar;
                ++psz;
                return IniResult::Ok;
            }
            else// 常规字符
                rs.PushBackChar(ch);
        }
        return IniResult::SecRBracketNotFound;
    }

    // 调用前：psz指向[
    // 调用后：psz指向]的下一个位置
    static IniResult ScanSectionName(CStringW& rs, PCWCH& psz, size_t cch) noexcept
    {
        const auto pR = TcsCharLength(psz, cch, ']');
        if (pR)
        {
            if (pR + 1 < psz + cch && !IsBreakLineOrCommentChar(*(pR + 1)))
                return IniResult::SecIllegalChar;
            rs.Assign(psz, int(pR - psz));
            psz = pR + 1;
            return IniResult::Ok;
        }
        else
            return IniResult::SecRBracketNotFound;
    }

    // 调用前：psz指向键的第一个字符
    // 调用后：psz指向值最后一个字符的下一个位置
    static IniResult UnescapeKeyValue(CStringW& rsKey, CStringW& rsVal,
        PCWCH& psz, size_t cch, BOOL bKeepSpace) noexcept
    {
        const auto pOrg = psz;
        const auto pszEnd = psz + cch;
        // 键
        for (; psz != pszEnd; ++psz)
        {
            const auto ch = *psz;
            if (ch == '\\')// 转义字符
            {
                if (psz + 1 >= pszEnd)
                    return IniResult::EscapeAtEnd;
                auto ch2 = *++psz;
                if (EscapeChar(ch2))
                    rsKey.PushBackChar(ch2);
                else
                    rsKey.PushBackChar(ch).PushBackChar(ch2);
            }
            else if (ch == '=')// 键末尾
                break;
            else// 常规字符
                rsKey.PushBackChar(ch);
        }
        if (*psz != '=')
            return IniResult::KvSepNotFound;
        ++psz;
        if (!bKeepSpace)
        {
            rsKey.TrimRight();
            psz = TrimStringLeft(psz, int(pszEnd - psz));
        }
        // 值
        for (; psz != pszEnd; ++psz)
        {
            const auto ch = *psz;
            if (ch == '\\')// 转义字符
            {
                auto ch2 = *++psz;
                if (EscapeChar(ch2))
                    rsVal.PushBackChar(ch2);
                else
                    rsVal.PushBackChar(ch).PushBackChar(ch2);
            }
            else if (ch == ';' || ch == '\n' || ch == '\r')// 值末尾
                break;
            else// 常规字符
                rsVal.PushBackChar(ch);
        }
        return IniResult::Ok;
    }

    // 调用前：psz指向键的第一个字符
    // 调用后：psz指向值最后一个字符的下一个位置
    static IniResult ScanKeyValue(CStringW& rsKey, CStringW& rsVal,
        PCWCH& psz, size_t cch, BOOL bKeepSpace) noexcept
    {
        const auto pOrg = psz;
        auto pR = TcsCharLength(psz, cch, '=');
        if (pR)
        {
            rsKey.Assign(psz, int(pR - psz));
            psz = pR + 1;
            cch = cch - (psz - pOrg);
        }
        else
            return IniResult::SecRBracketNotFound;
        if (!bKeepSpace)
        {
            rsKey.TrimRight();
            if (IsBreakLineOrCommentChar(*psz))
                return IniResult::Ok;
            const auto pL = TrimStringLeft(psz, (int)cch);
            cch -= (pL - psz);
            psz = pL;
        }

        constexpr static WCHAR ValEnd[]{ ';','\n','\r' };
        pR = TcsCharFirstOf(psz, cch, EckArgArray(ValEnd));
        if (!pR)// 若找不到值结束符，则包括到结尾
            pR = psz + cch;
        rsVal.Assign(psz, int(pR - psz));
        psz = pR;
        return IniResult::Ok;
    }

    static void ScanComments(CStringW& rs, PCWCH& psz, size_t cch) noexcept
    {
        constexpr static WCHAR ValEnd[]{ '\n','\r' };
        auto pR = TcsCharFirstOf(psz, cch, EckArgArray(ValEnd));
        if (!pR)
            pR = psz + cch;
        rs.PushBack(psz, int(pR - psz));
        psz = pR;
    }

    // 转义svOrg，并尾插到rsOut中
    static void EscapeString(
        std::wstring_view svOrg,
        Eck_Append_buffer_ CStringW& rsOut) noexcept
    {
        for (const auto ch : svOrg)
        {
            switch (ch)
            {
            case '\n':	rsOut.PushBackChar('\\').PushBackChar('n');	break;
            case '\r':	rsOut.PushBackChar('\\').PushBackChar('r');	break;
            case '\t':	rsOut.PushBackChar('\\').PushBackChar('t');	break;
            case '\0':	rsOut.PushBackChar('\\').PushBackChar('0');	break;
            case '\\':	rsOut.PushBackChar('\\').PushBackChar('\\');	break;
            case ';':	rsOut.PushBackChar('\\').PushBackChar(';');	break;
            case '[':	rsOut.PushBackChar('\\').PushBackChar('[');	break;
            case ']':	rsOut.PushBackChar('\\').PushBackChar(']');	break;
            case '=':	rsOut.PushBackChar('\\').PushBackChar('=');	break;
            default:	rsOut.PushBackChar(ch);	break;
            }
        }
    }

    static auto EmplacePair(auto Ret) noexcept
    {
        if constexpr (IsAllowMultiKeys)
            return std::make_pair(Ret, true);
        else
            return Ret;
    }

    static auto Iterator(auto Ret) noexcept
    {
        if constexpr (IsAllowMultiKeys)
            return Ret;
        else
            return Ret.first;
    }

    void ForEachEntry(Section& Section, auto&& Fn) noexcept
    {
        for (auto& Val : Section.Val)
            Fn(Val.second);
        for (auto& Child : Section.Child)
        {
            Fn(Child.second);
            ForEachEntry(Child.second, Fn);
        }
    }

    void ForEachEntry(auto&& Fn) noexcept
    {
        for (auto& Section : m_Root)
        {
            Fn(Section.second);
            ForEachEntry(Section.second, Fn);
        }
        for (auto& Comment : m_vComment)
            Fn(Comment);
    }

    void OnCreateEntry(UINT uId, std::wstring_view svName) noexcept
    {
        ForEachEntry([this, uId](Entry& e)
            {
                if (e.uId >= uId)
                    e.uId += 1;
            });
    }

    void OnDeleteEntry(UINT uId) noexcept
    {
        ForEachEntry([this, uId](Entry& e)
            {
                if (e.uId > uId)
                    e.uId -= 1;
            });
        --m_uId;
    }

    SectionContext InternalSetSection(
        TSectionSet& Set,
        const SectionContext& Section,
        std::wstring_view svName) noexcept
    {
        auto New{ std::move(*Section) };
        Set.erase(Section.it);
        New.rsName.Assign(svName);
        return { Iterator(Set.emplace(std::move(New))) };
    }
    SectionContext InternalCreateSection(
        TSectionSet& Set,
        UINT uId,
        std::wstring_view svName,
        UINT uFlags) noexcept
    {
        if (uId == UINT_MAX)
            uId = m_uId++;
        OnCreateEntry(uId, svName);
        return { Iterator(Set.emplace(Section{ svName, uId, uFlags, {} })) };
    }
    void InternalDeleteSection(
        TSectionSet& Set,
        const SectionContext& Section) noexcept
    {
        OnDeleteEntry(Section.it->uId);
        Set.erase(Section.it);
    }


    KeyValueContext InternalSetKeyName(
        TValueSet& Set,
        const KeyValueContext& Kv,
        std::wstring_view svName) noexcept
    {
        auto New{ std::move(*Kv) };
        Set.erase(Kv.it);
        New.rsName.Assign(svName);
        return { Iterator(Set.emplace(std::move(New))) };
    }
    KeyValueContext InternalCreateKeyValue(
        TValueSet& Set,
        UINT uId,
        std::wstring_view svName,
        std::wstring_view svValue,
        UINT uFlags) noexcept
    {
        if (uId != UINT_MAX)
            uId = m_uId++;
        OnCreateEntry(uId, svName);
        return { Iterator(Set.emplace(Value{ svName, uId, uFlags, {}, svValue })) };
    }
    void InternalDeleteKeyValue(TValueSet& Set, TKeyValueIterator it) noexcept
    {
        OnDeleteEntry(it->uId);
        Set.erase(it);
    }

    void PushBackEol(Eck_Append_buffer_ CStringW& rs) const noexcept
    {
        switch (m_eEolType)
        {
        case EolType::CRLF:
            rs.PushBackChar('\r').PushBackChar('\n');
            break;
        case EolType::LF:
            rs.PushBackChar('\n');
            break;
        case EolType::CR:
            rs.PushBackChar('\r');
            break;
        }
    }
public:
    IniResult Load(PCWCH pszIni, size_t cchIni = -1, UINT uFlags = INIE_IF_NONE) noexcept
    {
        Clear();
        if (cchIni < 0)
            cchIni = TcsLength(pszIni);
        const auto pszEnd = pszIni + cchIni;
        enum class State
        {
            Section,
            Key,
        };

        struct ITEM
        {
            TSectionIterator it{};
            BOOL bContainer{};// 该节是否为容器
        };

        const BOOL bKeepSpace = (uFlags & INIE_IF_KEEP_SPACE);
        const BOOL bEscape = (uFlags & INIE_IF_ESCAPE);

        State eState{ State::Section };
        CStringW rsComment{};
        std::vector<ITEM> stSec{};// 栈顶为当前节
        stSec.reserve(16);
        stSec.emplace_back();// Dummy
        Entry* pLastEntry{};
        while (pszIni < pszEnd)
        {
            const auto ch = *pszIni++;
            if (IsBreakLineChar(ch))// 到行尾时合并项目后面的注释
            {
                if (pLastEntry && !rsComment.IsEmpty())
                {
                    pLastEntry->rsComment = std::move(rsComment);
                    rsComment.Clear();
                    pLastEntry->uFlags |= INIE_EF_HAS_COMMENTS;
                }
                pLastEntry = nullptr;
                continue;
            }
            else if (IsSpaceChar(ch))
                continue;
            if (ch == ';')// 注释
            {
                if (!rsComment.IsEmpty())
                    PushBackEol(rsComment);
                ScanComments(rsComment, pszIni, pszEnd - pszIni);
                continue;
            }
            else// 到非注释字符时，合并不在项目后面的注释
            {
                if (!rsComment.IsEmpty())
                {
                    m_vComment.emplace_back(Comment{ std::move(rsComment), m_uId++ });
                    rsComment.Clear();
                }
            }

            switch (eState)
            {
            case State::Section:
            {
                if (ch == '[')
                {
                    if (pszIni >= pszEnd)
                        return IniResult::SecRBracketNotFound;
                    BOOL bContainer;
                    if (*pszIni == '>')
                    {
                        ++pszIni;
                        bContainer = TRUE;
                    }
                    else
                        bContainer = FALSE;
                    CStringW rsName{};
                    const auto r = (bEscape ?
                        UnescapeSectionName(rsName, pszIni, pszEnd - pszIni) :
                        ScanSectionName(rsName, pszIni, pszEnd - pszIni));
                    if (r != IniResult::Ok)
                        return r;
                    if (rsName.IsEmpty())
                        return IniResult::SecEmptyName;
                    if (rsName.Front() == '<')// 闭合容器节
                    {
                        if (rsName.Size() > 1)// 如果不止"<"一个字符，则校验当前栈顶
                        {
                            if (rsName.SubStringView(1, rsName.Size() - 1) !=
                                stSec.back().it->rsName.ToStringView())
                                return IniResult::SecContainerNotMatch;
                        }
                        stSec.pop_back();
                        continue;
                    }
                    if (!stSec.back().bContainer)
                        stSec.pop_back();
                    EckAssert(!rsName.IsLocal());
                    auto& Set = (stSec.empty() ? m_Root : stSec.back().it->Child);
                    const auto sv = rsName.ToStringView();
                    const UINT uFlags = (bContainer ? INIE_EF_IS_CONTAINER : INIE_EF_NONE) |
                        (rsComment.IsEmpty() ? INIE_EF_NONE : INIE_EF_HAS_COMMENTS);
                    const auto Ret = EmplacePair(Set.emplace(
                        Section{
                            std::move(rsName),
                            m_uId++,
                            uFlags,
                            {}
                        }));
                    if (bContainer && !Ret.second)
                        return IniResult::SecDuplicate;
                    stSec.emplace_back(Ret.first, bContainer);
                    pLastEntry = &Ret.first->second;
                    eState = State::Key;
                }
                else if (!IsSpaceCharHeader(ch))
                    return IniResult::SecIllegalChar;
            }
            break;

            case State::Key:
            {
                --pszIni;
                if (ch == '[')
                {
                    eState = State::Section;
                    continue;
                }
                CStringW rsKey{}, rsVal{};
                const auto r = (bEscape ?
                    UnescapeKeyValue(rsKey, rsVal, pszIni, pszEnd - pszIni, bKeepSpace) :
                    ScanKeyValue(rsKey, rsVal, pszIni, pszEnd - pszIni, bKeepSpace));
                if (r != IniResult::Ok)
                    return r;
                if (rsKey.IsEmpty())
                    return IniResult::KvEmptyKey;
                if (rsKey.IsLocal())
                    rsKey.Reserve(24);
                EckAssert(!rsKey.IsLocal());
                auto& Set = stSec.back().it->Val;
                const auto sv = rsKey.ToStringView();
                const auto it = Iterator(Set.emplace(Value{
                    std::move(rsKey),
                    m_uId++,
                    (rsComment.IsEmpty() ? INIE_EF_NONE : INIE_EF_HAS_COMMENTS),
                    {},
                    std::move(rsVal) }));
                pLastEntry = &*it;
            }
            break;
            }
        }
        if (!rsComment.IsEmpty())
        {
            m_vComment.emplace_back(Comment{ std::move(rsComment), m_uId++ });
            rsComment.Clear();
        }
        return IniResult::Ok;
    }

    IniResult Save(CStringW& rsOut, UINT uFlags = INIE_IF_NONE) const noexcept
    {
        const auto bEscape = (uFlags & INIE_IF_ESCAPE);
        struct STACK
        {
            TSectionConstIterator it{};
            BOOL bExtended{};
        };
        std::vector<STACK> sSec{};
        sSec.reserve(m_uId);
        std::vector<TKeyValueConstIterator> vKv{};
        vKv.reserve(m_uId);
        struct STACK2
        {
            std::wstring_view svName{};
            size_t cChild{};
        };
        std::vector<STACK2> sContainer{};
        CStringW rsSpace{};

        auto Fn_PushBackSection = [&](const TSectionSet& Set)
            {
                if (Set.empty())
                    return;
                const BOOL bInsertingChild = sSec.empty() ? FALSE : sSec.back().bExtended;
                size_t i{ bInsertingChild ? sSec.size() - 1 : sSec.size() };
                sSec.resize(sSec.size() + Set.size());
                if (bInsertingChild)
                {
                    sSec.back() = sSec[i];
                    sSec[i].bExtended = FALSE;
                }
                const auto itSecBegin = sSec.begin() + i;
                for (auto it = Set.begin(); it != Set.end(); ++it, ++i)
                    sSec[i].it = it;
                const auto itSecEnd = sSec.end();
                std::sort(itSecBegin, itSecEnd, [](const STACK& x1, const STACK& x2)
                    {
                        return x1.it->uId > x2.it->uId;// 倒排序，便于后续弹出
                    });
            };

        Fn_PushBackSection(m_Root);
        while (!sSec.empty())
        {
            auto& e = *sSec.back().it;
            if ((e.uFlags & INIE_EF_IS_CONTAINER) && !sSec.back().bExtended)
            {
                sContainer.emplace_back(sSec.back().it->rsName.ToStringView(), e.Child.size() + 1);
                rsSpace.PushBackChar(' ').PushBackChar(' ');
                sSec.back().bExtended = TRUE;
                Fn_PushBackSection(e.Child);
            }
            else
            {
                rsOut.PushBack(rsSpace.Data(), rsSpace.Size() - 2);
                rsOut.PushBackChar('[');
                if (e.uFlags & INIE_EF_IS_CONTAINER)
                    rsOut.PushBackChar('>');
                if (bEscape)
                    EscapeString(e.rsName.ToStringView(), rsOut);
                else
                    rsOut.PushBack(e.rsName);
                rsOut.PushBackChar(']');
                PushBackEol(rsOut);
                for (auto it = e.Val.begin(); it != e.Val.end(); ++it)
                    vKv.emplace_back(it);
                std::sort(vKv.begin(), vKv.end(), [](const TKeyValueConstIterator& x1, const TKeyValueConstIterator& x2)
                    {
                        return x1->uId < x2->uId;
                    });
                for (const auto& f : vKv)
                {
                    rsOut.PushBack(rsSpace.Data(), rsSpace.Size() - 2);
                    if (bEscape)
                    {
                        EscapeString(f->rsName.ToStringView(), rsOut);
                        rsOut.PushBackChar('=');
                        EscapeString(f->rsValue.ToStringView(), rsOut);
                    }
                    else
                        rsOut
                        .PushBack(f->rsName)
                        .PushBackChar('=')
                        .PushBack(f->rsValue);
                    PushBackEol(rsOut);
                }
                vKv.clear();
                if (uFlags & INIE_IF_EOL_BEFORE_SECTION)
                    PushBackEol(rsOut);
                sSec.pop_back();

                while (!sContainer.empty() && --sContainer.back().cChild == 0)
                {
                    rsOut.PushBack(rsSpace.Data(), rsSpace.Size() - 2);
                    rsOut.PushBackChar('[').PushBackChar('<');
                    if (uFlags & INIE_IF_END_CONTAINER_WITH_NAME)
                        if (bEscape)
                            EscapeString(sContainer.back().svName, rsOut);
                        else
                            rsOut.PushBack(sContainer.back().svName);
                    rsOut.PushBackChar(']');
                    PushBackEol(rsOut);
                    sContainer.pop_back();
                    rsSpace.PopBack().PopBack();
                }
            }
        }
        return IniResult::Ok;
    }

    EckInlineNd SectionContext GetSection(std::wstring_view svName) noexcept
    {
        const auto it = m_Root.find(svName);
        return { it == m_Root.end() ? TSectionIterator{} : it };
    }
    EckInlineNd SectionConstContext GetSection(std::wstring_view svName) const noexcept
    {
        const auto it = m_Root.find(svName);
        return { it == m_Root.end() ? TSectionConstIterator{} : it };
    }

    EckInlineNd SectionContext GetSection(
        const SectionContext& Section,
        std::wstring_view svName) const noexcept
    {
        const auto it = Section->Child.find(svName);
        return { it == Section->Child.end() ? TSectionIterator{} : it };
    }
    EckInlineNd SectionConstContext GetSection(
        const SectionConstContext& Section,
        std::wstring_view svName) const noexcept
    {
        const auto it = Section->Child.find(svName);
        return { it == Section->Child.end() ? TSectionConstIterator{} : it };
    }

    EckInline SectionContext SetSection(
        const SectionContext& Section,
        std::wstring_view svName) noexcept
    {
        return InternalSetSection(m_Root, Section, svName);
    }
    EckInline SectionContext SetSection(
        const SectionContext& Section,
        const SectionContext& SecParent,
        std::wstring_view svName) noexcept
    {
        return InternalSetSection(SecParent->Child, Section, svName);
    }

    EckInline SectionContext CreateSection(
        std::wstring_view svName,
        UINT uFlags = INIE_EF_NONE,
        UINT uId = UINT_MAX) noexcept
    {
        return InternalCreateSection(m_Root, uId, svName, uFlags);
    }
    EckInline SectionContext CreateSection(
        const SectionContext& SecParent,
        std::wstring_view svName,
        UINT uFlags = INIE_EF_NONE,
        UINT uId = UINT_MAX) noexcept
    {
        return InternalCreateSection(SecParent->Child, uId, svName, uFlags);
    }

    EckInline void DeleteSection(const SectionContext& Section) noexcept { InternalDeleteSection(m_Root, Section); }
    EckInline void DeleteSection(
        const SectionContext& Section,
        const SectionContext& SecParent) noexcept
    {
        InternalDeleteSection(SecParent->Child, Section);
    }

    EckInlineNd KeyValueContext GetKeyValue(
        const SectionContext& Section,
        std::wstring_view svName) noexcept
    {
        if (!Section)
            return {};
        const auto it = Section->Val.find(svName);
        return { it == Section->Val.end() ? TKeyValueIterator{} : it };
    }
    EckInlineNd CtxKvCst GetKeyValue(
        const SectionConstContext& Section,
        std::wstring_view svName) const noexcept
    {
        if (!Section)
            return {};
        const auto it = Section->Val.find(svName);
        return { it == Section->Val.end() ? TKeyValueConstIterator{} : it };
    }

    EckInlineNd KeyValueContext GetKeyValue(
        std::wstring_view svSection,
        std::wstring_view svName) noexcept
    {
        return GetKeyValue(GetSection(svSection), svName);
    }
    EckInlineNd CtxKvCst GetKeyValue(
        std::wstring_view svSection,
        std::wstring_view svName) const noexcept
    {
        return GetKeyValue(GetSection(svSection), svName);
    }

    EckInline KeyValueContext SetKeyName(
        const KeyValueContext& Kv,
        const SectionContext& Section,
        std::wstring_view svName) noexcept
    {
        return InternalSetKeyName(Section->Val, Kv, svName);
    }
    EckInline KeyValueContext SetKeyName(
        std::wstring_view svName,
        std::wstring_view svSection,
        std::wstring_view svNewName) noexcept
    {
        auto Section = GetSection(svSection);
        if (!Section)
            Section = CreateSection(svSection);
        const auto Kv = GetKeyValue(Section, svName);
        if (!Kv)
            return CreateKeyValue(Section, svNewName, {});
        return SetKeyName(Kv, Section, svNewName);
    }

    EckInline KeyValueContext CreateKeyValue(
        const SectionContext& Section,
        std::wstring_view svName,
        std::wstring_view svValue,
        UINT uFlags = INIE_EF_NONE,
        UINT uId = UINT_MAX) noexcept
    {
        return InternalCreateKeyValue(Section->Val, uId, svName, svValue, uFlags);
    }

    EckInline void SetKeyValue(
        const SectionContext& Section,
        std::wstring_view svName,
        std::wstring_view svValue,
        UINT uFlags = INIE_EF_NONE) noexcept
    {
        auto Kv = GetKeyValue(Section, svName);
        if (!Kv)
            Kv = CreateKeyValue(Section, svName, svValue, uFlags);
        Kv->rsValue.Assign(svValue);
    }

    EckInline void DeleteKeyValue(
        const SectionContext& Section,
        std::wstring_view svName) noexcept
    {
        const auto it = Section->Val.find(svName);
        if (it != Section->Val.end())
            InternalDeleteKeyValue(Section->Val, it);
    }
private:
    void IntForEachSectionInOrder(std::vector<TSectionIterator>& vSec, TSectionSet& Set) noexcept
    {
        for (auto it = m_Root.begin(); it != m_Root.end(); ++it)
        {
            vSec[it->uId] = it;
            IntForEachSectionInOrder(vSec, it->Child);
        }
    }
public:
    void ForEachSectionInOrder(auto&& Fn, const SectionContext& Section = {}) noexcept
    {
        std::vector<TSectionIterator> vSec{ m_uId };
        IntForEachSectionInOrder(vSec, Section ? Section->Child : m_Root);
        const auto itEnd = std::remove(vSec.begin(), vSec.end(), TSectionIterator{});
        for (auto it = vSec.begin(); it != itEnd; ++it)
            Fn(*it);
    }

    void ForEachValueInOrder(auto&& Fn, const SectionContext& Section) noexcept
    {
        auto& Val = Section->Val;
        std::vector<TKeyValueIterator> vVal{};
        vVal.reserve(Val.size());
        for (auto it = Val.begin(); it != Val.end(); ++it)
            vVal.emplace_back(it);
        std::sort(vVal.begin(), vVal.end(), [](const TKeyValueIterator& x1, const TKeyValueIterator& x2)
            {
                return x1->second.uId < x2->second.uId;
            });
        for (auto e : vVal)
            Fn(e);
    }

    EckInlineNd BOOL IsEmpty() const noexcept { return m_Root.empty(); }

    void Clear() noexcept
    {
        m_Root.clear();
        m_vComment.clear();
        m_uId = 0;
    }
};
ECK_NAMESPACE_END