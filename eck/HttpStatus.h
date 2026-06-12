#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
enum class HttpStatus
{
    // 1xx Informational
    Continue = 100,
    SwitchingProtocols = 101,
    Processing = 102,
    EarlyHints = 103,

    // 2xx Success
    Ok = 200,
    Created = 201,
    Accepted = 202,
    NonAuthoritativeInformation = 203,
    NoContent = 204,
    ResetContent = 205,
    PartialContent = 206,
    MultiStatus = 207,
    AlreadyReported = 208,
    ImUsed = 226,

    // 3xx Redirection
    MultipleChoices = 300,
    MovedPermanently = 301,
    Found = 302,
    SeeOther = 303,
    NotModified = 304,
    UseProxy = 305,
    TemporaryRedirect = 307,
    PermanentRedirect = 308,

    // 4xx Client Error
    BadRequest = 400,
    Unauthorized = 401,
    PaymentRequired = 402,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    NotAcceptable = 406,
    ProxyAuthenticationRequired = 407,
    RequestTimeout = 408,
    Conflict = 409,
    Gone = 410,
    LengthRequired = 411,
    PreconditionFailed = 412,
    ContentTooLarge = 413,
    UriTooLong = 414,
    UnsupportedMediaType = 415,
    RangeNotSatisfiable = 416,
    ExpectationFailed = 417,
    ImATeapot = 418,
    MisdirectedRequest = 421,
    UnprocessableContent = 422,
    Locked = 423,
    FailedDependency = 424,
    TooEarly = 425,
    UpgradeRequired = 426,
    PreconditionRequired = 428,
    TooManyRequests = 429,
    RequestHeaderFieldsTooLarge = 431,
    UnavailableForLegalReasons = 451,

    // 5xx Server Error
    InternalServerError = 500,
    NotImplemented = 501,
    BadGateway = 502,
    ServiceUnavailable = 503,
    GatewayTimeout = 504,
    HttpVersionNotSupported = 505,
    VariantAlsoNegotiates = 506,
    InsufficientStorage = 507,
    LoopDetected = 508,
    NotExtended = 510,
    NetworkAuthenticationRequired = 511
};

inline constexpr std::string_view HttpStatusDescription(HttpStatus e) noexcept
{
    switch (e)
    {
    case HttpStatus::Continue:                      return "Continue"sv;
    case HttpStatus::SwitchingProtocols:            return "Switching Protocols"sv;
    case HttpStatus::Processing:                    return "Processing"sv;
    case HttpStatus::EarlyHints:                    return "Early Hints"sv;

    case HttpStatus::Ok:                            return "OK"sv;
    case HttpStatus::Created:                       return "Created"sv;
    case HttpStatus::Accepted:                      return "Accepted"sv;
    case HttpStatus::NonAuthoritativeInformation:   return "Non-Authoritative Information"sv;
    case HttpStatus::NoContent:                     return "No Content"sv;
    case HttpStatus::ResetContent:                  return "Reset Content"sv;
    case HttpStatus::PartialContent:                return "Partial Content"sv;
    case HttpStatus::MultiStatus:                   return "Multi-Status"sv;
    case HttpStatus::AlreadyReported:               return "Already Reported"sv;
    case HttpStatus::ImUsed:                        return "IM Used"sv;

    case HttpStatus::MultipleChoices:               return "Multiple Choices"sv;
    case HttpStatus::MovedPermanently:              return "Moved Permanently"sv;
    case HttpStatus::Found:                         return "Found"sv;
    case HttpStatus::SeeOther:                      return "See Other"sv;
    case HttpStatus::NotModified:                   return "Not Modified"sv;
    case HttpStatus::UseProxy:                      return "Use Proxy"sv;
    case HttpStatus::TemporaryRedirect:             return "Temporary Redirect"sv;
    case HttpStatus::PermanentRedirect:             return "Permanent Redirect"sv;

    case HttpStatus::BadRequest:                    return "Bad Request"sv;
    case HttpStatus::Unauthorized:                  return "Unauthorized"sv;
    case HttpStatus::PaymentRequired:               return "Payment Required"sv;
    case HttpStatus::Forbidden:                     return "Forbidden"sv;
    case HttpStatus::NotFound:                      return "Not Found"sv;
    case HttpStatus::MethodNotAllowed:              return "Method Not Allowed"sv;
    case HttpStatus::NotAcceptable:                 return "Not Acceptable"sv;
    case HttpStatus::ProxyAuthenticationRequired:   return "Proxy Authentication Required"sv;
    case HttpStatus::RequestTimeout:                return "Request Timeout"sv;
    case HttpStatus::Conflict:                      return "Conflict"sv;
    case HttpStatus::Gone:                          return "Gone"sv;
    case HttpStatus::LengthRequired:                return "Length Required"sv;
    case HttpStatus::PreconditionFailed:            return "Precondition Failed"sv;
    case HttpStatus::ContentTooLarge:               return "Content Too Large"sv;
    case HttpStatus::UriTooLong:                    return "URI Too Long"sv;
    case HttpStatus::UnsupportedMediaType:          return "Unsupported Media Type"sv;
    case HttpStatus::RangeNotSatisfiable:           return "Range Not Satisfiable"sv;
    case HttpStatus::ExpectationFailed:             return "Expectation Failed"sv;
    case HttpStatus::ImATeapot:                     return "I'm a Teapot"sv;
    case HttpStatus::MisdirectedRequest:            return "Misdirected Request"sv;
    case HttpStatus::UnprocessableContent:          return "Unprocessable Content"sv;
    case HttpStatus::Locked:                        return "Locked"sv;
    case HttpStatus::FailedDependency:              return "Failed Dependency"sv;
    case HttpStatus::TooEarly:                      return "Too Early"sv;
    case HttpStatus::UpgradeRequired:               return "Upgrade Required"sv;
    case HttpStatus::PreconditionRequired:          return "Precondition Required"sv;
    case HttpStatus::TooManyRequests:               return "Too Many Requests"sv;
    case HttpStatus::RequestHeaderFieldsTooLarge:   return "Request Header Fields Too Large"sv;
    case HttpStatus::UnavailableForLegalReasons:    return "Unavailable For Legal Reasons"sv;

    case HttpStatus::InternalServerError:           return "Internal Server Error"sv;
    case HttpStatus::NotImplemented:                return "Not Implemented"sv;
    case HttpStatus::BadGateway:                    return "Bad Gateway"sv;
    case HttpStatus::ServiceUnavailable:            return "Service Unavailable"sv;
    case HttpStatus::GatewayTimeout:                return "Gateway Timeout"sv;
    case HttpStatus::HttpVersionNotSupported:       return "HTTP Version Not Supported"sv;
    case HttpStatus::VariantAlsoNegotiates:         return "Variant Also Negotiates"sv;
    case HttpStatus::InsufficientStorage:           return "Insufficient Storage"sv;
    case HttpStatus::LoopDetected:                  return "Loop Detected"sv;
    case HttpStatus::NotExtended:                   return "Not Extended"sv;
    case HttpStatus::NetworkAuthenticationRequired: return "Network Authentication Required"sv;

    default:                                        return "Unassigned"sv;
    }
}
ECK_NAMESPACE_END