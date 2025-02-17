//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_HTTP_IMPL_VERB_IPP
#define BOOST_BEAST_HTTP_IMPL_VERB_IPP

#include <boost/hydra/rtsp/verb.hpp>
#include <boost/throw_exception.hpp>
#include <stdexcept>

namespace boost {
namespace hydra {
namespace rtsp {

string_view
to_string(verb v)
{
    using namespace hydra::detail::string_literals;
    switch(v)
    {

    case verb::describe:            return "DESCRIBE"_sv;
    case verb::announce:            return "ANNOUNCE"_sv;
    case verb::get_parameter:       return "GET_PARAMETER"_sv;
    case verb::set_parameter:       return "SET_PARAMETER"_sv;
    case verb::options:             return "OPTIONS"_sv;
    case verb::pause:               return "PAUSE"_sv;
    case verb::play:                return "PLAY"_sv;
    case verb::record:              return "RECORD"_sv;
    case verb::redirect:            return "REDIRECT"_sv;
    case verb::setup:               return "SETUP"_sv;
    case verb::teardown:            return "TEARDOWN"_sv;

    case verb::unknown:
        return "<unknown>"_sv;
    }

    BOOST_THROW_EXCEPTION(std::invalid_argument{"unknown verb"});
}

verb
string_to_verb(string_view v)
{
    using namespace hydra::detail::string_literals;

    if(v.size() < 3)
        return verb::unknown;
    auto c = v[0];
    v.remove_prefix(1);

    switch(c)
    {
    case 'A':
        if(v == "NNOUNCE"_sv)
            return verb::announce;

        break;

    // case 'B':
    //     break;

    // case 'C':
    //     break;

    case 'D':
        if(v == "ESCRIBE"_sv)
            return verb::describe;

        break;

    // case 'E':
    //     break;

    // case 'F':
    //     break;

    case 'G':
        if(v == "ET_PARAMETER"_sv)
            return verb::get_parameter;

        break;

    // case 'H':
    //     break;

    // case 'I':
    //     break;

    // case 'J':
    //     break;

    // case 'K':
    //     break;

    // case 'L':
    //     break;

    // case 'M':
    //     break;

    // case 'N':
    //     break;

    case 'O':
        if(v == "PTIONS"_sv)
            return verb::options;

        break;

    case 'P':
        if(v == "LAY"_sv)
            return verb::play;

        if(v == "AUSE"_sv)
            return verb::pause;

        break;
    
    // case 'Q':
    //     break;

    case 'R':
        if(v == "ECORD"_sv)
            return verb::record;

        if(v == "EDIRECT"_sv)
            return verb::redirect;

        break;

    case 'S':
        if(v == "ETUP")
            return verb::setup;
            
        if(v == "ET_PARAMETER")
            return verb::set_parameter;

        break;

    case 'T':
        if(v == "EARDOWN"_sv)
            return verb::teardown;

        break;

    // case 'U':
    //     break;

    default:
        break;
    }

    return verb::unknown;
}

} // http
} // beast
} // boost

#endif
