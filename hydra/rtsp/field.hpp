//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_HTTP_FIELD_HPP
#define BOOST_BEAST_HTTP_FIELD_HPP

#include <boost/hydra/core/detail/config.hpp>
#include <boost/hydra/core/string.hpp>
#include <iosfwd>

namespace boost {
namespace hydra {
namespace rtsp {

    enum class field : unsigned short
    {
        unknown = 0,

        //  general header
        cache_control,
        connection,
        date,
        via,
        cseq,
        transport,
        session,

        //  request header
        accept,
        accept_encoding,
        accept_language,
        authorization,
        from,
        if_modified_since,
        range,
        referer,
        user_agent,
        
        //  response header
        location,
        proxy_authenticate,
        public_,
        retry_after,
        server,
        vary,
        www_authenticate,
        rtp_info,

        //  entity header
        allow,
        content_base,
        content_encoding,
        content_language,
        content_length,
        content_location,
        content_type,
        content_disposition,
        expires,
        last_modified
    };
/** Convert a field enum to a string.

    @param f The field to convert
*/
BOOST_BEAST_DECL
string_view
to_string(field f);

/** Attempt to convert a string to a field enum.

    The string comparison is case-insensitive.

    @return The corresponding field, or @ref field::unknown
    if no known field matches.
*/
BOOST_BEAST_DECL
field
string_to_field(string_view s);

/// Write the text for a field name to an output stream.
BOOST_BEAST_DECL
std::ostream&
operator<<(std::ostream& os, field f);

} // http
} // beast
} // boost

#ifdef BOOST_BEAST_HEADER_ONLY
#include <boost/hydra/rtsp/impl/field.ipp>
#endif

#endif
