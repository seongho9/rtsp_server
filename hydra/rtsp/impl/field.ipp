//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_HTTP_IMPL_FIELD_IPP
#define BOOST_BEAST_HTTP_IMPL_FIELD_IPP

#include <boost/hydra/rtsp/field.hpp>
#include <boost/assert.hpp>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <ostream>


namespace boost {
namespace hydra {
namespace rtsp {

namespace detail {

struct field_table
{
    static
    std::uint32_t
    get_chars(
        unsigned char const* p) noexcept
    {
        // VFALCO memcpy is endian-dependent
        //std::memcpy(&v, p, 4);
        // Compiler should be smart enough to
        // optimize this down to one instruction.
        return
             p[0] |
            (p[1] <<  8) |
            (p[2] << 16) |
            (p[3] << 24);
    }

    using array_type =
        std::array<string_view, 357>;

    // Strings are converted to lowercase
    static
    std::uint32_t
    digest(string_view s)
    {
        std::uint32_t r = 0;
        std::size_t n = s.size();
        auto p = reinterpret_cast<
            unsigned char const*>(s.data());
        // consume N characters at a time
        // VFALCO Can we do 8 on 64-bit systems?
        while(n >= 4)
        {
            auto const v = get_chars(p);
            r = (r * 5 + (
                v | 0x20202020 )); // convert to lower
            p += 4;
            n -= 4;
        }
        // handle remaining characters
        while( n > 0 )
        {
            r = r * 5 + ( *p | 0x20 );
            ++p;
            --n;
        }
        return r;
    }

    // This comparison is case-insensitive, and the
    // strings must contain only valid http field characters.
    static
    bool
    equals(string_view lhs, string_view rhs)
    {
        using Int = std::uint32_t; // VFALCO std::size_t?
        auto n = lhs.size();
        if(n != rhs.size())
            return false;
        auto p1 = reinterpret_cast<
            unsigned char const*>(lhs.data());
        auto p2 = reinterpret_cast<
            unsigned char const*>(rhs.data());
        auto constexpr S = sizeof(Int);
        auto constexpr Mask = static_cast<Int>(
            0xDFDFDFDFDFDFDFDF & ~Int{0});
        for(; n >= S; p1 += S, p2 += S, n -= S)
        {
            Int const v1 = get_chars(p1);
            Int const v2 = get_chars(p2);
            if((v1 ^ v2) & Mask)
                return false;
        }
        for(; n; ++p1, ++p2, --n)
            if(( *p1 ^ *p2) & 0xDF)
                return false;
        return true;
    }

    array_type by_name_;

    enum { N = 5155 };
    unsigned char map_[ N ][ 2 ] = {};

/*
    From:
    
    https://www.iana.org/assignments/message-headers/message-headers.xhtml
*/
    field_table()
        : by_name_({{
           // string constants for RTSP
           "<unknown-field>",
           // general header
           "Cache-Control",
           "Connection",
           "Date",
           "Via",
           "CSeq",
           "Transport",
           "Session"
           // request header
           "Accept",
           "Accept-Encoding",
           "Accept-Language",
           "Authorization",
           "From",
           "If-Modified-Since",
           "Range",
           "Referer",
           "User-Agent",
           // response header
           "Location",
           "Proxy-Authenticate",
           "Public",
           "Retry-After",
           "Server",
           "Vary",
           "WWW-Authenticate",
           "RTP-Info",
           //  entity header
           "Allow",
           "Content-Base",
           "Content-Encoding",
           "Content-Language",
           "Content-Length",
           "Content-Location",
           "Content-Type",
           "Content-Disposition",
           "Expires",
           "Last-Modified"
        }})
    {
        for(std::size_t i = 1, n = 256; i < n; ++i)
        {
            auto sv = by_name_[ i ];
            auto h = digest(sv);
            auto j = h % N;

            if(sv.empty())
                break;
            BOOST_ASSERT(map_[j][0] == 0);
            map_[j][0] = static_cast<unsigned char>(i);
        }
        for(std::size_t i = 256, n = by_name_.size(); i < n; ++i)
        {
            auto sv = by_name_[i];
            
            if(sv.empty())
                break;
            auto h = digest(sv);
            auto j = h % N;
            BOOST_ASSERT(map_[j][1] == 0);
            map_[j][1] = static_cast<unsigned char>(i - 255);
        }
    }

    field
    string_to_field(string_view s) const
    {
        auto h = digest(s);
        auto j = h % N;
        int i = map_[j][0];
        string_view s2 = by_name_[i];
        if(i != 0 && equals(s, s2))
            return static_cast<field>(i);
        i = map_[j][1];
        if(i == 0)
            return field::unknown;
        i += 255;
        s2 = by_name_[i];

        if(equals(s, s2))
            return static_cast<field>(i);
        return field::unknown;
    }

    //
    // Deprecated
    //

    using const_iterator =
    array_type::const_iterator; 

    std::size_t
    size() const
    {
        return by_name_.size();
    }

    const_iterator
    begin() const
    {
        return by_name_.begin();
    }

    const_iterator
    end() const
    {
        return by_name_.end();
    }
};

BOOST_BEAST_DECL
field_table const&
get_field_table()
{
    static field_table const tab;
    return tab;
}

BOOST_BEAST_DECL
string_view
to_string(field f)
{
    auto const& v = get_field_table();
    BOOST_ASSERT(static_cast<unsigned>(f) < v.size());
    return v.begin()[static_cast<unsigned>(f)];
}

} // detail

string_view
to_string(field f)
{
    return detail::to_string(f);
}

field
string_to_field(string_view s)
{
    return detail::get_field_table().string_to_field(s);
}

std::ostream&
operator<<(std::ostream& os, field f)
{
    return os << to_string(f);
}

} // http
} // beast
} // boost

#endif
