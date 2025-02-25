//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_HTTP_IMPL_STATUS_IPP
#define BOOST_BEAST_HTTP_IMPL_STATUS_IPP

#include <boost/hydra/rtsp/status.hpp>
#include <boost/throw_exception.hpp>

namespace boost {
namespace hydra {
namespace rtsp {

status
int_to_status(unsigned v)
{
    switch(static_cast<status>(v))
    {
    // 1xx
    case status::continue_:
        BOOST_FALLTHROUGH;

    // 2xx
    case status::ok:
    case status::created:
    case status::low_on_storage_space:
        BOOST_FALLTHROUGH;

    // 3xx
    case status::multiple_choices:
    case status::moved_permanently:
    case status::moved_temporarily:
    case status::see_other:
    case status::not_modified:
    case status::use_proxy:
        BOOST_FALLTHROUGH;

    // 4xx
    case status::bad_request:
    case status::unauthorized:
    case status::payment_required:
    case status::forbidden:
    case status::not_found:
    case status::method_not_allowed:
    case status::not_acceptable:
    case status::proxy_authentication_required:
    case status::request_timeout:
    case status::conflict:
    case status::gone:
    case status::length_required:
    case status::precondition_failed:
    case status::payload_too_large:
    case status::uri_too_long:
    case status::unsupported_media_type:
    // 4xx for RTSP
    case status::invalid_parameter:
    case status::invalid_conference_identifier:
    case status::not_enough_bandwidth:
    case status::session_not_found:
    case status::method_not_valid_in_this_state:
    case status::header_field_not_valid:
    case status::invalid_range:
    case status::parameter_is_read_only:
    case status::aggregate_operation_not_allowed:
    case status::only_aggregate_operation_allowed:
    case status::unsupported_transport:
    case status::destination_unreachable:
        BOOST_FALLTHROUGH;

    // 5xx
    case status::internal_server_error:
    case status::not_implemented:
    case status::bad_gateway:
    case status::service_unavailable:
    case status::gateway_timeout:
    case status::rtsp_version_not_supported:
    case status::option_not_support:

        return static_cast<status>(v);

    default:
        break;
    }
    return status::unknown;
}

status_class
to_status_class(unsigned v)
{
    switch(v / 100)
    {
    case 1: return status_class::informational;
    case 2: return status_class::successful;
    case 3: return status_class::redirection;
    case 4: return status_class::client_error;
    case 5: return status_class::server_error;
    default:
        break;
    }
    return status_class::unknown;
}

status_class
to_status_class(status v)
{
    return to_status_class(static_cast<int>(v));
}

string_view
obsolete_reason(status v)
{
    switch(static_cast<status>(v))
    {
    // 1xx
    case status::continue_:                             return "Continue";

    // 2xx
    case status::ok:                                    return "OK";
    case status::created:                               return "Created";
    case status::low_on_storage_space:                  return "Low On Storage Space";

    // 3xx
    case status::multiple_choices:                      return "Multiple Choices";
    case status::moved_permanently:                     return "Moved Permanently";
    case status::moved_temporarily:                     return "Moved Temproarily";
    case status::see_other:                             return "See Other";
    case status::not_modified:                          return "Not Modified";
    case status::use_proxy:                             return "Use Proxy";

    // 4xx
    case status::bad_request:                           return "Bad Request";
    case status::unauthorized:                          return "Unauthorized";
    case status::payment_required:                      return "Payment Required";
    case status::forbidden:                             return "Forbidden";
    case status::not_found:                             return "Not Found";
    case status::method_not_allowed:                    return "Method Not Allowed";
    case status::not_acceptable:                        return "Not Acceptable";
    case status::proxy_authentication_required:         return "Proxy Authentication Required";
    case status::request_timeout:                       return "Request Timeout";
    case status::conflict:                              return "Conflict";
    case status::gone:                                  return "Gone";
    case status::length_required:                       return "Length Required";
    case status::precondition_failed:                   return "Precondition Failed";
    case status::payload_too_large:                     return "Payload Too Large";
    case status::uri_too_long:                          return "URI Too Long";
    case status::unsupported_media_type:                return "Unsupported Media Type";
    // 4xx for RTSP
    case status::invalid_parameter:                     return "Invalid Parameter";
    case status::invalid_conference_identifier:         return "Invalid Conference_Identifier";
    case status::not_enough_bandwidth:                  return "Not Enough Bandwidth";
    case status::session_not_found:                     return "Session Not Found";
    case status::method_not_valid_in_this_state:        return "Method Not Valid In This State";
    case status::header_field_not_valid:                return "Header Field Not Valid";
    case status::invalid_range:                         return "Invalid Range";
    case status::parameter_is_read_only:                return "Parameter Is Read-Only";
    case status::aggregate_operation_not_allowed:       return "Aggregate Operation Not Allowed";
    case status::only_aggregate_operation_allowed:      return "Only Aggregate Operation Allowed";
    case status::unsupported_transport:                 return "Unsupported Transport";
    case status::destination_unreachable:               return "Destinatino Unreachable";

    // 5xx
    case status::internal_server_error:                 return "Internal Server Error";
    case status::not_implemented:                       return "Not Implemented";
    case status::bad_gateway:                           return "Bad Gateway";
    case status::service_unavailable:                   return "Service Unavailable";
    case status::gateway_timeout:                       return "Gateway Timeout";
    // 5xx for RTSP
    case status::rtsp_version_not_supported:            return "RTSP Version Not Supported";
    case status::option_not_support:                    return "Option Not Support";

    default:
        break;
    }
    return "<unknown-status>";
}

std::ostream&
operator<<(std::ostream& os, status v)
{
    return os << obsolete_reason(v);
}

} // http
} // beast
} // boost

#endif
