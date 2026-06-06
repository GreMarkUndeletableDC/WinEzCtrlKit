#pragma once
#include "StringConvert.h"

#define ECK_SERVER_NAMESPACE_BEGIN  namespace Server {
#define ECK_SERVER_NAMESPACE_END    }

ECK_NAMESPACE_BEGIN
ECK_SERVER_NAMESPACE_BEGIN
using TQueryKv = std::pair<std::string_view, std::string_view>;

std::span<const TQueryKv> ApiParseQueryString(
    std::string_view svQuery,
    std::span<TQueryKv> spResult) noexcept
{
    size_t i = 0, j = 0;
    while (i < svQuery.size())
    {
        const auto posEq = svQuery.find('=', i);
        const auto posAmp = svQuery.find('&', i);
        if (posEq == std::string_view::npos ||
            (posAmp != std::string_view::npos && posAmp < posEq))
            break;
        if (j >= spResult.size())
            break;

        TQueryKv kv{};
        kv.first = svQuery.substr(i, posEq - i);
        if (posAmp == std::string_view::npos)
        {
            kv.second = svQuery.substr(posEq + 1);
            spResult[j++] = kv;
            break;
        }
        else
        {
            kv.second = svQuery.substr(posEq + 1, posAmp - posEq - 1);
            spResult[j++] = kv;
            i = posAmp + 1;
        }
    }
    return spResult.first(j);
}
void ApiParseInt(std::string_view sv, _Inout_ int& i) noexcept
{
    int j;
    if (TcvToInt(sv.data(), sv.size(), j, 10) == TcvResult::Ok)
        i = j;
}

// 检查枚举值v是否在(Min, Max)内
template<CcpNumberOrEnum U>
EckInlineNdCe BOOL ApiEnumerationInRange(auto v, U Min, U Max) noexcept
{
    using TInt = UnderlyingType_T<U>;
    return (TInt)v > (TInt)Min && (TInt)v < (TInt)Max;
}
ECK_SERVER_NAMESPACE_END
ECK_NAMESPACE_END