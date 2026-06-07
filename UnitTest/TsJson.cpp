#include "pch.h"
#include "../eck/Json.h"

using namespace eck;
using namespace eck::Json;

TS_NS_BEGIN
namespace
{
    // 简单标量
    constexpr const char k_JsonNull[] = "null";
    constexpr const char k_JsonTrue[] = "true";
    constexpr const char k_JsonFalse[] = "false";
    constexpr const char k_JsonInt[] = "42";
    constexpr const char k_JsonNegInt[] = "-7";
    constexpr const char k_JsonUInt64[] = "18446744073709551615"; // UINT64_MAX
    constexpr const char k_JsonReal[] = "3.14";
    constexpr const char k_JsonString[] = R"("hello")";
    constexpr const char k_JsonEmptyStr[] = R"("")";
    constexpr const char k_JsonRaw[] = "42"; // 用来做 Raw 测试
    // 数组
    constexpr const char k_JsonArray[] = R"([1,2,3])";
    constexpr const char k_JsonEmptyArray[] = R"([])";
    constexpr const char k_JsonNestedArray[] = R"([[1,2],[3,4]])";
    // 对象
    constexpr const char k_JsonObject[] = R"({"a":1,"b":"str","c":true})";
    constexpr const char k_JsonEmptyObject[] = R"({})";
    constexpr const char k_JsonNestedObject[] = R"({"x":{"y":99}})";
    // 混合复杂
    constexpr const char k_JsonComplex[] =
        R"({"nums":[1,2,3],"flag":false,"sub":{"k":"v"}})";
    // 无效 JSON
    constexpr const char k_JsonInvalid[] = "{ bad json !!! }";
}

// ════════════════════════════════════════════════════════════════════
// TEST CLASS 1: CDocument — 构造、有效性、根节点、索引访问
// ════════════════════════════════════════════════════════════════════
TEST_CLASS(TsJsonDocument)
{
public:
    // ── 基础构造 ─────────────────────────────────────
    TEST_METHOD(Construct_FromCStr_Valid)
    {
        CDocument doc(k_JsonObject);
        Assert::IsTrue(doc.IsValid());
    }

    TEST_METHOD(Construct_FromCStr_Invalid)
    {
        CDocument doc(k_JsonInvalid);
        Assert::IsFalse(doc.IsValid());
    }

    TEST_METHOD(Construct_WithExplicitLength)
    {
        // 只取前 4 个字节 "null" — 剩余内容忽略
        CDocument doc(k_JsonNull, 4);
        Assert::IsTrue(doc.IsValid());
        Assert::IsTrue(doc.GetRoot().IsNull());
    }

    TEST_METHOD(Construct_WithChar8t)
    {
        CDocument doc(u8R"({"x":1})");
        Assert::IsTrue(doc.IsValid());
    }

    TEST_METHOD(Construct_FromStdString)
    {
        std::string s(k_JsonArray);
        CDocument doc(s);
        Assert::IsTrue(doc.IsValid());
    }

    TEST_METHOD(Construct_FromStringView)
    {
        std::string_view sv(k_JsonArray);
        CDocument doc(sv);
        Assert::IsTrue(doc.IsValid());
    }

    // ── Move 语义 ────────────────────────────────────
    TEST_METHOD(MoveConstruct)
    {
        CDocument a(k_JsonObject);
        Assert::IsTrue(a.IsValid());
        CDocument b(std::move(a));
        Assert::IsTrue(b.IsValid());
        Assert::IsFalse(a.IsValid());   // 移后状态
    }

    TEST_METHOD(MoveAssign)
    {
        CDocument a(k_JsonObject);
        CDocument b(k_JsonNull);
        b = std::move(a);
        Assert::IsTrue(b.IsValid());
    }

    // ── Detach / Attach ──────────────────────────────
    TEST_METHOD(DetachAndAttach)
    {
        CDocument doc(k_JsonNull);
        YyDocument* raw = doc.Detach();
        Assert::IsNotNull(raw);
        Assert::IsFalse(doc.IsValid());
        doc.Attach(raw);
        Assert::IsTrue(doc.IsValid());
    }

    // ── Free ─────────────────────────────────────────
    TEST_METHOD(FreeBecomesInvalid)
    {
        CDocument doc(k_JsonNull);
        Assert::IsTrue(doc.IsValid());
        doc.Free();
        Assert::IsFalse(doc.IsValid());
        doc.Free(); // double-free 无 crash
    }

    // ── 根节点读取 ───────────────────────────────────
    TEST_METHOD(GetRoot_NullDoc)
    {
        CDocument doc(k_JsonNull);
        Assert::IsTrue(doc.GetRoot().IsNull());
    }

    TEST_METHOD(GetRoot_Array)
    {
        CDocument doc(k_JsonArray);
        Assert::IsTrue(doc.GetRoot().IsArray());
        Assert::AreEqual((size_t)3, doc.GetRoot().ArrSize());
    }

    TEST_METHOD(GetRoot_Object)
    {
        CDocument doc(k_JsonObject);
        Assert::IsTrue(doc.GetRoot().IsObject());
        Assert::AreEqual((size_t)3, doc.GetRoot().ObjSize());
    }

    // ── GetReadSize / GetValueCount ───────────────────
    TEST_METHOD(GetReadSize_NonZero)
    {
        CDocument doc(k_JsonArray);
        Assert::IsTrue(doc.GetReadSize() > 0);
    }

    TEST_METHOD(GetValueCount_Array3Elements)
    {
        CDocument doc(k_JsonArray);
        // root array + 3 numbers = 4
        Assert::AreEqual((size_t)4, doc.GetValueCount());
    }

    // ── operator[] — JSON Pointer ─────────────────────
    TEST_METHOD(IndexOperator_ObjKey)
    {
        CDocument doc(k_JsonObject);
        CValue v = doc["/a"];
        Assert::IsTrue(v.IsValid());
        Assert::AreEqual(1, v.GetInt());
    }

    TEST_METHOD(IndexOperator_ArrayIndex)
    {
        CDocument doc(k_JsonArray);
        CValue root = doc.GetRoot();
        CValue v = root[size_t(1)];
        Assert::IsTrue(v.IsValid());
        Assert::AreEqual(2, v.GetInt());
    }

    // ── AtValue (JSON Pointer) ────────────────────────
    TEST_METHOD(AtValue_DeepPath)
    {
        CDocument doc(k_JsonNestedObject);
        CValue v = doc.AtValue("/x/y");
        Assert::IsTrue(v.IsValid());
        Assert::AreEqual(99, v.GetInt());
    }

    TEST_METHOD(AtValue_NotFound)
    {
        CDocument doc(k_JsonObject);
        CValue v = doc.AtValue("/nonexistent");
        Assert::IsFalse(v.IsValid());
    }

    // ── Clone -> CMutableDocument ─────────────────────
    TEST_METHOD(Clone_ProducesMutableDocument)
    {
        CDocument doc(k_JsonObject);
        CMutableDocument mut = doc.Clone();
        Assert::IsTrue(mut.IsValid());
    }
};

// ════════════════════════════════════════════════════════════════════
// TEST CLASS 2: CValue — 类型查询、标量读取、容器访问
// ════════════════════════════════════════════════════════════════════
TEST_CLASS(TsJsonValue)
{
public:
    // ── 类型判断：覆盖所有 Is* 分支 ─────────────────
    TEST_METHOD(IsNull_True)
    {
        CDocument doc(k_JsonNull);
        Assert::IsTrue(doc.GetRoot().IsNull());
    }
    TEST_METHOD(IsNull_False)
    {
        CDocument doc(k_JsonTrue);
        Assert::IsFalse(doc.GetRoot().IsNull());
    }

    TEST_METHOD(IsTrue_And_IsBool)
    {
        CDocument doc(k_JsonTrue);
        CValue v = doc.GetRoot();
        Assert::IsTrue(v.IsTrue());
        Assert::IsFalse(v.IsFalse());
        Assert::IsTrue(v.IsBool());
    }
    TEST_METHOD(IsFalse_And_IsBool)
    {
        CDocument doc(k_JsonFalse);
        CValue v = doc.GetRoot();
        Assert::IsFalse(v.IsTrue());
        Assert::IsTrue(v.IsFalse());
        Assert::IsTrue(v.IsBool());
    }

    TEST_METHOD(IsInt_PositiveSmall)
    {
        CDocument doc(k_JsonInt);
        CValue v = doc.GetRoot();
        Assert::IsTrue(v.IsInt());
        Assert::IsTrue(v.IsNumber());
        Assert::AreEqual(42, v.GetInt());
    }
    TEST_METHOD(IsInt64_Negative)
    {
        CDocument doc(k_JsonNegInt);
        CValue v = doc.GetRoot();
        Assert::IsTrue(v.IsInt64());
        Assert::AreEqual((int64_t)-7, v.GetInt64());
    }
    TEST_METHOD(IsUInt64_Max)
    {
        CDocument doc(k_JsonUInt64);
        CValue v = doc.GetRoot();
        Assert::IsTrue(v.IsUInt64());
        Assert::AreEqual((uint64_t)UINT64_MAX, v.GetUInt64());
    }
    TEST_METHOD(IsReal)
    {
        CDocument doc(k_JsonReal);
        CValue v = doc.GetRoot();
        Assert::IsTrue(v.IsReal());
        Assert::AreEqual(3.14, v.GetReal(), 1e-9);
    }
    TEST_METHOD(IsString_NonEmpty)
    {
        CDocument doc(k_JsonString);
        CValue v = doc.GetRoot();
        Assert::IsTrue(v.IsString());
        Assert::AreEqual("hello", v.GetString());
        Assert::AreEqual((size_t)5, v.GetLength());
    }
    TEST_METHOD(IsString_Empty)
    {
        CDocument doc(k_JsonEmptyStr);
        CValue v = doc.GetRoot();
        Assert::IsTrue(v.IsString());
        Assert::AreEqual((size_t)0, v.GetLength());
    }
    TEST_METHOD(IsArray_True)
    {
        CDocument doc(k_JsonArray);
        Assert::IsTrue(doc.GetRoot().IsArray());
        Assert::IsTrue(doc.GetRoot().IsContainer());
    }
    TEST_METHOD(IsObject_True)
    {
        CDocument doc(k_JsonObject);
        Assert::IsTrue(doc.GetRoot().IsObject());
        Assert::IsTrue(doc.GetRoot().IsContainer());
    }

    // ── GetBool ──────────────────────────────────────
    TEST_METHOD(GetBool_True)
    {
        CDocument doc(k_JsonTrue);
        Assert::IsTrue(doc.GetRoot().GetBool());
    }
    TEST_METHOD(GetBool_False)
    {
        CDocument doc(k_JsonFalse);
        Assert::IsFalse(doc.GetRoot().GetBool());
    }

    // ── GetNumber (unified) ──────────────────────────
    TEST_METHOD(GetNumber_FromInt)
    {
        CDocument doc(k_JsonInt);
        Assert::AreEqual(42.0, doc.GetRoot().GetNumber(), 1e-9);
    }

    // ── GetStringView ────────────────────────────────
    TEST_METHOD(GetStringView_CorrectContent)
    {
        CDocument doc(k_JsonString);
        auto sv = doc.GetRoot().GetStringView();
        Assert::AreEqual("hello", std::string(sv).c_str());
    }

    // ── GetStringW ───────────────────────────────────
    TEST_METHOD(GetStringW_NonEmpty)
    {
        CDocument doc(k_JsonString);
        CStringW w = doc.GetRoot().GetStringW();
        Assert::AreEqual(L"hello", w.Data());
    }

    // ── Array 访问 ───────────────────────────────────
    TEST_METHOD(ArrSize_3Elements)
    {
        CDocument doc(k_JsonArray);
        Assert::AreEqual((size_t)3, doc.GetRoot().ArrSize());
    }
    TEST_METHOD(ArrAt_InBounds)
    {
        CDocument doc(k_JsonArray);
        CValue root = doc.GetRoot();
        Assert::AreEqual(1, root.ArrAt(0).GetInt());
        Assert::AreEqual(2, root.ArrAt(1).GetInt());
        Assert::AreEqual(3, root.ArrAt(2).GetInt());
    }
    TEST_METHOD(ArrFront_And_ArrBack)
    {
        CDocument doc(k_JsonArray);
        CValue root = doc.GetRoot();
        Assert::AreEqual(1, root.ArrFront().GetInt());
        Assert::AreEqual(3, root.ArrBack().GetInt());
    }
    TEST_METHOD(ArrAt_EmptyArray)
    {
        CDocument doc(k_JsonEmptyArray);
        CValue v = doc.GetRoot().ArrAt(0);
        Assert::IsFalse(v.IsValid());
    }

    // ── Object 访问 ──────────────────────────────────
    TEST_METHOD(ObjSize_3Pairs)
    {
        CDocument doc(k_JsonObject);
        Assert::AreEqual((size_t)3, doc.GetRoot().ObjSize());
    }
    TEST_METHOD(ObjAt_ExistingKey)
    {
        CDocument doc(k_JsonObject);
        CValue v = doc.GetRoot().ObjAt("b");
        Assert::IsTrue(v.IsValid());
        Assert::IsTrue(v.IsString());
        Assert::AreEqual("str", v.GetString());
    }
    TEST_METHOD(ObjAt_WithLength)
    {
        CDocument doc(k_JsonObject);
        CValue v = doc.GetRoot().ObjAt("a", 1);
        Assert::IsTrue(v.IsValid());
        Assert::AreEqual(1, v.GetInt());
    }
    TEST_METHOD(ObjAt_MissingKey)
    {
        CDocument doc(k_JsonObject);
        CValue v = doc.GetRoot().ObjAt("zzz");
        Assert::IsFalse(v.IsValid());
    }

    // ── ObjGetVal (通过 ObjectIterator 拿到 value) ───
    TEST_METHOD(ObjGetVal_FromIteratorKey)
    {
        CDocument doc(k_JsonObject);
        ObjectIterator it(doc.GetRoot());
        Assert::IsTrue(it.HasNext());
        CValue key = it.Next();       // key = "a"
        CValue val = doc.GetRoot().ObjGetVal(key); // should be 1
        Assert::AreEqual(1, val.GetInt());
    }

    // ── EqualString ──────────────────────────────────
    TEST_METHOD(EqualString_Match)
    {
        CDocument doc(k_JsonString);
        Assert::IsTrue(doc.GetRoot().EqualString("hello"));
    }
    TEST_METHOD(EqualString_NoMatch)
    {
        CDocument doc(k_JsonString);
        Assert::IsFalse(doc.GetRoot().EqualString("world"));
    }
    TEST_METHOD(EqualString_WithLength)
    {
        CDocument doc(k_JsonString);
        Assert::IsTrue(doc.GetRoot().EqualString("hello!", 5)); // 前 5 字节
    }

    // ── GetType / GetSubType / GetTypeDescription ────
    TEST_METHOD(GetType_Array)
    {
        CDocument doc(k_JsonArray);
        Assert::AreEqual((int)YYJSON_TYPE_ARR, (int)doc.GetRoot().GetType());
    }
    TEST_METHOD(GetTypeDescription_String)
    {
        CDocument doc(k_JsonString);
        const char* desc = doc.GetRoot().GetTypeDescription();
        Assert::IsNotNull(desc);
    }

    // ── AsArray / AsObject proxy ─────────────────────
    TEST_METHOD(AsArray_IterationCount)
    {
        CDocument doc(k_JsonArray);
        int cnt = 0;
        for (CValue v : doc.GetRoot().AsArray())
            ++cnt;
        Assert::AreEqual(3, cnt);
    }
    TEST_METHOD(AsObject_IterationCount)
    {
        CDocument doc(k_JsonObject);
        int cnt = 0;
        for (CValue key : doc.GetRoot().AsObject())
            ++cnt;
        Assert::AreEqual(3, cnt);
    }

    // ── AtValue (CValue 层的 JSON Pointer) ───────────
    TEST_METHOD(AtValue_OnValue_Found)
    {
        CDocument doc(k_JsonNestedObject);
        CValue sub = doc.GetRoot().ObjAt("x");
        CValue v = sub.AtValue("/y");
        Assert::AreEqual(99, v.GetInt());
    }
};

// ════════════════════════════════════════════════════════════════════
// TEST CLASS 3: ArrayIterator / ObjectIterator
// ════════════════════════════════════════════════════════════════════
TEST_CLASS(TsJsonImmutableIterator)
{
public:
    // ── ArrayIterator ────────────────────────────────
    TEST_METHOD(ArrayIterator_DefaultIsEnd)
    {
        ArrayIterator a, b;
        Assert::IsTrue(a == b);
    }

    TEST_METHOD(ArrayIterator_EmptyArray_BeginEqualsEnd)
    {
        CDocument doc(k_JsonEmptyArray);
        ArrayIterator beg(doc.GetRoot());
        ArrayIterator end;
        Assert::IsTrue(beg == end);
    }

    TEST_METHOD(ArrayIterator_SingleElement)
    {
        CDocument doc(R"([99])");
        ArrayIterator it(doc.GetRoot());
        Assert::IsTrue(it.HasNext());
        CValue v = it.Next();
        Assert::AreEqual(99, v.GetInt());
        Assert::IsFalse(it.HasNext());
    }

    TEST_METHOD(ArrayIterator_Traversal_3Elements)
    {
        CDocument doc(k_JsonArray);
        ArrayIterator it(doc.GetRoot());
        int expected[] = { 1, 2, 3 };
        int i = 0;
        while (it.HasNext())
        {
            CValue v = it.Next();
            Assert::AreEqual(expected[i++], v.GetInt());
        }
        Assert::AreEqual(3, i);
    }

    TEST_METHOD(ArrayIterator_ForRange_3Elements)
    {
        CDocument doc(k_JsonArray);
        int sum = 0;

        for (CValue v : doc.GetRoot().AsArray())
            sum += v.GetInt();
        Assert::AreEqual(6, sum);
    }

    TEST_METHOD(ArrayIterator_PrefixIncrement_StarDereference)
    {
        CDocument doc(k_JsonArray);
        ArrayIterator it(doc.GetRoot());
        CValue first = *it;
        Assert::AreEqual(1, first.GetInt());
        ++it;
        Assert::AreEqual(2, (*it).GetInt());
    }

    TEST_METHOD(ArrayIterator_FromValue_Method)
    {
        CDocument doc(k_JsonArray);
        ArrayIterator it;
        it.FromValue(doc.GetRoot());
        Assert::IsTrue(it.HasNext());
    }

    TEST_METHOD(ArrayIterator_EqualIterator_DepletedVsEnd)
    {
        // 全部遍历后，迭代器应 == end()
        CDocument doc(R"([42])");
        ArrayIterator it(doc.GetRoot());
        it.Next(); // 取出唯一元素
        ArrayIterator end;
        // 已无 next，should compare equal to end
        Assert::IsTrue(it == end);
    }

    // ── ObjectIterator ───────────────────────────────
    TEST_METHOD(ObjectIterator_DefaultIsEnd)
    {
        ObjectIterator a, b;
        Assert::IsTrue(a == b);
    }

    TEST_METHOD(ObjectIterator_EmptyObject_BeginEqualsEnd)
    {
        CDocument doc(k_JsonEmptyObject);
        ObjectIterator beg(doc.GetRoot());
        ObjectIterator end;
        Assert::IsTrue(beg == end);
    }

    TEST_METHOD(ObjectIterator_Traversal_CollectKeys)
    {
        CDocument doc(k_JsonObject);  // {"a":1,"b":"str","c":true}
        ObjectIterator it(doc.GetRoot());
        std::vector<std::string> keys;
        while (it.HasNext())
        {
            CValue key = it.Next();
            keys.push_back(key.GetString());
        }
        Assert::AreEqual((size_t)3, keys.size());
        Assert::AreEqual("a", keys[0].c_str());
        Assert::AreEqual("b", keys[1].c_str());
        Assert::AreEqual("c", keys[2].c_str());
    }

    TEST_METHOD(ObjectIterator_ForRange)
    {
        CDocument doc(k_JsonObject);
        int cnt = 0;
        for (CValue key : doc.GetRoot().AsObject())
        {
            Assert::IsTrue(key.IsString());
            ++cnt;
        }
        Assert::AreEqual(3, cnt);
    }

    TEST_METHOD(ObjectIterator_Get_ByKey)
    {
        CDocument doc(k_JsonObject);
        ObjectIterator it(doc.GetRoot());
        CValue val = it.Get("b");
        Assert::IsTrue(val.IsValid());
    }

    TEST_METHOD(ObjectIterator_Get_WithLength)
    {
        CDocument doc(k_JsonObject);
        ObjectIterator it(doc.GetRoot());
        CValue val = it.Get("a", 1);
        Assert::IsTrue(val.IsValid());
    }

    TEST_METHOD(ObjectIterator_PrefixIncrement)
    {
        CDocument doc(k_JsonObject);
        ObjectIterator it(doc.GetRoot());
        CValue k1 = *it;
        ++it;
        CValue k2 = *it;
        Assert::AreNotEqual(std::string(k1.GetString()), std::string(k2.GetString()));
    }
};

// ════════════════════════════════════════════════════════════════════
// TEST CLASS 4: CMutableDocument — 创建、工厂方法、写入
// ════════════════════════════════════════════════════════════════════
TEST_CLASS(TsJsonMutableDocument)
{
public:
    TEST_METHOD(DefaultConstruct_IsValid)
    {
        CMutableDocument doc;
        Assert::IsTrue(doc.IsValid());
    }

    TEST_METHOD(CreateFromImmutable)
    {
        CDocument src(k_JsonObject);
        CMutableDocument doc(src);
        Assert::IsTrue(doc.IsValid());
    }

    TEST_METHOD(CreateFromMutable)
    {
        CMutableDocument src;
        src.ProxyReplace(
            {
                "name", "123",
                "age", 18
            });
        CMutableDocument doc{ src.Clone() };
        Assert::IsTrue(doc.IsValid());
    }

    TEST_METHOD(MoveConstruct)
    {
        CMutableDocument a;
        CMutableDocument b(std::move(a));
        Assert::IsTrue(b.IsValid());
        Assert::IsFalse(a.IsValid());
    }

    TEST_METHOD(Free_BecomesInvalid)
    {
        CMutableDocument doc;
        doc.Free();
        Assert::IsFalse(doc.IsValid());
    }

    TEST_METHOD(Detach_And_Attach)
    {
        CMutableDocument doc;
        auto* raw = doc.Detach();
        Assert::IsNotNull(raw);
        Assert::IsFalse(doc.IsValid());
        doc.Attach(raw);
        Assert::IsTrue(doc.IsValid());
    }

    // ── NewXxx 工厂方法覆盖 ──────────────────────────
    TEST_METHOD(NewNull)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewNull());
        Assert::IsTrue(doc.GetRoot().IsNull());
    }
    TEST_METHOD(NewTrue_NewFalse)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewTrue());
        Assert::IsTrue(doc.GetRoot().IsTrue());
        doc.SetRoot(doc.NewFalse());
        Assert::IsTrue(doc.GetRoot().IsFalse());
    }
    TEST_METHOD(NewBool_Branch_True)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewBool(true));
        Assert::IsTrue(doc.GetRoot().IsTrue());
    }
    TEST_METHOD(NewBool_Branch_False)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewBool(false));
        Assert::IsTrue(doc.GetRoot().IsFalse());
    }
    TEST_METHOD(NewInt_NewInt64_NewUInt64)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewInt(123));
        Assert::AreEqual(123, doc.GetRoot().GetInt());

        doc.SetRoot(doc.NewInt64(INT64_MIN));
        Assert::AreEqual(INT64_MIN, doc.GetRoot().GetInt64());

        doc.SetRoot(doc.NewUInt64(UINT64_MAX));
        Assert::AreEqual((uint64_t)UINT64_MAX, doc.GetRoot().GetUInt64());
    }
    TEST_METHOD(NewReal)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewReal(2.718));
        Assert::AreEqual(2.718, doc.GetRoot().GetReal(), 1e-9);
    }
    TEST_METHOD(NewString_NoOwnership)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewString("world"));
        Assert::AreEqual("world", doc.GetRoot().GetString());
    }
    TEST_METHOD(NewStringCopy_Narrow)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewStringCopy("copied"));
        Assert::AreEqual("copied", doc.GetRoot().GetString());
    }
    TEST_METHOD(NewStringCopy_Wide)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewStringCopy(L"宽字符"));
        Assert::IsTrue(doc.GetRoot().IsString());
        Assert::IsTrue(doc.GetRoot().GetLength() > 0);
    }
    TEST_METHOD(NewRaw_And_NewRawCopy)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewRaw("true"));
        Assert::IsTrue(doc.GetRoot().IsRaw());
    }

    // ── NewArray 标量批量工厂 ────────────────────────
    TEST_METHOD(NewArray_FromInt32Span)
    {
        CMutableDocument doc;
        int32_t vals[] = { 10, 20, 30 };
        doc.SetRoot(doc.NewArray(vals, 3));
        Assert::IsTrue(doc.GetRoot().IsArray());
        Assert::AreEqual((size_t)3, doc.GetRoot().ArrSize());
    }
    TEST_METHOD(NewArray_FromDoubleSpan)
    {
        CMutableDocument doc;
        double vals[] = { 1.1, 2.2 };
        doc.SetRoot(doc.NewArray(vals, 2));
        Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
    }
    TEST_METHOD(NewArray_FromBoolSpan)
    {
        CMutableDocument doc;
        bool vals[] = { true, false, true };
        doc.SetRoot(doc.NewArray(vals, 3));
        Assert::AreEqual((size_t)3, doc.GetRoot().ArrSize());
    }
    // 更多整型重载
    TEST_METHOD(NewArray_Int8_Int16_Int64)
    {
        CMutableDocument doc;
        int8_t  v8[] = { 1, -1 };
        int16_t v16[] = { 100, -100 };
        int64_t v64[] = { INT64_MAX, INT64_MIN };
        doc.SetRoot(doc.NewArray(v8, 2));   Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
        doc.SetRoot(doc.NewArray(v16, 2));  Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
        doc.SetRoot(doc.NewArray(v64, 2));  Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
    }
    TEST_METHOD(NewArray_UInt8_UInt16_UInt32_UInt64)
    {
        CMutableDocument doc;
        uint8_t  u8[] = { 0, 255 };
        uint16_t u16[] = { 0, 65535 };
        uint32_t u32[] = { 0, UINT32_MAX };
        uint64_t u64[] = { 0, UINT64_MAX };
        doc.SetRoot(doc.NewArray(u8, 2));   Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
        doc.SetRoot(doc.NewArray(u16, 2));  Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
        doc.SetRoot(doc.NewArray(u32, 2));  Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
        doc.SetRoot(doc.NewArray(u64, 2));  Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
    }
    TEST_METHOD(NewArray_FloatSpan)
    {
        CMutableDocument doc;
        float vals[] = { 1.f, 2.f };
        doc.SetRoot(doc.NewArray(vals, 2));
        Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
    }
    TEST_METHOD(NewArray_CstrPtrArray)
    {
        CMutableDocument doc;
        const char* strs[] = { "foo", "bar" };
        doc.SetRoot(doc.NewArray(strs, 2));
        Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
    }
    TEST_METHOD(NewArray_CstrWithLengths)
    {
        CMutableDocument doc;
        const char* strs[] = { "foo", "bar" };
        size_t lens[] = { 3, 3 };
        doc.SetRoot(doc.NewArray(strs, lens, 2));
        Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
    }
    TEST_METHOD(NewArrayCopy_Cstr)
    {
        CMutableDocument doc;
        const char* strs[] = { "hello", "world" };
        doc.SetRoot(doc.NewArrayCopy(strs, 2));
        Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
    }

    // ── NewObject 工厂 ───────────────────────────────
    TEST_METHOD(NewObject_Empty)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewObject());
        Assert::IsTrue(doc.GetRoot().IsObject());
        Assert::AreEqual((size_t)0, doc.GetRoot().ObjSize());
    }
    TEST_METHOD(NewObject_FromKVPairs)
    {
        CMutableDocument doc;
        const char* kv[] = { "key1", "val1", "key2", "val2" };
        doc.SetRoot(doc.NewObject(kv, 2));
        Assert::AreEqual((size_t)2, doc.GetRoot().ObjSize());
    }

    // ── SetStringPoolSize / SetValuePoolSize ─────────
    TEST_METHOD(SetStringPoolSize_Succeeds)
    {
        CMutableDocument doc;
        BOOL ok = doc.SetStringPoolSize(4096);
        Assert::IsTrue(ok);
    }

    // ── Write ────────────────────────────────────────
    TEST_METHOD(Write_Roundtrip_Object)
    {
        CDocument src(k_JsonObject);
        CMutableDocument mut(src);
        size_t cch;
        PSTR pszJson = mut.Write(&cch);
        Assert::IsNotNull(pszJson);
        Assert::IsTrue(cch > 0);
        free(pszJson);
    }

    TEST_METHOD(WriteW_Roundtrip)
    {
        CDocument src(k_JsonObject);
        CMutableDocument mut(src);
        CStringW w = mut.WriteW();
        Assert::IsTrue(w.Size() > 0);
    }

    // ── Clone / CloneImmutable ───────────────────────
    TEST_METHOD(Clone_ProducesMutableDoc)
    {
        CMutableDocument a;
        a = nullptr;
        CMutableDocument b = a.Clone();
        Assert::IsTrue(b.IsValid());
    }
    TEST_METHOD(CloneImmutable)
    {
        CMutableDocument a;
        a = nullptr;
        CDocument b = a.CloneImmutable();
        Assert::IsTrue(b.IsValid());
    }

    // ── operator[] — JSON Pointer ─────────────────────
    TEST_METHOD(IndexOperator_ByKey_OnRoot)
    {
        CDocument src(k_JsonObject);
        CMutableDocument doc(src);
        CMutableValue v = doc["/a"];
        Assert::IsTrue(v.IsValid());
        Assert::AreEqual(1, v.GetInt());
    }

    // ── AtValue ──────────────────────────────────────
    TEST_METHOD(AtValue_Deep)
    {
        CDocument src(k_JsonNestedObject);
        CMutableDocument doc(src);
        CMutableValue v = doc.AtValue("/x/y");
        Assert::AreEqual(99, v.GetInt());
    }
};

// ════════════════════════════════════════════════════════════════════
// TEST CLASS 5: CMutableValue — 变异操作、容器操作
// ════════════════════════════════════════════════════════════════════
TEST_CLASS(TsJsonMutableValue)
{
public:
    // ── Set* 标量变异 ────────────────────────────────
    TEST_METHOD(SetNull_ChangeType)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewInt(1));
        doc.GetRoot().SetNull();
        Assert::IsTrue(doc.GetRoot().IsNull());
    }
    TEST_METHOD(SetBool_True_And_False)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewInt(0));
        doc.GetRoot().SetBool(true);
        Assert::IsTrue(doc.GetRoot().IsTrue());
        doc.GetRoot().SetBool(false);
        Assert::IsTrue(doc.GetRoot().IsFalse());
    }
    TEST_METHOD(SetInt_And_SetInt64)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewNull());
        doc.GetRoot().SetInt(42);
        Assert::AreEqual(42, doc.GetRoot().GetInt());

        doc.GetRoot().SetInt64(INT64_MIN);
        Assert::AreEqual(INT64_MIN, doc.GetRoot().GetInt64());
    }
    TEST_METHOD(SetUInt64)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewNull());
        doc.GetRoot().SetUInt64(UINT64_MAX);
        Assert::AreEqual((uint64_t)UINT64_MAX, doc.GetRoot().GetUInt64());
    }
    TEST_METHOD(SetReal)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewNull());
        doc.GetRoot().SetReal(1.5);
        Assert::AreEqual(1.5, doc.GetRoot().GetReal(), 1e-9);
    }
    TEST_METHOD(SetString_NoOwnership)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewNull());
        const char* s = "test_string";
        doc.GetRoot().SetString(s);
        Assert::AreEqual("test_string", doc.GetRoot().GetString());
    }
    TEST_METHOD(SetString_WithLength)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewNull());
        doc.GetRoot().SetString("hello world", 5); // 只取 "hello"
        Assert::AreEqual((size_t)5, doc.GetRoot().GetLength());
    }
    TEST_METHOD(SetStringCopy_WithDoc)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewNull());
        CMutableValue root = doc.GetRoot();
        root.SetString("copied_val", 10);
        Assert::AreEqual("copied_val", doc.GetRoot().GetString());
    }

    TEST_METHOD(SetArray_And_SetObject)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewNull());
        doc.GetRoot().SetArray(2);
        Assert::IsTrue(doc.GetRoot().IsArray());
        doc.GetRoot().SetObject(2);
        Assert::IsTrue(doc.GetRoot().IsObject());
    }
    TEST_METHOD(SetRaw)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewNull());
        doc.GetRoot().SetRaw("99");
        Assert::IsTrue(doc.GetRoot().IsRaw());
    }

    // ── Array 变异操作 ───────────────────────────────
    TEST_METHOD(ArrPushBack_ArrPopBack)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewArray());
        doc.GetRoot().ArrPushBack(doc.NewInt(1));
        doc.GetRoot().ArrPushBack(doc.NewInt(2));
        Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
        CMutableValue popped = doc.GetRoot().ArrPopBack();
        Assert::AreEqual(2, popped.GetInt());
        Assert::AreEqual((size_t)1, doc.GetRoot().ArrSize());
    }

    TEST_METHOD(ArrPushFront_ArrPopFront)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewArray());
        doc.GetRoot().ArrPushBack(doc.NewInt(10));
        doc.GetRoot().ArrPushFront(doc.NewInt(5));
        Assert::AreEqual(5, doc.GetRoot().ArrFront().GetInt());
        CMutableValue p = doc.GetRoot().ArrPopFront();
        Assert::AreEqual(5, p.GetInt());
    }

    TEST_METHOD(ArrInsert_MiddlePosition)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewArray());
        doc.GetRoot().ArrPushBack(doc.NewInt(1));
        doc.GetRoot().ArrPushBack(doc.NewInt(3));
        doc.GetRoot().ArrInsert(1, doc.NewInt(2));
        Assert::AreEqual(3u, (unsigned)doc.GetRoot().ArrSize());
        Assert::AreEqual(2, doc.GetRoot().ArrAt(1).GetInt());
    }

    TEST_METHOD(ArrReplace)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewArray());
        doc.GetRoot().ArrPushBack(doc.NewInt(1));
        doc.GetRoot().ArrReplace(0, doc.NewInt(99));
        Assert::AreEqual(99, doc.GetRoot().ArrAt(0).GetInt());
    }

    TEST_METHOD(ArrRemove_Single)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewArray());
        doc.GetRoot().ArrPushBack(doc.NewInt(1));
        doc.GetRoot().ArrPushBack(doc.NewInt(2));
        doc.GetRoot().ArrRemove(size_t(0));
        Assert::AreEqual((size_t)1, doc.GetRoot().ArrSize());
        Assert::AreEqual(2, doc.GetRoot().ArrAt(0).GetInt());
    }

    TEST_METHOD(ArrRemove_Range)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewArray());
        for (int i = 0; i < 5; ++i)
            doc.GetRoot().ArrPushBack(doc.NewInt(i));
        doc.GetRoot().ArrRemove(size_t(1), size_t(3)); // 删除索引 1-3
        Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
    }

    TEST_METHOD(ArrClear)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewArray());
        doc.GetRoot().ArrPushBack(doc.NewInt(1));
        doc.GetRoot().ArrClear();
        Assert::AreEqual((size_t)0, doc.GetRoot().ArrSize());
    }

    TEST_METHOD(ArrRotate)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewArray());
        for (int i = 0; i < 3; ++i)
            doc.GetRoot().ArrPushBack(doc.NewInt(i));
        doc.GetRoot().ArrRotate(1); // [0,1,2] -> [1,2,0]
        Assert::AreEqual(1, doc.GetRoot().ArrAt(0).GetInt());
        Assert::AreEqual(0, doc.GetRoot().ArrAt(2).GetInt());
    }

    // ── Object 变异操作 ──────────────────────────────
    TEST_METHOD(ObjInsert_And_ObjSize)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewObject());
        doc.GetRoot().ObjInsert(0, doc.NewString("k"), doc.NewInt(1));
        Assert::AreEqual((size_t)1, doc.GetRoot().ObjSize());
    }

    TEST_METHOD(ObjRemove_ByKey)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewObject());
        doc.GetRoot().ObjInsert(0, doc.NewString("x"), doc.NewInt(7));
        doc.GetRoot().ObjRemove(doc.NewString("x"));
        Assert::AreEqual((size_t)0, doc.GetRoot().ObjSize());
    }

    TEST_METHOD(ObjRemove_ByCStr)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewObject());
        doc.GetRoot().ObjInsert(0, doc.NewString("y"), doc.NewInt(8));
        doc.GetRoot().ObjRemove("y");
        Assert::AreEqual((size_t)0, doc.GetRoot().ObjSize());
    }

    TEST_METHOD(ObjReplace_ExistingKey)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewObject());
        doc.GetRoot().ObjInsert(0, doc.NewString("n"), doc.NewInt(1));
        doc.GetRoot().ObjReplace(doc.NewString("n"), doc.NewInt(42));
        Assert::AreEqual(42, doc.GetRoot().ObjAt("n").GetInt());
    }

    TEST_METHOD(ObjClear)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewObject());
        doc.GetRoot().ObjInsert(0, doc.NewString("a"), doc.NewInt(1));
        doc.GetRoot().ObjClear();
        Assert::AreEqual((size_t)0, doc.GetRoot().ObjSize());
    }

    TEST_METHOD(ObjRotate)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewObject());
        doc.GetRoot().ObjInsert(0, doc.NewString("a"), doc.NewInt(1));
        doc.GetRoot().ObjInsert(1, doc.NewString("b"), doc.NewInt(2));
        doc.GetRoot().ObjInsert(2, doc.NewString("c"), doc.NewInt(3));
        doc.GetRoot().ObjRotate(1); // {"a":1,"b":2,"c":3} -> {"b":2,"c":3,"a":1}
        Assert::IsTrue(doc.GetRoot().ObjAt("a").IsValid());
    }

    // ── AsArray / AsObject proxy ─────────────────────
    TEST_METHOD(AsArray_MutableProxy)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewArray());
        for (int i = 0; i < 3; ++i)
            doc.GetRoot().ArrPushBack(doc.NewInt(i * 10));
        int sum = 0;
        for (CMutableValue v : doc.GetRoot().AsArray())
            sum += v.GetInt();
        Assert::AreEqual(30, sum); // 0+10+20
    }

    TEST_METHOD(AsObject_MutableProxy)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewObject());
        doc.GetRoot().ObjInsert(0, doc.NewString("x"), doc.NewInt(5));
        int cnt = 0;
        for (CMutableValue k : doc.GetRoot().AsObject())
        {
            Assert::IsTrue(k.IsString());
            ++cnt;
        }
        Assert::AreEqual(1, cnt);
    }

    // ── AtValue (JSON Pointer) ───────────────────────
    TEST_METHOD(AtValue_OnMutableValue)
    {
        CDocument src(k_JsonNestedObject);
        CMutableDocument doc(src);
        CMutableValue root = doc.GetRoot();
        CMutableValue v = root.AtValue("/x/y");
        Assert::AreEqual(99, v.GetInt());
    }

    // ── Write ────────────────────────────────────────
    TEST_METHOD(Write_Scalar)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewInt(7));
        size_t cch;
        PSTR p = doc.GetRoot().Write(&cch);
        Assert::IsNotNull(p);
        Assert::AreEqual('7', p[0]);
        free(p);
    }
};

// ════════════════════════════════════════════════════════════════════
// TEST CLASS 6: MutableArrayIterator / MutableObjectIterator
// ════════════════════════════════════════════════════════════════════
TEST_CLASS(TsJsonMutableIterator)
{
public:
    // ── MutableArrayIterator ─────────────────────────
    TEST_METHOD(MutableArrayIterator_DefaultIsEnd)
    {
        MutableArrayIterator a, b;
        Assert::IsTrue(a == b);
    }

    TEST_METHOD(MutableArrayIterator_EmptyArray)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewArray());
        MutableArrayIterator beg(doc.GetRoot());
        MutableArrayIterator end;
        Assert::IsTrue(beg == end);
    }

    TEST_METHOD(MutableArrayIterator_Traversal)
    {
        CMutableDocument doc;
        const auto arr = doc.NewArray();
        doc.SetRoot(arr);
        arr.ArrPushBack(doc.NewInt(10));
        arr.ArrPushBack(doc.NewInt(20));
        arr.ArrPushBack(doc.NewInt(30));

        MutableArrayIterator it(doc.GetRoot());
        MutableArrayIterator itEnd{};
        int expected[] = { 10, 20, 30 };
        int i = 0;
        while (it != itEnd)
        {
            CMutableValue v = it.GetCurrent();
            auto aaa = v.GetInt();
            Assert::AreEqual(expected[i++], v.GetInt());
            ++it;
        }
        Assert::AreEqual(3, i);
    }

    TEST_METHOD(MutableArrayIterator_ForRange)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewArray());
        for (int i = 1; i <= 5; ++i)
            doc.GetRoot().ArrPushBack(doc.NewInt(i));
        int sum = 0;
        for (CMutableValue v : doc.GetRoot().AsArray())
            sum += v.GetInt();
        Assert::AreEqual(15, sum);
    }

    TEST_METHOD(MutableArrayIterator_Remove_WhileIterating)
    {
        // 演示边删边走
        CMutableDocument doc;
        doc.SetRoot(doc.NewArray());
        for (int i = 0; i < 4; ++i)
            doc.GetRoot().ArrPushBack(doc.NewInt(i));
        MutableArrayIterator it(doc.GetRoot());
        while (it.HasNext())
        {
            CMutableValue v = it.Next();
            if (v.GetInt() % 2 == 0)
                it.Remove();
        }
        // 仅剩奇数 1,3
        Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
    }

    TEST_METHOD(MutableArrayIterator_PrefixIncrement_Star)
    {
        CMutableDocument doc;
        const auto arr = doc.NewArray();
        doc.SetRoot(arr);
        arr.ArrPushBack(doc.NewInt(7));
        arr.ArrPushBack(doc.NewInt(8));
        MutableArrayIterator it(doc.GetRoot());
        Assert::AreEqual(7, (*it).GetInt());
        ++it;
        Assert::AreEqual(8, (*it).GetInt());
    }

    TEST_METHOD(MutableArrayIterator_FromValue_Method)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewArray());
        doc.GetRoot().ArrPushBack(doc.NewInt(1));
        MutableArrayIterator it;
        it.FromValue(doc.GetRoot());
        Assert::IsFalse(it.HasNext());
    }

    // ── MutableObjectIterator ────────────────────────

    TEST_METHOD(MutableObjectIterator_EmptyObject)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewObject());
        MutableObjectIterator beg(doc.GetRoot());
        MutableObjectIterator end;
        Assert::IsTrue(beg == end);
    }

    TEST_METHOD(MutableObjectIterator_Traversal_CollectKeys)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewObject());
        doc.GetRoot().ObjInsert(0, doc.NewString("k1"), doc.NewInt(1));
        doc.GetRoot().ObjInsert(1, doc.NewString("k2"), doc.NewInt(2));

        MutableObjectIterator it(doc.GetRoot());
        std::vector<std::string> keys;
        while (it != MutableObjectIterator{})
        {
            CMutableValue k = it.GetCurrent();
            keys.push_back(k.GetString());
            it.Next();
        }
        Assert::AreEqual((size_t)2, keys.size());
        Assert::AreEqual("k1", keys[0].c_str());
        Assert::AreEqual("k2", keys[1].c_str());
    }

    TEST_METHOD(MutableObjectIterator_ForRange)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewObject());
        doc.GetRoot().ObjInsert(0, doc.NewString("a"), doc.NewInt(1));
        doc.GetRoot().ObjInsert(1, doc.NewString("b"), doc.NewInt(2));
        int cnt = 0;
        for (CMutableValue key : doc.GetRoot().AsObject())
        {
            Assert::IsTrue(key.IsString());
            ++cnt;
        }
        Assert::AreEqual(2, cnt);
    }

    TEST_METHOD(MutableObjectIterator_Get_ByKey)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewObject());
        doc.GetRoot().ObjInsert(0, doc.NewString("mykey"), doc.NewInt(42));
        MutableObjectIterator it(doc.GetRoot());
        CMutableValue val = it.Get("mykey");
        Assert::IsTrue(val.IsValid());
        Assert::AreEqual(42, val.GetInt());
    }

    TEST_METHOD(MutableObjectIterator_Get_WithLength)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewObject());
        doc.GetRoot().ObjInsert(0, doc.NewString("abc"), doc.NewInt(9));
        MutableObjectIterator it(doc.GetRoot());
        CMutableValue val = it.Get("abc", 3);
        Assert::IsTrue(val.IsValid());
    }

    TEST_METHOD(MutableObjectIterator_Remove_WhileIterating)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewObject());
        doc.GetRoot().ObjInsert(0, doc.NewString("del"), doc.NewInt(0));
        doc.GetRoot().ObjInsert(1, doc.NewString("keep"), doc.NewInt(1));

        MutableObjectIterator it(doc.GetRoot());
        while (it != MutableObjectIterator{})
        {
            CMutableValue k = it.GetCurrent();
            if (std::string(k.GetString()) == "del")
                it.Remove();
            it.Next();
        }
        Assert::AreEqual((size_t)1, doc.GetRoot().ObjSize());
        Assert::IsTrue(doc.GetRoot().ObjAt("keep").IsValid());
    }

    TEST_METHOD(MutableObjectIterator_PrefixIncrement)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewObject());
        doc.GetRoot().ObjInsert(0, doc.NewString("p"), doc.NewInt(1));
        doc.GetRoot().ObjInsert(1, doc.NewString("q"), doc.NewInt(2));
        MutableObjectIterator it(doc.GetRoot());
        CMutableValue k1 = *it;
        ++it;
        CMutableValue k2 = *it;
        Assert::AreNotEqual(std::string(k1.GetString()), std::string(k2.GetString()));
    }
};

// ════════════════════════════════════════════════════════════════════
// TEST CLASS 7: Detail::JsonProxy — operator= 赋值语法糖
// ════════════════════════════════════════════════════════════════════
TEST_CLASS(TsJsonProxy)
{
public:
    // ── 标量赋值（CMutableDocument::operator=） ──────
    TEST_METHOD(Proxy_AssignNull)
    {
        CMutableDocument doc;
        doc = nullptr;
        Assert::IsTrue(doc.GetRoot().IsNull());
    }
    TEST_METHOD(Proxy_AssignBool_True)
    {
        CMutableDocument doc;
        doc = true;
        Assert::IsTrue(doc.GetRoot().IsTrue());
    }
    TEST_METHOD(Proxy_AssignBool_False)
    {
        CMutableDocument doc;
        doc = false;
        Assert::IsTrue(doc.GetRoot().IsFalse());
    }
    TEST_METHOD(Proxy_AssignInt)
    {
        CMutableDocument doc;
        doc = 42;
        Assert::AreEqual(42, doc.GetRoot().GetInt());
    }
    TEST_METHOD(Proxy_AssignInt64)
    {
        CMutableDocument doc;
        doc = (long long)INT64_MIN;
        Assert::AreEqual(INT64_MIN, doc.GetRoot().GetInt64());
    }
    TEST_METHOD(Proxy_AssignUInt64)
    {
        CMutableDocument doc;
        doc = (unsigned long long)UINT64_MAX;
        Assert::AreEqual((uint64_t)UINT64_MAX, doc.GetRoot().GetUInt64());
    }
    TEST_METHOD(Proxy_AssignDouble)
    {
        CMutableDocument doc;
        doc = 2.71828;
        Assert::AreEqual(2.71828, doc.GetRoot().GetReal(), 1e-9);
    }
    TEST_METHOD(Proxy_AssignCStr_Literal)
    {
        CMutableDocument doc;
        doc = "hello";
        Assert::AreEqual("hello", doc.GetRoot().GetString());
    }
    TEST_METHOD(Proxy_AssignChar8t_Literal)
    {
        CMutableDocument doc;
        doc = u8"utf8text";
        Assert::AreEqual("utf8text", doc.GetRoot().GetString());
    }
    TEST_METHOD(Proxy_AssignWCStr_Literal)
    {
        CMutableDocument doc;
        doc = L"wide";
        Assert::IsTrue(doc.GetRoot().IsString());
    }
    TEST_METHOD(Proxy_AssignStdString)
    {
        CMutableDocument doc;
        std::string s("stdstr");
        doc = s;
        Assert::AreEqual("stdstr", doc.GetRoot().GetString());
    }
    TEST_METHOD(Proxy_AssignStdStringView)
    {
        CMutableDocument doc;
        std::string_view sv("svstr");
        doc = sv;
        Assert::AreEqual("svstr", doc.GetRoot().GetString());
    }

    // ── 枚举类型 ────────────────────────────────────
    TEST_METHOD(Proxy_AssignEnum)
    {
        enum class Color { Red = 1, Green = 2 };
        CMutableDocument doc;
        doc = Color::Green;
        Assert::AreEqual(2, doc.GetRoot().GetInt());
    }

    // ── 无符号整型重载 ───────────────────────────────
    TEST_METHOD(Proxy_AssignUnsignedInt)
    {
        CMutableDocument doc;
        doc = (unsigned int)999u;
        Assert::AreEqual((uint64_t)999u, doc.GetRoot().GetUInt64());
    }

    // ── 内联对象语法 {key,val,...} ──────────────────
    TEST_METHOD(Proxy_InlineObject_TwoPairs)
    {
        CMutableDocument doc;
        doc = { "name", "Alice", "age", 30 };
        Assert::IsTrue(doc.GetRoot().IsObject());
        Assert::AreEqual("Alice", doc.GetRoot().ObjAt("name").GetString());
        Assert::AreEqual(30, doc.GetRoot().ObjAt("age").GetInt());
    }

    TEST_METHOD(Proxy_InlineObject_SinglePair)
    {
        CMutableDocument doc;
        doc = { "x", 1 };
        Assert::IsTrue(doc.GetRoot().IsObject());
        Assert::AreEqual(1, doc.GetRoot().ObjAt("x").GetInt());
    }

    // ── 内联数组语法 {Array_T{}, e1, e2, ...} ────────
    TEST_METHOD(Proxy_InlineArray)
    {
        CMutableDocument doc;
        doc = { Array_T{}, 1, 2, 3 };
        Assert::IsTrue(doc.GetRoot().IsArray());
        Assert::AreEqual((size_t)3, doc.GetRoot().ArrSize());
        Assert::AreEqual(1, doc.GetRoot().ArrAt(0).GetInt());
        Assert::AreEqual(3, doc.GetRoot().ArrAt(2).GetInt());
    }

    TEST_METHOD(Proxy_InlineArray_Empty)
    {
        CMutableDocument doc;
        doc = { Array_T{} };
        Assert::IsTrue(doc.GetRoot().IsArray());
        Assert::AreEqual((size_t)0, doc.GetRoot().ArrSize());
    }

    TEST_METHOD(Proxy_InlineArray_MixedTypes)
    {
        CMutableDocument doc;
        doc = { Array_T{}, 1, "two", 3.0, true, nullptr };
        Assert::AreEqual((size_t)5, doc.GetRoot().ArrSize());
        Assert::IsTrue(doc.GetRoot().ArrAt(1).IsString());
        Assert::IsTrue(doc.GetRoot().ArrAt(3).IsBool());
        Assert::IsTrue(doc.GetRoot().ArrAt(4).IsNull());
    }

    // ── 嵌套结构 ────────────────────────────────────
    TEST_METHOD(Proxy_NestedObject_InArray)
    {
        CMutableDocument doc;
        doc = { Array_T{},
               {Array_T{}, 1, 2},
               {"k", "v"}
        };
        Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
        Assert::IsTrue(doc.GetRoot().ArrAt(0).IsArray());
        Assert::IsTrue(doc.GetRoot().ArrAt(1).IsObject());
    }

    // ── CMutableValue::operator= (ReplaceMutValue) ──
    TEST_METHOD(Proxy_ReplaceExistingValue_Int)
    {
        CMutableDocument doc;
        doc = 1;
        doc.GetRoot().ProxyReplace(99, doc);
        Assert::AreEqual(99, doc.GetRoot().GetInt());
    }

    TEST_METHOD(Proxy_ReplaceExistingValue_String)
    {
        CMutableDocument doc;
        doc = "old";
        doc.GetRoot().ProxyReplace("new", doc);
        Assert::AreEqual("new", doc.GetRoot().GetString());
    }

    TEST_METHOD(Proxy_ReplaceExistingValue_ToArray)
    {
        CMutableDocument doc;
        doc = 0;
        doc.GetRoot().ProxyReplace({ Array_T{}, 10, 20 }, doc);
        Assert::IsTrue(doc.GetRoot().IsArray());
        Assert::AreEqual((size_t)2, doc.GetRoot().ArrSize());
    }

    TEST_METHOD(Proxy_ReplaceExistingValue_ToObject)
    {
        CMutableDocument doc;
        doc = 0;
        doc.GetRoot().ProxyReplace({ "a", 1 }, doc);
        Assert::IsTrue(doc.GetRoot().IsObject());
    }

    TEST_METHOD(Proxy_ReplaceExistingValue_NullToTrue)
    {
        CMutableDocument doc;
        doc = nullptr;
        doc.GetRoot().ProxyReplace(true, doc);
        Assert::IsTrue(doc.GetRoot().IsTrue());
    }
};


TEST_CLASS(TsJsonOthers)
{
public:
    // CDocument 的 CStringT 模板构造
    TEST_METHOD(CDocument_FromCStringT_Char)
    {
        CStringA s(k_JsonNull);
        CDocument doc(s);
        Assert::IsTrue(doc.IsValid());
        Assert::IsTrue(doc.GetRoot().IsNull());
    }

    // CDocument 的 std::basic_string 模板构造
    TEST_METHOD(CDocument_FromStdString_Char)
    {
        std::string s(k_JsonTrue);
        CDocument doc(s);
        Assert::IsTrue(doc.IsValid());
        Assert::IsTrue(doc.GetRoot().IsTrue());
    }

    // JsonProxy 的 CStringT<wchar_t> 构造
    TEST_METHOD(Proxy_AssignCStringW)
    {
        CMutableDocument doc;
        CStringW ws(L"wcs_test");
        doc = ws;
        Assert::IsTrue(doc.GetRoot().IsString());
    }

    // JsonProxy 的 CStringT<char> 构造
    TEST_METHOD(Proxy_AssignCStringA)
    {
        CMutableDocument doc;
        CStringA cs("narrow_eck");
        doc = cs;
        Assert::AreEqual("narrow_eck", doc.GetRoot().GetString());
    }

    // JsonProxy 的 std::wstring 构造
    TEST_METHOD(Proxy_AssignStdWstring)
    {
        CMutableDocument doc;
        std::wstring ws(L"wide_std");
        doc = ws;
        Assert::IsTrue(doc.GetRoot().IsString());
    }

    // JsonProxy 的 CByteBuffer 构造（当作 UTF-8 字节串）
    TEST_METHOD(Proxy_AssignCByteBuffer)
    {
        CMutableDocument doc;
        CByteBuffer buf;
        buf.PushBack((const BYTE*)"bytes", 5);
        doc = buf;
        Assert::AreEqual((size_t)5, doc.GetRoot().GetLength());
    }

    // Detail::JsonValueAtType：字符串字面量重载（char[N]）
    TEST_METHOD(JsonValueAtType_CharArrayLiteral)
    {
        CDocument doc(k_JsonObject);
        CValue v = doc.GetRoot()["/a"];  // const char(&)[2]
        Assert::AreEqual(1, v.GetInt());
    }

    // Detail::JsonValueAtType：char8_t 字面量重载
    TEST_METHOD(JsonValueAtType_Char8tLiteral)
    {
        CDocument doc(k_JsonObject);
        CValue v = doc.GetRoot()[u8"/b"];
        Assert::IsTrue(v.IsString());
    }

    // Detail::JsonValueAtType：std::string_view
    TEST_METHOD(JsonValueAtType_StdStringView)
    {
        CDocument doc(k_JsonObject);
        std::string_view sv("/c");
        CValue v = doc.GetRoot()[sv];
        Assert::IsTrue(v.IsBool());
    }

    // CMutableDocument 的 NewStringCopy(PCWSTR) 重载
    TEST_METHOD(NewStringCopy_WideOverload)
    {
        CMutableDocument doc;
        CMutableValue v = doc.NewStringCopy(L"日本語", 3);
        Assert::IsTrue(v.IsString());
        Assert::IsTrue(v.GetLength() > 0);  // UTF-8 编码后更长
    }

    // CMutableDocument 的 NewArrayCopy 带 length 重载
    TEST_METHOD(NewArrayCopy_WithLengths)
    {
        CMutableDocument doc;
        const char* strs[] = { "ab", "cdef" };
        size_t lens[] = { 2, 4 };
        CMutableValue arr = doc.NewArrayCopy(strs, lens, 2);
        Assert::IsTrue(arr.IsArray());
        Assert::AreEqual((size_t)2, arr.ArrSize());
    }

    // NewObject with kv flat array
    TEST_METHOD(NewObject_FromKVFlatArray)
    {
        CMutableDocument doc;
        const char* kv[] = { "a","1","b","2" };
        CMutableValue obj = doc.NewObject(kv, 2);
        Assert::IsTrue(obj.IsObject());
        Assert::AreEqual((size_t)2, obj.ObjSize());
    }

    // ── 空数组/对象 ──────────────────────────────────
    TEST_METHOD(EmptyArray_ArrFront_Invalid)
    {
        CDocument doc(k_JsonEmptyArray);
        Assert::IsFalse(doc.GetRoot().ArrFront().IsValid());
    }
    TEST_METHOD(EmptyArray_ArrBack_Invalid)
    {
        CDocument doc(k_JsonEmptyArray);
        Assert::IsFalse(doc.GetRoot().ArrBack().IsValid());
    }
    TEST_METHOD(EmptyObject_ObjAt_Invalid)
    {
        CDocument doc(k_JsonEmptyObject);
        Assert::IsFalse(doc.GetRoot().ObjAt("any").IsValid());
    }

    // ── CValue::IsValid 对 nullptr ───────────────────
    TEST_METHOD(CValue_NullPointer_IsInvalid)
    {
        CValue v(nullptr);
        Assert::IsFalse(v.IsValid());
    }

    // ── CMutableValue 默认构造 IsValid ───────────────
    TEST_METHOD(CMutableValue_DefaultConstruct_IsInvalid)
    {
        CMutableValue v(nullptr);
        Assert::IsFalse(v.IsValid());
    }

    // ── CDocument 移动后原对象不可用 ─────────────────
    TEST_METHOD(CDocument_AfterMove_NotValid)
    {
        CDocument a(k_JsonNull);
        CDocument b(std::move(a));
        Assert::IsFalse(a.IsValid());
        Assert::IsTrue(b.IsValid());
    }

    // ── 嵌套数组访问 ─────────────────────────────────
    TEST_METHOD(NestedArray_Access)
    {
        CDocument doc(k_JsonNestedArray);  // [[1,2],[3,4]]
        CValue outer = doc.GetRoot();
        Assert::AreEqual((size_t)2, outer.ArrSize());
        CValue inner = outer.ArrAt(1);    // [3,4]
        Assert::AreEqual((size_t)2, inner.ArrSize());
        Assert::AreEqual(4, inner.ArrAt(1).GetInt());
    }

    // ── 复杂文档的多路径访问 ─────────────────────────
    TEST_METHOD(ComplexDoc_MultipleAccess)
    {
        CDocument doc(k_JsonComplex);
        // {"nums":[1,2,3],"flag":false,"sub":{"k":"v"}}
        CValue nums = doc.GetRoot().ObjAt("nums");
        Assert::IsTrue(nums.IsArray());
        Assert::AreEqual((size_t)3, nums.ArrSize());

        CValue flag = doc.GetRoot().ObjAt("flag");
        Assert::IsTrue(flag.IsFalse());

        CValue sub = doc.GetRoot().ObjAt("sub");
        Assert::IsTrue(sub.IsObject());
        CValue kv = sub.ObjAt("k");
        Assert::AreEqual("v", kv.GetString());
    }

    // ── UInt64 边界 ──────────────────────────────────
    TEST_METHOD(NewUInt64_MaxValue_Roundtrip)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewUInt64(UINT64_MAX));
        Assert::AreEqual((uint64_t)UINT64_MAX, doc.GetRoot().GetUInt64());
    }

    // ── Int64 边界 ───────────────────────────────────
    TEST_METHOD(NewInt64_MinValue_Roundtrip)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewInt64(INT64_MIN));
        Assert::AreEqual(INT64_MIN, doc.GetRoot().GetInt64());
    }

    // ── 空字符串 ─────────────────────────────────────
    TEST_METHOD(NewString_Empty_Length0)
    {
        CMutableDocument doc;
        doc.SetRoot(doc.NewString("", 0));
        Assert::IsTrue(doc.GetRoot().IsString());
        Assert::AreEqual((size_t)0, doc.GetRoot().GetLength());
    }

    // ── GetTagInfo ────────────────────────────────────
    TEST_METHOD(GetTag_NonZeroForNumber)
    {
        CDocument doc(k_JsonInt);
        uint8_t tag = doc.GetRoot().GetTag();
        Assert::AreNotEqual((uint8_t)0, tag);
    }

    // ── YyLocateStringPosition ────────────────────────
    TEST_METHOD(LocateStringPosition_SimplePos)
    {
        const char json[] = "[\n  1,\n  2\n]";
        size_t nLine, nCol, nChar;
        BOOL ok = YyLocateStringPosition(json, sizeof(json) - 1, 5, nLine, nCol, nChar);
        Assert::IsTrue(ok);
        Assert::IsTrue(nLine >= 1);
    }

    // ── 迭代器 EqualIterator 的四个分支 ──────────────
    TEST_METHOD(EqualIterator_BothNull_Equal)
    {
        ArrayIterator a, b;
        Assert::IsTrue(a == b);  // both null -> true
    }
    TEST_METHOD(EqualIterator_BothSamePointer_Equal)
    {
        CDocument doc(R"([1])");
        ArrayIterator a(doc.GetRoot());
        ArrayIterator b(doc.GetRoot());
        // 两者 cur 指向同一个 first child pointer
        Assert::IsTrue(a == b);
    }
    TEST_METHOD(EqualIterator_DifferentPointers_NotEqual)
    {
        CDocument doc(k_JsonArray);   // [1,2,3]
        ArrayIterator a(doc.GetRoot());
        ArrayIterator b(doc.GetRoot());
        b.Next();   // b 前进一步
        Assert::IsFalse(a == b);
    }
    TEST_METHOD(EqualIterator_DepletedVsDefault)
    {
        CDocument doc(R"([99])");
        ArrayIterator it(doc.GetRoot());
        it.Next();
        ArrayIterator end;
        Assert::IsTrue(it == end);
    }
};
TS_NS_END