// Copyright Vladimir Prus 2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "platform_config.hpp"

#include <boost/filesystem/config.hpp>

#include <boost/filesystem/detail/header.hpp>

#define BOOST_UTF8_BEGIN_NAMESPACE \
    namespace boost { \
    namespace filesystem { \
    namespace detail {

#define BOOST_UTF8_END_NAMESPACE \
    } \
    } \
    }
#define BOOST_UTF8_DECL BOOST_FILESYSTEM_DECL

#include <boost/detail/utf8_codecvt_facet.ipp>

#undef BOOST_UTF8_BEGIN_NAMESPACE
#undef BOOST_UTF8_END_NAMESPACE
#undef BOOST_UTF8_DECL

#include <boost/filesystem/detail/footer.hpp>
