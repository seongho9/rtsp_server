//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_SRC_HPP
#define BOOST_BEAST_SRC_HPP

/*

This file is meant to be included once, in a translation unit of
the program, with the macro BOOST_BEAST_SEPARATE_COMPILATION defined.

*/

#define BOOST_BEAST_SOURCE

#include <boost/hydra/core/detail/config.hpp>

#if defined(BOOST_BEAST_HEADER_ONLY)
# error Do not compile Beast library source with BOOST_BEAST_HEADER_ONLY defined
#endif

#include <boost/hydra/_experimental/test/impl/error.ipp>
#include <boost/hydra/_experimental/test/impl/fail_count.ipp>
#include <boost/hydra/_experimental/test/impl/stream.ipp>
#include <boost/hydra/_experimental/test/detail/stream_state.ipp>

#include <boost/hydra/core/detail/base64.ipp>
#include <boost/hydra/core/detail/sha1.ipp>
#include <boost/hydra/core/detail/impl/temporary_buffer.ipp>
#include <boost/hydra/core/impl/error.ipp>
#include <boost/hydra/core/impl/file_posix.ipp>
#include <boost/hydra/core/impl/file_stdio.ipp>
#include <boost/hydra/core/impl/file_win32.ipp>
#include <boost/hydra/core/impl/flat_static_buffer.ipp>
#include <boost/hydra/core/impl/saved_handler.ipp>
#include <boost/hydra/core/impl/static_buffer.ipp>
#include <boost/hydra/core/impl/string.ipp>

#include <boost/hydra/rtsp/detail/basic_parser.ipp>
#include <boost/hydra/rtsp/detail/rfc7230.ipp>
#include <boost/hydra/rtsp/impl/basic_parser.ipp>
#include <boost/hydra/rtsp/impl/error.ipp>
#include <boost/hydra/rtsp/impl/field.ipp>
#include <boost/hydra/rtsp/impl/fields.ipp>
#include <boost/hydra/rtsp/impl/rfc7230.ipp>
#include <boost/hydra/rtsp/impl/status.ipp>
#include <boost/hydra/rtsp/impl/verb.ipp>

#include <boost/hydra/zlib/detail/deflate_stream.ipp>
#include <boost/hydra/zlib/detail/inflate_stream.ipp>
#include <boost/hydra/zlib/impl/error.ipp>

#endif
