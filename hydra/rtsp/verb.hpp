//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_HTTP_VERB_HPP
#define BOOST_BEAST_HTTP_VERB_HPP

#include <boost/hydra/core/detail/config.hpp>
#include <boost/hydra/core/string.hpp>
#include <iosfwd>

namespace boost {
namespace hydra {
namespace rtsp {

/** HTTP request method verbs

    Each verb corresponds to a particular method string
    used in HTTP request messages.
*/
enum class verb
{
    unknown = 0,

    describe, 
    announce, 
    get_parameter, 
    set_parameter,
    options, 
    pause, 
    play, 
    record, 
    redirect, 
    setup,
    teardown
};

/** Converts a string to the request method verb.

    If the string does not match a known request method,
    @ref verb::unknown is returned.
*/
BOOST_BEAST_DECL
verb
string_to_verb(string_view s);

/// Returns the text representation of a request method verb.
BOOST_BEAST_DECL
string_view
to_string(verb v);

/// Write the text for a request method verb to an output stream.
inline
std::ostream&
operator<<(std::ostream& os, verb v)
{
    return os << to_string(v);
}

} // http
} // beast
} // boost

#ifdef BOOST_BEAST_HEADER_ONLY
#include <boost/hydra/rtsp/impl/verb.ipp>
#endif

#endif
