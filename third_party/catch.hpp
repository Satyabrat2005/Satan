/*
 *  Catch v2.13.10
 *  Generated: 2022-10-16 11:01:23.452308
 *  ----------------------------------------------------------
 *  This file has been merged from multiple headers. Please don't edit it directly
 *  Copyright (c) 2022 Two Blue Cubes Ltd. All rights reserved.
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef TWOBLUECUBES_SINGLE_INCLUDE_CATCH_HPP_INCLUDED
#define TWOBLUECUBES_SINGLE_INCLUDE_CATCH_HPP_INCLUDED
// start catch.hpp


#define CATCH_VERSION_MAJOR 2
#define CATCH_VERSION_MINOR 13
#define CATCH_VERSION_PATCH 10

#ifdef __clang__
#    pragma clang system_header
#elif defined __GNUC__
#    pragma GCC system_header
#endif

// start catch_suppress_warnings.h

#ifdef __clang__
#   ifdef __ICC // icpc defines the __clang__ macro
#       pragma warning(push)
#       pragma warning(disable: 161 1682)
#   else // __ICC
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wpadded"
#       pragma clang diagnostic ignored "-Wswitch-enum"
#       pragma clang diagnostic ignored "-Wcovered-switch-default"
#    endif
#elif defined __GNUC__
     // Because REQUIREs trigger GCC's -Wparentheses, and because still
     // supported version of g++ have only buggy support for _Pragmas,
     // Wparentheses have to be suppressed globally.
#    pragma GCC diagnostic ignored "-Wparentheses" // See #674 for details

#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wunused-variable"
#    pragma GCC diagnostic ignored "-Wpadded"
#endif
// end catch_suppress_warnings.h
#if defined(CATCH_CONFIG_MAIN) || defined(CATCH_CONFIG_RUNNER)
#  define CATCH_IMPL
#  define CATCH_CONFIG_ALL_PARTS
#endif
