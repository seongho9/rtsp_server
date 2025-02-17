//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_UNIT_TEST_SUITE_LIST_HPP
#define BOOST_BEAST_UNIT_TEST_SUITE_LIST_HPP

#include <boost/hydra/_experimental/unit_test/suite_info.hpp>
#include <boost/hydra/_experimental/unit_test/detail/const_container.hpp>
#include <boost/assert.hpp>
#include <boost/type_index.hpp>
#include <boost/functional/hash.hpp>
#include <typeindex>
#include <set>
#include <unordered_set>

namespace boost {
namespace hydra {
namespace unit_test {

/// A container of test suites.
class suite_list
    : public detail::const_container <std::set <suite_info>>
{
private:
#ifndef NDEBUG
    std::unordered_set<std::string> names_;

    using type_index = boost::typeindex::type_index;
    std::unordered_set<type_index, boost::hash<type_index>> classes_;
#endif

public:
    /** Insert a suite into the set.

        The suite must not already exist.
    */
    template<class Suite>
    void
    insert(
        char const* name,
        char const* module,
        char const* library,
        bool manual);
};

//------------------------------------------------------------------------------

template<class Suite>
void
suite_list::insert(
    char const* name,
    char const* module,
    char const* library,
    bool manual)
{
#ifndef NDEBUG
    {
        std::string s;
        s = std::string(library) + "." + module + "." + name;
        auto const result(names_.insert(s));
        BOOST_ASSERT(result.second); // Duplicate name
    }

    {
        auto const result(classes_.insert(
            boost::typeindex::type_id<Suite>()));
        BOOST_ASSERT(result.second); // Duplicate type
    }
#endif
    cont().emplace(make_suite_info<Suite>(
        name, module, library, manual));
}

} // unit_test
} // beast
} // boost

#endif

