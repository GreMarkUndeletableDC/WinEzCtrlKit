#pragma once
#include "CString.h"
#include "CByteBuffer.h"

#pragma push_macro("free")
#pragma push_macro("malloc")
#pragma push_macro("realloc")
#undef free
#undef malloc
#undef realloc
#include "../ThirdPartyLib/YyJson/yyjson.h"

#define ECK_JSON_NAMESPACE_BEGIN	namespace Json {
#define ECK_JSON_NAMESPACE_END		}

ECK_NAMESPACE_BEGIN
ECK_JSON_NAMESPACE_BEGIN
using YyReadFlag = yyjson_read_flag;
using YyReadError = yyjson_read_err;
using YyAllocator = yyjson_alc;
using YyDocument = yyjson_doc;
using YyMutableDocument = yyjson_mut_doc;
using YyValue = yyjson_val;
using YyMutableValue = yyjson_mut_val;
using YyType = yyjson_type;
using YySubType = yyjson_subtype;
using YyWriteFlag = yyjson_write_flag;
using YyWriteError = yyjson_write_err;
using YyArrayIterator = yyjson_arr_iter;
using YyMutableArrayIterator = yyjson_mut_arr_iter;
using YyObjectIterator = yyjson_obj_iter;
using YyMutableObjectIterator = yyjson_mut_obj_iter;
using YyPointerError = yyjson_ptr_err;
using YyPointerContext = yyjson_ptr_ctx;

class CValue;
class CDocument;
class CMutableValue;
class CMutableDocument;
struct ArrayProxy;
struct ObjectProxy;
struct MutableArrayProxy;
struct MutableObjectProxy;
namespace Detail { struct JsonProxy; }

struct Array_T {};

EckInline BOOL YyLocateStringPosition(
    _In_reads_(cchText) PCSTR pszText,
    size_t cchText,
    size_t ocbPos,
    _Out_ size_t& nLine,
    _Out_ size_t& nCol,
    _Out_ size_t& nChar) noexcept
{
    return yyjson_locate_pos(pszText, cchText, ocbPos, &nLine, &nCol, &nChar);
}

namespace Detail
{
    template<class TThis, class T>
    EckInline auto JsonValueAtType(TThis& This, const T& x) noexcept
    {
        using T1 = std::remove_cvref_t<T>;
        if constexpr (std::is_integral_v<T1>)
            return This.ArrAt(x);
        else if constexpr (
            std::is_convertible_v<T1, PCCH> ||
            std::is_convertible_v<T1, PCBYTE> ||
            std::is_convertible_v<T1, const char8_t*>)
        {
            return This.AtValue((PCSTR)x);
        }
        else if constexpr (
            IsSameTemplate<CStringT, T1>::V &&
            sizeof(typename T1::TChar) == 1)
            return This.AtValue((PCCH)x.Data(), (size_t)x.Size());
        else if constexpr (
            IsSameTemplate<std::basic_string, T1>::V &&
            sizeof(typename T1::value_type) == 1)
            return This.AtValue((PCCH)x.data(), x.size());
        else if constexpr (
            IsSameTemplate<std::basic_string_view, T1>::V &&
            sizeof(typename T1::value_type) == 1)
            return This.AtValue((PCCH)x.data(), x.size());
        else
            static_assert(!sizeof(T), "Unsupported type.");
    }
    template<class TThis, class T, size_t N>
        requires (sizeof(std::remove_cvref_t<T>) == 1)
    EckInline auto JsonValueAtType(TThis& This, const T(&x)[N]) noexcept
    {
        return This.AtValue((PCCH)&x, N - 1);
    }

    template<class T>
    EckInline bool EqualIterator(const T& x, const T& y) noexcept
    {
        if (x.m_Iter.cur)
            if (y.m_Iter.cur)
                return x.m_Iter.cur == y.m_Iter.cur;
            else
                return !x.HasNext();
        else
            if (y.m_Iter.cur)
                return !y.HasNext();
            else
                return true;
    }

    // NOTE 函数返回后，pszU8已被释放
    inline CStringW WriteW(
        _In_reads_opt_(cchU8) PSTR pszU8,
        size_t cchU8,
        _In_opt_ const YyAllocator* pAlc) noexcept
    {
        if (!pszU8)
            return {};
        CStringW rs{ EcdMultiByteToWide(pszU8, (int)cchU8, CP_UTF8) };
        if (pAlc && pAlc->free)
            pAlc->free(pAlc->ctx, pszU8);
        else
            free(pszU8);
        return rs;
    }

    class CValueBase
    {
    protected:
        void* m_pVal{};
    public:
        constexpr CValueBase(void* p) noexcept : m_pVal{ p } {}
        EckInlineNd YyType GetType() const noexcept { return unsafe_yyjson_get_type(m_pVal); }
        EckInlineNd YySubType GetSubType() const noexcept { return unsafe_yyjson_get_subtype(m_pVal); }
        EckInlineNd uint8_t GetTag() const noexcept { return unsafe_yyjson_get_tag(m_pVal); }
        EckInlineNd PCSTR GetTypeDescription() const noexcept { return yyjson_get_type_desc((yyjson_val*)m_pVal); }

        EckInline bool EqualString(_In_z_ PCSTR pszStr) const noexcept { return yyjson_equals_str((yyjson_val*)m_pVal, pszStr); }
        EckInline bool EqualString(
            _In_reads_(cchStr) PCSTR pszStr,
            size_t cchStr) const noexcept
        {
            return yyjson_equals_strn((yyjson_val*)m_pVal, pszStr, cchStr);
        }

        EckInlineNd bool IsNull() const noexcept { return unsafe_yyjson_is_null(m_pVal); }
        EckInlineNd bool IsTrue() const noexcept { return unsafe_yyjson_is_true(m_pVal); }
        EckInlineNd bool IsFalse() const noexcept { return unsafe_yyjson_is_false(m_pVal); }
        EckInlineNd bool IsBool() const noexcept { return unsafe_yyjson_is_bool(m_pVal); }
        EckInlineNd bool IsUInt64() const noexcept { return unsafe_yyjson_is_uint(m_pVal); }
        EckInlineNd bool IsInt64() const noexcept { return unsafe_yyjson_is_sint(m_pVal); }
        EckInlineNd bool IsInt() const noexcept { return unsafe_yyjson_is_int(m_pVal); }
        EckInlineNd bool IsReal() const noexcept { return unsafe_yyjson_is_real(m_pVal); }
        EckInlineNd bool IsNumber() const noexcept { return unsafe_yyjson_is_num(m_pVal); }
        EckInlineNd bool IsString() const noexcept { return unsafe_yyjson_is_str(m_pVal); }
        EckInlineNd bool IsArray() const noexcept { return unsafe_yyjson_is_arr(m_pVal); }
        EckInlineNd bool IsObject() const noexcept { return unsafe_yyjson_is_obj(m_pVal); }
        EckInlineNd bool IsContainer() const noexcept { return unsafe_yyjson_is_ctn(m_pVal); }
        EckInlineNd bool IsRaw() const noexcept { return unsafe_yyjson_is_raw(m_pVal); }

        EckInlineNd PCSTR GetRaw() const noexcept { return yyjson_get_raw((yyjson_val*)m_pVal); }
        EckInlineNd bool GetBool() const noexcept { return yyjson_get_bool((yyjson_val*)m_pVal); }
        EckInlineNd uint64_t GetUInt64() const noexcept { return yyjson_get_uint((yyjson_val*)m_pVal); }
        EckInlineNd int64_t GetInt64() const noexcept { return yyjson_get_sint((yyjson_val*)m_pVal); }
        EckInlineNd int GetInt() const noexcept { return yyjson_get_int((yyjson_val*)m_pVal); }
        EckInlineNd double GetReal() const noexcept { return yyjson_get_real((yyjson_val*)m_pVal); }
        EckInlineNd double GetNumber() const noexcept { return yyjson_get_num((yyjson_val*)m_pVal); }
        EckInlineNd PCSTR GetString() const noexcept { return yyjson_get_str((yyjson_val*)m_pVal); }
        EckInlineNd size_t GetLength() const noexcept { return yyjson_get_len((yyjson_val*)m_pVal); }
        EckInlineNd std::string_view GetStringView() const noexcept { return { GetString(), GetLength() }; }
        EckInlineNd CStringW GetStringW() const noexcept { return EcdMultiByteToWide(GetString(), (int)GetLength(), CP_UTF8); }
    };
}

class CValue : public Detail::CValueBase
{
public:
    constexpr CValue(YyValue* pVal) noexcept : CValueBase{ (void*)pVal } {}
    EckInlineNdCe auto GetPointer() const noexcept { return (YyValue*)m_pVal; }
    EckInlineNdCe BOOL IsValid() const noexcept { return !!m_pVal; }

    EckInline bool SetRaw(
        _In_reads_or_z_(cchRaw) PCSTR pszRaw,
        size_t cchRaw = MaxSizeT) const noexcept
    {
        return yyjson_set_raw(GetPointer(), pszRaw, cchRaw);
    }
    EckInline bool SetNull() const noexcept { return yyjson_set_null(GetPointer()); }
    EckInline bool SetBool(bool bVal) const noexcept { return yyjson_set_bool(GetPointer(), bVal); }
    EckInline bool SetUInt64(uint64_t uVal) const noexcept { return yyjson_set_uint(GetPointer(), uVal); }
    EckInline bool SetInt64(int64_t iVal) const noexcept { return yyjson_set_sint(GetPointer(), iVal); }
    EckInline bool SetInt(int iVal) const noexcept { return yyjson_set_int(GetPointer(), iVal); }
    EckInline bool SetReal(double dVal) const noexcept { return yyjson_set_real(GetPointer(), dVal); }
    EckInline bool SetString(_In_z_ PCSTR pszVal) const noexcept { return yyjson_set_str(GetPointer(), pszVal); }
    EckInline bool SetString(
        _In_reads_(cchVal) PCSTR pszVal,
        size_t cchVal) const noexcept
    {
        return yyjson_set_strn(GetPointer(), pszVal, cchVal);
    }

    EckInlineNd size_t ArrSize() const noexcept { return yyjson_arr_size(GetPointer()); }
    EckInlineNd CValue ArrAt(size_t idx) const noexcept { return CValue(yyjson_arr_get(GetPointer(), idx)); }
    EckInlineNd CValue ArrFront() const noexcept { return CValue(yyjson_arr_get_first(GetPointer())); }
    EckInlineNd CValue ArrBack() const noexcept { return CValue(yyjson_arr_get_last(GetPointer())); }
    EckInlineNd size_t ObjSize() const noexcept { return yyjson_obj_size(GetPointer()); }
    EckInlineNd CValue ObjAt(_In_z_ PCSTR pszKey) const noexcept { return CValue(yyjson_obj_get(GetPointer(), pszKey)); }
    EckInlineNd CValue ObjAt(
        _In_reads_(cchKey) PCSTR pszKey,
        size_t cchKey) const noexcept
    {
        return CValue(yyjson_obj_getn(GetPointer(), pszKey, cchKey));
    }
    EckInlineNd CValue ObjGetVal(CValue Key) const noexcept { return CValue(yyjson_obj_iter_get_val(Key.GetPointer())); }

    EckInlineNd PSTR Write(
        _Out_opt_ size_t* pcchOut,
        YyWriteFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyWriteError* pErr = nullptr) noexcept
    {
        return yyjson_val_write_opts(GetPointer(), uFlags, pAlc, pcchOut, pErr);
    }
    EckInline BOOL Write(
        _In_z_ PCSTR pszFile,
        YyWriteFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyWriteError* pErr = nullptr) noexcept
    {
        return yyjson_val_write_file(pszFile, GetPointer(), uFlags, pAlc, pErr);
    }
    CStringW WriteW(
        YyWriteFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyWriteError* pErr = nullptr) noexcept
    {
        size_t cchU8{};
        PSTR pszU8 = Write(&cchU8, uFlags, pAlc, pErr);
        return Detail::WriteW(pszU8, cchU8, pAlc);
    }

    EckInlineNd CValue AtValue(
        _In_reads_or_z_(cchPtr) PCSTR pszPtr,
        size_t cchPtr = MaxSizeT,
        _Out_opt_ YyPointerError* pErr = nullptr) const noexcept
    {
        return CValue(yyjson_ptr_getx(GetPointer(), pszPtr,
            cchPtr == MaxSizeT ? strlen(pszPtr) : cchPtr, pErr));
    }

    EckInlineNd CValue operator[](const auto& x) const noexcept
    {
        return Detail::JsonValueAtType(*this, x);
    }

    EckInlineNd ArrayProxy AsArray() const noexcept;
    EckInlineNd ObjectProxy AsObject() const noexcept;
};

class CDocument
{
private:
    YyDocument* m_pDoc{};
public:
    ECK_DISABLE_COPY_DEF_CONS(CDocument);
    CDocument(
        _In_reads_or_z_(cchJson) PCSTR pszJson,
        size_t cchJson = MaxSizeT,
        YyReadFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyReadError* pErr = nullptr) noexcept
    {
        m_pDoc = yyjson_read_opts((PSTR)pszJson, cchJson == MaxSizeT ? strlen(pszJson) : cchJson,
            uFlags, pAlc, pErr);
    }

    template<class TAllocator>
    CDocument(
        const CByteBufferT<TAllocator>& rb,
        YyReadFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyReadError* pErr = nullptr) noexcept
        : CDocument((PCSTR)rb.Data(), rb.Size(), uFlags, pAlc, pErr)
    {}

    template<class TTraits, class TAllocator>
    CDocument(
        const CStringT<CHAR, TTraits, TAllocator>& rs,
        YyReadFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyReadError* pErr = nullptr) noexcept
        : CDocument(rs.Data(), rs.Size(), uFlags, pAlc, pErr)
    {}

    template<class TTraits, class TAllocator>
    CDocument(
        const std::basic_string<CHAR, TTraits, TAllocator>& s,
        YyReadFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyReadError* pErr = nullptr) noexcept
        : CDocument(s.data(), s.size(), uFlags, pAlc, pErr)
    {}

    template<class TTraits>
    CDocument(
        const std::basic_string_view<CHAR, TTraits>& sv,
        YyReadFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyReadError* pErr = nullptr) noexcept
        : CDocument(sv.data(), sv.size(), uFlags, pAlc, pErr)
    {}

    CDocument(
        _In_reads_or_z_(cchJson) const char8_t* pszJson,
        size_t cchJson = MaxSizeT,
        YyReadFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyReadError* pErr = nullptr) noexcept
        : CDocument((PCSTR)pszJson, cchJson, uFlags, pAlc, pErr)
    {}

    constexpr CDocument(YyDocument* pDoc) noexcept : m_pDoc{ pDoc } {}
    constexpr CDocument(CDocument&& x) noexcept : m_pDoc{ x.Detach() } {}
    constexpr CDocument& operator=(CDocument&& x) noexcept
    {
        std::swap(m_pDoc, x.m_pDoc);
        return *this;
    }
    ~CDocument() { Free(); }

    EckInlineNdCe BOOL IsValid() const noexcept { return !!m_pDoc; }
    EckInlineNdCe YyDocument* GetPointer() const noexcept { return m_pDoc; }
    EckInline void Free() noexcept
    {
        if (m_pDoc)
        {
            yyjson_doc_free(m_pDoc);
            m_pDoc = nullptr;
        }
    }
    EckInlineNdCe YyDocument* Detach() noexcept
    {
        YyDocument* pDoc = m_pDoc;
        m_pDoc = nullptr;
        return pDoc;
    }
    EckInlineCe YyDocument* Attach(YyDocument* pDoc) noexcept
    {
        YyDocument* pOldDoc = m_pDoc;
        m_pDoc = pDoc;
        return pOldDoc;
    }

    EckInlineNd CValue GetRoot() const noexcept { return CValue(yyjson_doc_get_root(m_pDoc)); }
    EckInlineNd size_t GetReadSize() const noexcept { return yyjson_doc_get_read_size(m_pDoc); }
    EckInlineNd size_t GetValueCount() const noexcept { return yyjson_doc_get_val_count(m_pDoc); }
    EckInlineNd CValue AtValue(
        _In_reads_or_z_(cchPtr) PCSTR pszPtr,
        size_t cchPtr = MaxSizeT,
        _Out_opt_ YyPointerError* pErr = nullptr) const noexcept
    {
        return CValue(yyjson_doc_ptr_getx(m_pDoc, pszPtr,
            cchPtr == MaxSizeT ? strlen(pszPtr) : cchPtr, pErr));
    }
    EckInlineNd PSTR Write(
        _Out_opt_ size_t* pcchOut,
        YyWriteFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyWriteError* pErr = nullptr) const noexcept
    {
        return yyjson_write_opts(m_pDoc, uFlags, pAlc, pcchOut, pErr);
    }
    EckInline BOOL Write(
        _In_z_ PCSTR pszFile,
        YyWriteFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyWriteError* pErr = nullptr) const noexcept
    {
        return yyjson_write_file(pszFile, m_pDoc, uFlags, pAlc, pErr);
    }
    CStringW WriteW(
        YyWriteFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyWriteError* pErr = nullptr) const noexcept
    {
        size_t cchU8{};
        PSTR pszU8 = Write(&cchU8, uFlags, pAlc, pErr);
        return Detail::WriteW(pszU8, cchU8, pAlc);
    }
    EckInlineNd CMutableDocument Clone(const YyAllocator* pAlc = nullptr) const noexcept;

    EckInlineNd CValue operator[](const auto& x) const noexcept
    {
        return Detail::JsonValueAtType(*this, x);
    }
};

struct ArrayIterator
{
    YyArrayIterator m_Iter{};

    ArrayIterator() = default;
    constexpr ArrayIterator(YyArrayIterator Iter) noexcept : m_Iter{ Iter } {}
    ArrayIterator(CValue Val) noexcept : m_Iter{ yyjson_arr_iter_with(Val.GetPointer()) } {}

    EckInline void FromValue(CValue Val) noexcept
    {
        m_Iter = yyjson_arr_iter_with(Val.GetPointer());
    }
    EckInlineNd BOOL HasNext() const noexcept
    {
        return yyjson_arr_iter_has_next((YyArrayIterator*)&m_Iter);
    }
    EckInlineNd CValue Next() noexcept
    {
        return CValue(yyjson_arr_iter_next(&m_Iter));
    }
    EckInlineCe CValue GetCurrent() const noexcept { return CValue(m_Iter.cur); }
    EckInline ArrayIterator& operator++() noexcept { Next(); return *this; }
    EckInlineCe CValue operator*() const noexcept { return GetCurrent(); }
};
EckInlineNd bool operator==(const ArrayIterator& x, const ArrayIterator& y) noexcept
{
    return Detail::EqualIterator<ArrayIterator>(x, y);
}

struct ObjectIterator
{
    YyObjectIterator m_Iter{};

    ObjectIterator() = default;
    constexpr ObjectIterator(YyObjectIterator Iter) noexcept : m_Iter{ Iter } {}
    ObjectIterator(CValue Val) noexcept : m_Iter{ yyjson_obj_iter_with(Val.GetPointer()) } {}

    EckInline void FromValue(CValue Val) noexcept
    {
        m_Iter = yyjson_obj_iter_with(Val.GetPointer());
    }
    EckInlineNd BOOL HasNext() const noexcept
    {
        return yyjson_obj_iter_has_next((YyObjectIterator*)&m_Iter);
    }
    EckInlineNd CValue Next() noexcept
    {
        return CValue(yyjson_obj_iter_next(&m_Iter));
    }
    EckInlineNd CValue Get(_In_z_ PCSTR pszKey) noexcept
    {
        return yyjson_obj_iter_get(&m_Iter, pszKey);
    }
    EckInlineNd CValue Get(_In_reads_(cchKey) PCSTR pszKey, size_t cchKey) noexcept
    {
        return yyjson_obj_iter_getn(&m_Iter, pszKey, cchKey);
    }
    EckInlineCe CValue GetCurrent() const noexcept { return CValue(m_Iter.cur); }
    EckInline ObjectIterator& operator++() noexcept { Next(); return *this; }
    EckInlineCe CValue operator*() const noexcept { return GetCurrent(); }
};
EckInlineNd bool operator==(const ObjectIterator& x, const ObjectIterator& y) noexcept
{
    return Detail::EqualIterator<ObjectIterator>(x, y);
}

struct ArrayProxy
{
    CValue Val;
    EckInline ArrayIterator begin() const noexcept { return ArrayIterator{ Val }; }
    EckInline ArrayIterator end() const noexcept { return ArrayIterator{}; }
};
struct ObjectProxy
{
    CValue Val;
    EckInline ObjectIterator begin() const noexcept { return ObjectIterator{ Val }; }
    EckInline ObjectIterator end() const noexcept { return ObjectIterator{}; }
};

class CMutableValue : public Detail::CValueBase
{
private:
    const CMutableDocument* m_pDoc{};
public:
    constexpr CMutableValue(YyMutableValue* pVal, const CMutableDocument* pDoc = nullptr) noexcept
        : CValueBase{ pVal }, m_pDoc{ pDoc }
    {}
    EckInlineNdCe auto GetPointer() const noexcept { return (YyMutableValue*)m_pVal; }
    EckInlineNdCe BOOL IsValid() const noexcept { return !!m_pVal; }

    EckInline void SetRaw(
        _In_reads_or_z_(cchRaw) PCSTR pszRaw,
        size_t cchRaw = MaxSizeT) const noexcept
    {
        return unsafe_yyjson_set_raw(GetPointer(), pszRaw, cchRaw);
    }
    EckInline void SetNull() const noexcept { return unsafe_yyjson_set_null(GetPointer()); }
    EckInline void SetBool(bool bVal) const noexcept { return unsafe_yyjson_set_bool(GetPointer(), bVal); }
    EckInline void SetUInt64(uint64_t uVal) const noexcept { return unsafe_yyjson_set_uint(GetPointer(), uVal); }
    EckInline void SetInt64(int64_t iVal) const noexcept { return unsafe_yyjson_set_sint(GetPointer(), iVal); }
    EckInline void SetInt(int iVal) const noexcept { return unsafe_yyjson_set_sint(GetPointer(), iVal); }
    EckInline void SetReal(double dVal) const noexcept { return unsafe_yyjson_set_real(GetPointer(), dVal); }
    EckInline void SetString(_In_z_ PCSTR pszVal) const noexcept { return unsafe_yyjson_set_str(GetPointer(), pszVal); }
    EckInline void SetString(
        _In_reads_(cchVal) PCSTR pszVal,
        size_t cchVal) const noexcept
    {
        return unsafe_yyjson_set_strn(GetPointer(), pszVal, cchVal);
    }
    EckInline void SetStringCopy(
        _In_reads_(cchVal) PCSTR pszVal,
        size_t cchVal) const noexcept
    {
        const auto pNew = unsafe_yyjson_mut_strncpy(m_pDoc->GetPointer(), pszVal, cchVal);
        return unsafe_yyjson_set_strn(GetPointer(), pNew, cchVal);
    }
    EckInline void SetArray(size_t c = 0) const noexcept { return unsafe_yyjson_set_arr(GetPointer(), c); }
    EckInline void SetObject(size_t c = 0) const noexcept { return unsafe_yyjson_set_obj(GetPointer(), c); }

    EckInlineNd size_t ArrSize() const noexcept { return yyjson_mut_arr_size(GetPointer()); }
    EckInlineNd CMutableValue ArrAt(size_t idx) const noexcept { return CMutableValue(yyjson_mut_arr_get(GetPointer(), idx), m_pDoc); }
    EckInlineNd CMutableValue ArrFront() const noexcept { return CMutableValue(yyjson_mut_arr_get_first(GetPointer()), m_pDoc); }
    EckInlineNd CMutableValue ArrBack() const noexcept { return CMutableValue(yyjson_mut_arr_get_last(GetPointer()), m_pDoc); }
    EckInline BOOL ArrInsert(size_t idx, CMutableValue Val) const noexcept
    {
        return yyjson_mut_arr_insert(GetPointer(), Val.GetPointer(), idx);
    }
    EckInline BOOL ArrPushBack(CMutableValue Val) const noexcept { return yyjson_mut_arr_append(GetPointer(), Val.GetPointer()); }
    EckInline BOOL ArrPushFront(CMutableValue Val) const noexcept { return yyjson_mut_arr_prepend(GetPointer(), Val.GetPointer()); }
    EckInline CMutableValue ArrReplace(size_t idx, CMutableValue Val) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_replace(GetPointer(), idx, Val.GetPointer()), m_pDoc);
    }
    EckInline CMutableValue ArrRemove(size_t idx) const noexcept { return CMutableValue(yyjson_mut_arr_remove(GetPointer(), idx), m_pDoc); }
    EckInline BOOL ArrRemove(size_t idx, size_t c) const noexcept
    {
        return yyjson_mut_arr_remove_range(GetPointer(), idx, c);
    }
    EckInline CMutableValue ArrPopBack() const noexcept { return CMutableValue(yyjson_mut_arr_remove_last(GetPointer()), m_pDoc); }
    EckInline CMutableValue ArrPopFront() const noexcept { return CMutableValue(yyjson_mut_arr_remove_first(GetPointer()), m_pDoc); }
    EckInline BOOL ArrClear() const noexcept { return yyjson_mut_arr_clear(GetPointer()); }
    EckInline BOOL ArrRotate(size_t idx) const noexcept { return yyjson_mut_arr_rotate(GetPointer(), idx); }

    EckInlineNd size_t ObjSize() const noexcept { return yyjson_mut_obj_size(GetPointer()); }
    EckInlineNd CMutableValue ObjAt(_In_z_ PCSTR pszKey) const noexcept { return CMutableValue(yyjson_mut_obj_get(GetPointer(), pszKey), m_pDoc); }
    EckInlineNd CMutableValue ObjAt(
        _In_reads_(cchKey) PCSTR pszKey,
        size_t cchKey) const noexcept
    {
        return CMutableValue(yyjson_mut_obj_getn(GetPointer(), pszKey, cchKey), m_pDoc);
    }
    EckInline BOOL ObjInsert(size_t idx, CMutableValue Key, CMutableValue Val) const noexcept
    {
        return yyjson_mut_obj_insert(GetPointer(), Key.GetPointer(), Val.GetPointer(), idx);
    }
    EckInline CMutableValue ObjRemove(CMutableValue Key) const noexcept
    {
        return CMutableValue(yyjson_mut_obj_remove(GetPointer(), Key.GetPointer()), m_pDoc);
    }
    EckInline CMutableValue ObjRemove(_In_z_ PCSTR pszKey) const noexcept
    {
        return CMutableValue(yyjson_mut_obj_remove_key(GetPointer(), pszKey), m_pDoc);
    }
    EckInline BOOL ObjClear() const noexcept { return yyjson_mut_obj_clear(GetPointer()); }
    EckInline BOOL ObjReplace(CMutableValue Key, CMutableValue Val) const noexcept
    {
        return yyjson_mut_obj_replace(GetPointer(), Key.GetPointer(), Val.GetPointer());
    }
    EckInline BOOL ObjRotate(size_t idx) const noexcept { return yyjson_mut_obj_rotate(GetPointer(), idx); }

    EckInlineNd PSTR Write(
        _Out_opt_ size_t* pcchOut,
        YyWriteFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyWriteError* pErr = nullptr) const noexcept
    {
        return yyjson_mut_val_write_opts(GetPointer(), uFlags, pAlc, pcchOut, pErr);
    }
    EckInline BOOL Write(
        _In_z_ PCSTR pszFile,
        YyWriteFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyWriteError* pErr = nullptr) const noexcept
    {
        return yyjson_mut_val_write_file(pszFile, GetPointer(), uFlags, pAlc, pErr);
    }
    EckInlineNd CStringW WriteW(
        YyWriteFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyWriteError* pErr = nullptr) const noexcept
    {
        size_t cchOut;
        const auto pszU8 = Write(&cchOut, uFlags, pAlc, pErr);
        return Detail::WriteW(pszU8, cchOut, pAlc);
    }

    EckInlineNd CMutableValue AtValue(
        _In_reads_or_z_(cchPtr) PCSTR pszPtr,
        size_t cchPtr = MaxSizeT,
        _Out_opt_ YyPointerContext* pCtx = nullptr,
        _Out_opt_ YyPointerError* pErr = nullptr) const noexcept
    {
        return CMutableValue(yyjson_mut_ptr_getx(GetPointer(), pszPtr,
            cchPtr == MaxSizeT ? strlen(pszPtr) : cchPtr, pCtx, pErr));
    }

    EckInlineNd CMutableValue operator[](const auto& x) const noexcept
    {
        return Detail::JsonValueAtType(*this, x);
    }
    EckInline const CMutableValue& operator=(Detail::JsonProxy x) const noexcept;
    EckInlineCe void SetParentDocument(const CMutableDocument* pDoc) { m_pDoc = pDoc; }

    EckInlineNd MutableArrayProxy AsArray() const noexcept;
    EckInlineNd MutableObjectProxy AsObject() const noexcept;
};

class CMutableDocument
{
private:
    YyMutableDocument* m_pDoc{};
public:
    ECK_DISABLE_COPY(CMutableDocument);

    CMutableDocument() : m_pDoc{ yyjson_mut_doc_new(nullptr) } {}
    explicit CMutableDocument(
        const CDocument& Doc,
        _In_opt_ const YyAllocator* pAlc = nullptr) noexcept
        : m_pDoc{ yyjson_doc_mut_copy(Doc.GetPointer(), pAlc) }
    {}
    explicit CMutableDocument(
        const CMutableDocument& Doc,
        _In_opt_ const YyAllocator* pAlc = nullptr) noexcept
        : m_pDoc{ yyjson_mut_doc_mut_copy(Doc.GetPointer(), pAlc) }
    {}
    explicit constexpr CMutableDocument(YyMutableDocument* pDoc) noexcept : m_pDoc{ pDoc } {}

    constexpr CMutableDocument(CMutableDocument&& x) noexcept : m_pDoc{ x.Detach() } {}
    CMutableDocument& operator=(CMutableDocument&& x) noexcept
    {
        std::swap(m_pDoc, x.m_pDoc);
        return *this;
    }
    ~CMutableDocument() { Free(); }

    EckInline void Create(_In_opt_ const YyAllocator* pAlc = nullptr) noexcept
    {
        Free();
        m_pDoc = yyjson_mut_doc_new(pAlc);
    }
    EckInline void Create(
        const CDocument& Doc,
        _In_opt_ const YyAllocator* pAlc = nullptr) noexcept
    {
        Free();
        m_pDoc = yyjson_doc_mut_copy(Doc.GetPointer(), pAlc);
    }
    EckInline void Create(
        const CMutableDocument& Doc,
        _In_opt_ const YyAllocator* pAlc = nullptr) noexcept
    {
        Free();
        m_pDoc = yyjson_mut_doc_mut_copy(Doc.GetPointer(), pAlc);
    }

    EckInlineNdCe BOOL IsValid() const noexcept { return !!m_pDoc; }
    EckInlineNdCe YyMutableDocument* GetPointer() const noexcept { return m_pDoc; }

    void Free() noexcept
    {
        if (m_pDoc)
        {
            yyjson_mut_doc_free(m_pDoc);
            m_pDoc = nullptr;
        }
    }
    EckInlineNdCe YyMutableDocument* Detach()
    {
        const auto pDoc = m_pDoc;
        m_pDoc = nullptr;
        return pDoc;
    }
    EckInlineCe YyMutableDocument* Attach(YyMutableDocument* pDoc)
    {
        const auto pOldDoc = m_pDoc;
        m_pDoc = pDoc;
        return pOldDoc;
    }

    EckInlineNd CMutableValue GetRoot() const noexcept { return CMutableValue(yyjson_mut_doc_get_root(m_pDoc)); }
    EckInline void SetRoot(CMutableValue Val) const noexcept { yyjson_mut_doc_set_root(m_pDoc, Val.GetPointer()); }
    EckInline BOOL SetStringPoolSize(size_t cb) const noexcept { return yyjson_mut_doc_set_str_pool_size(m_pDoc, cb); }
    EckInline BOOL SetValuePoolSize(size_t cb) const noexcept { return yyjson_mut_doc_set_val_pool_size(m_pDoc, cb); }
    EckInlineNd CMutableValue AtValue(
        _In_reads_or_z_(cchPtr) PCSTR pszPtr,
        size_t cchPtr = MaxSizeT,
        _Out_opt_ YyPointerContext* pCtx = nullptr,
        _Out_opt_ YyPointerError* pErr = nullptr) const noexcept
    {
        return CMutableValue(yyjson_mut_doc_ptr_getx(m_pDoc, pszPtr,
            cchPtr == MaxSizeT ? strlen(pszPtr) : cchPtr, pCtx, pErr), this);
    }
    EckInlineNd PSTR Write(
        _Out_opt_ size_t* pcchOut,
        YyWriteFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyWriteError* pErr = nullptr) const noexcept
    {
        return yyjson_mut_write_opts(m_pDoc, uFlags, pAlc, pcchOut, pErr);
    }
    EckInline BOOL Write(
        _In_z_ PCSTR pszFile,
        YyWriteFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyWriteError* pErr = nullptr) const noexcept
    {
        return yyjson_mut_write_file(pszFile, m_pDoc, uFlags, pAlc, pErr);
    }
    EckInlineNd CStringW WriteW(
        YyWriteFlag uFlags = 0,
        _In_opt_ const YyAllocator* pAlc = nullptr,
        _Out_opt_ YyWriteError* pErr = nullptr) const noexcept
    {
        size_t cchOut;
        const auto pszU8 = Write(&cchOut, uFlags, pAlc, pErr);
        return Detail::WriteW(pszU8, cchOut, pAlc);
    }

    EckInlineNd CMutableDocument Clone() const noexcept { return CMutableDocument(yyjson_mut_doc_mut_copy(m_pDoc, nullptr)); }
    EckInlineNd CDocument CloneImmutable() const noexcept { return CDocument(yyjson_mut_doc_imut_copy(m_pDoc, nullptr)); }

    EckInlineNd CMutableValue NewRaw(
        _In_reads_or_z_(cchRaw) PCSTR pszRaw,
        size_t cchRaw = MaxSizeT) const noexcept
    {
        return CMutableValue(yyjson_mut_rawn(m_pDoc, pszRaw, cchRaw == MaxSizeT ? strlen(pszRaw) : cchRaw), this);
    }
    EckInlineNd CMutableValue NewRawCopy(
        _In_reads_or_z_(cchRaw) PCSTR pszRaw,
        size_t cchRaw = MaxSizeT) const noexcept
    {
        return CMutableValue(yyjson_mut_rawncpy(m_pDoc, pszRaw, cchRaw == MaxSizeT ? strlen(pszRaw) : cchRaw), this);
    }
    EckInlineNd CMutableValue NewNull() const noexcept { return CMutableValue(yyjson_mut_null(m_pDoc), this); }
    EckInlineNd CMutableValue NewTrue() const noexcept { return CMutableValue(yyjson_mut_true(m_pDoc), this); }
    EckInlineNd CMutableValue NewFalse() const noexcept { return CMutableValue(yyjson_mut_false(m_pDoc), this); }
    EckInlineNd CMutableValue NewBool(bool bVal) const noexcept { return CMutableValue(yyjson_mut_bool(m_pDoc, bVal), this); }
    EckInlineNd CMutableValue NewUInt64(uint64_t uVal) const noexcept { return CMutableValue(yyjson_mut_uint(m_pDoc, uVal), this); }
    EckInlineNd CMutableValue NewInt64(int64_t iVal) const noexcept { return CMutableValue(yyjson_mut_sint(m_pDoc, iVal), this); }
    EckInlineNd CMutableValue NewInt(int iVal) const noexcept { return CMutableValue(yyjson_mut_int(m_pDoc, iVal), this); }
    EckInlineNd CMutableValue NewReal(double dVal) const noexcept { return CMutableValue(yyjson_mut_real(m_pDoc, dVal), this); }
    EckInlineNd CMutableValue NewString(
        _In_reads_or_z_(cchVal) PCSTR pszVal,
        size_t cchVal = MaxSizeT) const noexcept
    {
        return CMutableValue(yyjson_mut_strn(m_pDoc, pszVal, cchVal == MaxSizeT ? strlen(pszVal) : cchVal), this);
    }
    EckInlineNd CMutableValue NewStringCopy(
        _In_reads_or_z_(cchVal) PCSTR pszVal,
        size_t cchVal = MaxSizeT) const noexcept
    {
        return CMutableValue(yyjson_mut_strncpy(m_pDoc, pszVal, cchVal == MaxSizeT ? strlen(pszVal) : cchVal), this);
    }
    EckInlineNd CMutableValue NewStringCopy(
        _In_reads_or_z_(cchVal) PCWSTR pszVal,
        size_t cchVal = MaxSizeT) const noexcept
    {
        if (cchVal == MaxSizeT)
            cchVal = wcslen(pszVal);
        const auto u8 = EcdWideToMultiByte(pszVal, (int)cchVal, CP_UTF8);
        return NewStringCopy(u8.Data(), u8.Size());
    }
    EckInlineNd CMutableValue NewArray() const noexcept { return CMutableValue(yyjson_mut_arr(m_pDoc), this); }
    EckInlineNd CMutableValue NewArray(_In_reads_(cVals) const bool* pbVals, size_t cVals) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_with_bool(m_pDoc, pbVals, cVals), this);
    }
    EckInlineNd CMutableValue NewArray(_In_reads_(cVals) const int8_t* piVals, size_t cVals) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_with_sint8(m_pDoc, piVals, cVals), this);
    }
    EckInlineNd CMutableValue NewArray(_In_reads_(cVals) const int16_t* piVals, size_t cVals) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_with_sint16(m_pDoc, piVals, cVals), this);
    }
    EckInlineNd CMutableValue NewArray(_In_reads_(cVals) const int32_t* piVals, size_t cVals) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_with_sint32(m_pDoc, piVals, cVals), this);
    }
    EckInlineNd CMutableValue NewArray(_In_reads_(cVals) const int64_t* piVals, size_t cVals) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_with_sint64(m_pDoc, piVals, cVals), this);
    }
    EckInlineNd CMutableValue NewArray(_In_reads_(cVals) const uint8_t* puVals, size_t cVals) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_with_uint8(m_pDoc, puVals, cVals), this);
    }
    EckInlineNd CMutableValue NewArray(_In_reads_(cVals) const uint16_t* puVals, size_t cVals) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_with_uint16(m_pDoc, puVals, cVals), this);
    }
    EckInlineNd CMutableValue NewArray(_In_reads_(cVals) const uint32_t* puVals, size_t cVals) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_with_uint32(m_pDoc, puVals, cVals), this);
    }
    EckInlineNd CMutableValue NewArray(_In_reads_(cVals) const uint64_t* puVals, size_t cVals) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_with_uint64(m_pDoc, puVals, cVals), this);
    }
    EckInlineNd CMutableValue NewArray(_In_reads_(cVals) const float* pVals, size_t cVals) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_with_float(m_pDoc, pVals, cVals), this);
    }
    EckInlineNd CMutableValue NewArray(_In_reads_(cVals) const double* pdVals, size_t cVals) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_with_real(m_pDoc, pdVals, cVals), this);
    }
    EckInlineNd CMutableValue NewArray(_In_reads_(cVals) const char** ppszVals, size_t cVals) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_with_str(m_pDoc, ppszVals, cVals), this);
    }
    EckInlineNd CMutableValue NewArray(_In_reads_(cVals) const char** ppszVals, _In_reads_(cVals) const size_t* pcch, size_t cVals) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_with_strn(m_pDoc, ppszVals, pcch, cVals), this);
    }
    EckInlineNd CMutableValue NewArrayCopy(_In_reads_(cVals) const char** ppszVals, size_t cVals) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_with_strcpy(m_pDoc, ppszVals, cVals), this);
    }
    EckInlineNd CMutableValue NewArrayCopy(_In_reads_(cVals) const char** ppszVals, _In_reads_(cVals) const size_t* pcch, size_t cVals) const noexcept
    {
        return CMutableValue(yyjson_mut_arr_with_strncpy(m_pDoc, ppszVals, pcch, cVals), this);
    }

    EckInlineNd CMutableValue NewObject() const noexcept { return CMutableValue(yyjson_mut_obj(m_pDoc), this); }
    EckInlineNd CMutableValue NewObject(
        _In_reads_(cPairs) const char** ppszKeys,
        _In_reads_(cPairs) const char** pVals,
        size_t cPairs) const noexcept
    {
        return CMutableValue(yyjson_mut_obj_with_str(m_pDoc, ppszKeys, pVals, cPairs), this);
    }
    EckInlineNd CMutableValue NewObject(
        _In_reads_(cPairs * 2) const char** ppszKV,
        size_t cPairs) const noexcept
    {
        return CMutableValue(yyjson_mut_obj_with_kv(m_pDoc, ppszKV, cPairs), this);
    }

    EckInlineNd CMutableValue operator[](const auto& x) const noexcept
    {
        return Detail::JsonValueAtType(*this, x);
    }

    EckInline const CMutableDocument& operator=(Detail::JsonProxy x) const noexcept;
};

namespace Detail
{
    template<class T>
    concept CcpJsonChar = std::same_as<T, char> ||
        std::same_as<T, wchar_t> || std::same_as<T, char8_t>;

    struct JsonProxy
    {
        enum class Type
        {
            Invalid,
            Null,
            Bool,
            Int,
            Int64,
            UInt64,
            Real,
            String,
            StringW,
            JsonProxy,
            ArrayMark,
            JsonMutableValue,
        };

        std::variant<
            std::monostate,
            std::nullptr_t,
            bool,
            int,
            long long,
            unsigned long long,
            double,
            std::string_view,
            std::wstring_view,
            std::initializer_list<JsonProxy>,
            Array_T,
            YyMutableValue*
        > m_Val{};

        JsonProxy(std::nullptr_t) noexcept : m_Val{ nullptr } {}
        JsonProxy(bool x) noexcept : m_Val{ x } {}
        JsonProxy(int x) noexcept : m_Val{ x } {}
        JsonProxy(long long x) noexcept : m_Val{ x } {}
        JsonProxy(unsigned long long x) noexcept : m_Val{ x } {}
        JsonProxy(double x) noexcept : m_Val{ x } {}
        template<CcpChar T, class U>
        JsonProxy(std::basic_string_view<T, U> x) noexcept : m_Val{ std::basic_string_view{ x.data(), x.size() } } {}
        JsonProxy(Array_T) noexcept : m_Val{ Array_T{} } {}
        JsonProxy(std::initializer_list<JsonProxy> x) noexcept : m_Val{ x } {}
        JsonProxy(const CMutableValue& x) noexcept : m_Val{ x.GetPointer() } {}

        JsonProxy(unsigned int x) noexcept : JsonProxy{ (unsigned long long)x } {}
        JsonProxy(unsigned long x) noexcept : JsonProxy{ (unsigned long long)x } {}
        JsonProxy(std::integral auto x) noexcept : JsonProxy{ (int)x } {}
        template<CcpEnum T>
        JsonProxy(T x) noexcept : JsonProxy{ std::underlying_type_t<T>(x) } {}

        template<size_t N>
        JsonProxy(const char(&x)[N]) noexcept : JsonProxy{ std::string_view{ x, N - 1 } } {}
        template<size_t N>
        JsonProxy(const char8_t(&x)[N]) noexcept : JsonProxy{ std::string_view{ (PCCH)x, N - 1 } } {}
        template<size_t N>
        JsonProxy(const wchar_t(&x)[N]) noexcept : JsonProxy{ std::wstring_view{ x, N - 1 } } {}
        JsonProxy(const char* x) noexcept : JsonProxy{ std::string_view(x) } {}
        JsonProxy(const char8_t* x) noexcept : JsonProxy{ std::string_view((PCCH)x) } {}
        JsonProxy(const wchar_t* x) noexcept : JsonProxy{ std::wstring_view(x) } {}
        template<class U>
        JsonProxy(std::basic_string_view<char8_t, U> x) noexcept : JsonProxy{ std::basic_string_view{ (PCCH)x.data(), x.size() } } {}
        template<CcpJsonChar T, class U, class V>
        JsonProxy(const std::basic_string<T, U, V>& x) noexcept : JsonProxy{ std::basic_string_view{ x.data(), x.size() } } {}
        template<CcpJsonChar T, class U, class V>
        JsonProxy(const CStringT<T, U, V>& x) noexcept : JsonProxy{ std::basic_string_view{ x.Data(), (size_t)x.Size() } } {}
        template<class U>
        JsonProxy(const CByteBufferT<U>& rb) noexcept : JsonProxy{ std::string_view{ (PCCH)rb.Data(), rb.Size() } } {}

        EckInlineNdCe Type GetType() const noexcept { return (Type)m_Val.index(); };
        template<Type E>
        EckInlineNdCe auto& Get() const noexcept { return std::get<(size_t)E>(m_Val); }

        void AppendArray(
            const CMutableDocument& Doc,
            const CMutableValue& Arr,
            BOOL bCopyString = FALSE) const noexcept
        {
            const auto& v = Get<Type::JsonProxy>();
            EckAssert(v.begin()->GetType() == Type::ArrayMark);
            for (auto it = v.begin() + 1; it != v.end(); ++it)
                Arr.ArrPushBack(it->ToMutableValue(Doc, bCopyString));
        }
        void AppendObject(
            const CMutableDocument& Doc,
            const CMutableValue& Obj,
            BOOL bCopyString = FALSE) const noexcept
        {
            const auto& v = Get<Type::JsonProxy>();
            EckAssert(!(v.size() & 1));
            CMutableValue Key{ nullptr };
            size_t i{};
            for (auto it = v.begin(); it != v.end(); ++it)
            {
                Key = it->ToMutableValue(Doc, bCopyString);
                ++it;
                Obj.ObjInsert(i, Key, it->ToMutableValue(Doc, bCopyString));
                ++i;
            }
        }

        CMutableValue ToMutableValue(
            const CMutableDocument& Doc,
            BOOL bCopyString = FALSE) const noexcept
        {
            switch (GetType())
            {
            case Type::Null:    return Doc.NewNull();
            case Type::Bool:    return Doc.NewBool(Get<Type::Bool>());
            case Type::Int:     return Doc.NewInt(Get<Type::Int>());
            case Type::Int64:   return Doc.NewInt64(Get<Type::Int64>());
            case Type::UInt64:  return Doc.NewUInt64(Get<Type::UInt64>());
            case Type::Real:    return Doc.NewReal(Get<Type::Real>());
            case Type::String:
            {
                const auto& v = Get<Type::String>();
                if (v.empty())
                    return Doc.NewString("", 0);
                else if (bCopyString)
                    return Doc.NewStringCopy(v.data(), v.size());
                else
                    return Doc.NewString(v.data(), v.size());
            }
            ECK_UNREACHABLE;
            case Type::StringW:
            {
                const auto& v = Get<Type::StringW>();
                const auto u8 = EcdWideToMultiByte(v.data(), (int)v.size(), CP_UTF8);
                return Doc.NewStringCopy(u8.Data(), u8.Size());
            }
            ECK_UNREACHABLE;
            case Type::JsonMutableValue:
                return CMutableValue{ Get<Type::JsonMutableValue>() };
            case Type::JsonProxy:
            {
                const auto& v = Get<Type::JsonProxy>();
                if (v.begin()->GetType() == Type::ArrayMark)
                {
                    CMutableValue Ret{ Doc.NewArray() };
                    AppendArray(Doc, Ret, bCopyString);
                    return Ret;
                }
                else if (!(v.size() & 1))
                {
                    CMutableValue Ret{ Doc.NewObject() };
                    AppendObject(Doc, Ret, bCopyString);
                    return Ret;
                }
                else
                    EckDbgBreak();
            }
            ECK_UNREACHABLE;
            }
            return { nullptr };
        }

        void ReplaceMutValue(
            const CMutableDocument& Doc,
            const CMutableValue& Val,
            BOOL bCopyString = FALSE) const noexcept
        {
            switch (GetType())
            {
            case Type::Null:    Val.SetNull();                      break;
            case Type::Bool:    Val.SetBool(Get<Type::Bool>());     break;
            case Type::Int:     Val.SetInt(Get<Type::Int>());       break;
            case Type::Int64:   Val.SetInt64(Get<Type::Int64>());   break;
            case Type::UInt64:  Val.SetUInt64(Get<Type::UInt64>()); break;
            case Type::Real:    Val.SetReal(Get<Type::Real>());     break;
            case Type::String:
            {
                const auto& v = Get<Type::String>();
                if (v.empty())
                    Val.SetString("", 0);
                else if (bCopyString)
                    Val.SetStringCopy(v.data(), v.size());
                else
                    Val.SetString(v.data(), v.size());
            }
            break;
            case Type::StringW:
            {
                const auto& v = Get<Type::StringW>();
                const auto u8 = EcdWideToMultiByte(v.data(), (int)v.size(), CP_UTF8);
                Val.SetStringCopy(u8.Data(), u8.Size());
            }
            break;
            case Type::JsonProxy:
            {
                const auto& v = Get<Type::JsonProxy>();
                if (v.begin()->GetType() == Type::ArrayMark)
                {
                    Val.SetArray();
                    AppendArray(Doc, Val, bCopyString);
                }
                else if (!(v.size() & 1))
                {
                    Val.SetObject();
                    AppendObject(Doc, Val, bCopyString);
                }
                else
                    EckDbgBreak();
            }
            break;
            default:
                ECK_UNREACHABLE;
            }
        }
    };
}

struct MutableArrayIterator
{
    YyMutableArrayIterator m_Iter{};

    MutableArrayIterator() = default;
    constexpr MutableArrayIterator(const YyMutableArrayIterator& Iter) : m_Iter{ Iter } {}
    MutableArrayIterator(CMutableValue Val) :m_Iter{ yyjson_mut_arr_iter_with(Val.GetPointer()) } {}

    EckInline void FromValue(CMutableValue Val) { m_Iter = yyjson_mut_arr_iter_with(Val.GetPointer()); }
    EckInlineNd BOOL HasNext() const noexcept { return yyjson_mut_arr_iter_has_next((YyMutableArrayIterator*)&m_Iter); }
    EckInlineNd CMutableValue Next() { return CMutableValue(yyjson_mut_arr_iter_next(&m_Iter)); }
    EckInline CMutableValue Remove() { return yyjson_mut_arr_iter_remove(&m_Iter); }
    EckInlineCe CMutableValue GetCurrent() const noexcept { return CMutableValue(m_Iter.cur); }
    EckInline MutableArrayIterator& operator++() { Next(); return *this; }
    EckInlineCe CMutableValue operator*() const noexcept { return GetCurrent(); }
};
EckInlineNd bool operator==(
    const MutableArrayIterator& x,
    const MutableArrayIterator& y) noexcept
{
    return Detail::EqualIterator<MutableArrayIterator>(x, y);
}

struct MutableObjectIterator
{
    YyMutableObjectIterator m_Iter;

    MutableObjectIterator() = default;
    constexpr MutableObjectIterator(const YyMutableObjectIterator& Iter) noexcept : m_Iter{ Iter } {}
    MutableObjectIterator(CMutableValue Val) noexcept
        : m_Iter{ yyjson_mut_obj_iter_with(Val.GetPointer()) } {}

    EckInline void FromValue(CMutableValue Val) noexcept
    {
        m_Iter = yyjson_mut_obj_iter_with(Val.GetPointer());
    }
    EckInlineNd BOOL HasNext() const noexcept
    {
        return yyjson_mut_obj_iter_has_next((YyMutableObjectIterator*)&m_Iter);
    }
    EckInlineNd CMutableValue Next() noexcept
    {
        return CMutableValue(yyjson_mut_obj_iter_next(&m_Iter));
    }
    EckInlineNd CMutableValue Get(_In_z_ PCSTR pszKey) noexcept
    {
        return yyjson_mut_obj_iter_get(&m_Iter, pszKey);
    }
    EckInlineNd CMutableValue Get(
        _In_reads_(cchKey) PCSTR pszKey,
        size_t cchKey) noexcept
    {
        return yyjson_mut_obj_iter_getn(&m_Iter, pszKey, cchKey);
    }
    EckInline CMutableValue Remove() noexcept
    {
        return yyjson_mut_obj_iter_remove(&m_Iter);
    }
    EckInlineCe CMutableValue GetCurrent() const noexcept { return CMutableValue(m_Iter.cur); }
    EckInline MutableObjectIterator& operator++() noexcept { Next(); return *this; }
    EckInlineCe CMutableValue operator*() const noexcept { return GetCurrent(); }
};
EckInlineNd bool operator==(
    const MutableObjectIterator& x,
    const MutableObjectIterator& y) noexcept
{
    return Detail::EqualIterator<MutableObjectIterator>(x, y);
}

struct MutableArrayProxy
{
    CMutableValue Val;
    EckInline MutableArrayIterator begin() const noexcept { return MutableArrayIterator{ Val }; }
    EckInline MutableArrayIterator end() const noexcept { return MutableArrayIterator{}; }
};
struct MutableObjectProxy
{
    CMutableValue Val;
    EckInline MutableObjectIterator begin() const noexcept { return MutableObjectIterator{ Val }; }
    EckInline MutableObjectIterator end() const noexcept { return MutableObjectIterator{}; }
};

EckInline const CMutableValue& CMutableValue::operator=(Detail::JsonProxy x) const noexcept
{
    x.ReplaceMutValue(*m_pDoc, *this);
    return *this;
}
EckInline const CMutableDocument& CMutableDocument::operator=(Detail::JsonProxy x) const noexcept
{
    SetRoot(x.ToMutableValue(*this));
    return *this;
}
EckInlineNd CMutableDocument CDocument::Clone(const YyAllocator* pAlc) const noexcept { return CMutableDocument(yyjson_doc_mut_copy(m_pDoc, pAlc)); }
EckInlineNd ArrayProxy CValue::AsArray() const noexcept { return ArrayProxy(*this); }
EckInlineNd ObjectProxy CValue::AsObject() const noexcept { return ObjectProxy(*this); }
EckInlineNd MutableArrayProxy CMutableValue::AsArray() const noexcept { return MutableArrayProxy(*this); }
EckInlineNd MutableObjectProxy CMutableValue::AsObject() const noexcept { return MutableObjectProxy(*this); }
ECK_JSON_NAMESPACE_END
ECK_NAMESPACE_END

#pragma pop_macro("free")
#pragma pop_macro("malloc")
#pragma pop_macro("realloc")