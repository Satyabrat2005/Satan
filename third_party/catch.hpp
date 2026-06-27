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

// In the impl file, we want to have access to all parts of the headers
// Can also be used to sanely support PCHs
#if defined(CATCH_CONFIG_ALL_PARTS)
#  define CATCH_CONFIG_EXTERNAL_INTERFACES
#  if defined(CATCH_CONFIG_DISABLE_MATCHERS)
#    undef CATCH_CONFIG_DISABLE_MATCHERS
#  endif
#  if !defined(CATCH_CONFIG_ENABLE_CHRONO_STRINGMAKER)
#    define CATCH_CONFIG_ENABLE_CHRONO_STRINGMAKER
#  endif
#endif

#if !defined(CATCH_CONFIG_IMPL_ONLY)
// start catch_platform.h

// See e.g.:
// https://opensource.apple.com/source/CarbonHeaders/CarbonHeaders-18.1/TargetConditionals.h.auto.html
#ifdef __APPLE__
#  include <TargetConditionals.h>
#  if (defined(TARGET_OS_OSX) && TARGET_OS_OSX == 1) || \
      (defined(TARGET_OS_MAC) && TARGET_OS_MAC == 1)
#    define CATCH_PLATFORM_MAC
#  elif (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE == 1)
#    define CATCH_PLATFORM_IPHONE
#  endif

#elif defined(linux) || defined(__linux) || defined(__linux__)
#  define CATCH_PLATFORM_LINUX

#elif defined(WIN32) || defined(__WIN32__) || defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
#  define CATCH_PLATFORM_WINDOWS
#endif
// end catch_platform.h

#ifdef CATCH_IMPL
#  ifndef CLARA_CONFIG_MAIN
#    define CLARA_CONFIG_MAIN_NOT_DEFINED
#    define CLARA_CONFIG_MAIN
#  endif
#endif

// start catch_user_interfaces.h

namespace Catch {
    unsigned int rngSeed();
}

// end catch_user_interfaces.h
// start catch_tag_alias_autoregistrar.h

// start catch_common.h

// start catch_compiler_capabilities.h

// Detect a number of compiler features - by compiler
// The following features are defined:
//
// CATCH_CONFIG_COUNTER : is the __COUNTER__ macro supported?
// CATCH_CONFIG_WINDOWS_SEH : is Windows SEH supported?
// CATCH_CONFIG_POSIX_SIGNALS : are POSIX signals supported?
// CATCH_CONFIG_DISABLE_EXCEPTIONS : Are exceptions enabled?
// ****************
// Note to maintainers: if new toggles are added please document them
// in configuration.md, too
// ****************

// In general each macro has a _NO_<feature name> form
// (e.g. CATCH_CONFIG_NO_POSIX_SIGNALS) which disables the feature.
// Many features, at point of detection, define an _INTERNAL_ macro, so they
// can be combined, en-mass, with the _NO_ forms later.

#ifdef __cplusplus

#  if (__cplusplus >= 201402L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 201402L)
#    define CATCH_CPP14_OR_GREATER
#  endif

#  if (__cplusplus >= 201703L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
#    define CATCH_CPP17_OR_GREATER
#  endif

#endif
// Only GCC compiler should be used in this block, so other compilers trying to
// mask themselves as GCC should be ignored.
#if defined(__GNUC__) && !defined(__clang__) && !defined(__ICC) && !defined(__CUDACC__) && !defined(__LCC__)
#    define CATCH_INTERNAL_START_WARNINGS_SUPPRESSION _Pragma( "GCC diagnostic push" )
#    define CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION  _Pragma( "GCC diagnostic pop" )

#    define CATCH_INTERNAL_IGNORE_BUT_WARN(...) (void)__builtin_constant_p(__VA_ARGS__)

#endif

#if defined(__clang__)

#    define CATCH_INTERNAL_START_WARNINGS_SUPPRESSION _Pragma( "clang diagnostic push" )
#    define CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION  _Pragma( "clang diagnostic pop" )

// As of this writing, IBM XL's implementation of __builtin_constant_p has a bug
// which results in calls to destructors being emitted for each temporary,
// without a matching initialization. In practice, this can result in something
// like `std::string::~string` being called on an uninitialized value.
//
// For example, this code will likely segfault under IBM XL:
// ```
// REQUIRE(std::string("12") + "34" == "1234")
// ```
//
// Therefore, `CATCH_INTERNAL_IGNORE_BUT_WARN` is not implemented.
#  if !defined(__ibmxl__) && !defined(__CUDACC__)
#    define CATCH_INTERNAL_IGNORE_BUT_WARN(...) (void)__builtin_constant_p(__VA_ARGS__) /* NOLINT(cppcoreguidelines-pro-type-vararg, hicpp-vararg) */
#  endif
#    define CATCH_INTERNAL_SUPPRESS_GLOBALS_WARNINGS \
         _Pragma( "clang diagnostic ignored \"-Wexit-time-destructors\"" ) \
         _Pragma( "clang diagnostic ignored \"-Wglobal-constructors\"")

#    define CATCH_INTERNAL_SUPPRESS_PARENTHESES_WARNINGS \
         _Pragma( "clang diagnostic ignored \"-Wparentheses\"" )

#    define CATCH_INTERNAL_SUPPRESS_UNUSED_WARNINGS \
         _Pragma( "clang diagnostic ignored \"-Wunused-variable\"" )

#    define CATCH_INTERNAL_SUPPRESS_ZERO_VARIADIC_WARNINGS \
         _Pragma( "clang diagnostic ignored \"-Wgnu-zero-variadic-macro-arguments\"" )

#    define CATCH_INTERNAL_SUPPRESS_UNUSED_TEMPLATE_WARNINGS \
         _Pragma( "clang diagnostic ignored \"-Wunused-template\"" )

#endif // __clang__

////////////////////////////////////////////////////////////////////////////////
// Assume that non-Windows platforms support posix signals by default
#if !defined(CATCH_PLATFORM_WINDOWS)
    #define CATCH_INTERNAL_CONFIG_POSIX_SIGNALS
#endif

////////////////////////////////////////////////////////////////////////////////
// We know some environments not to support full POSIX signals
#if defined(__CYGWIN__) || defined(__QNX__) || defined(__EMSCRIPTEN__) || defined(__DJGPP__)
    #define CATCH_INTERNAL_CONFIG_NO_POSIX_SIGNALS
#endif

#ifdef __OS400__
#       define CATCH_INTERNAL_CONFIG_NO_POSIX_SIGNALS
#       define CATCH_CONFIG_COLOUR_NONE
#endif

////////////////////////////////////////////////////////////////////////////////
// Android somehow still does not support std::to_string
#if defined(__ANDROID__)
#    define CATCH_INTERNAL_CONFIG_NO_CPP11_TO_STRING
#    define CATCH_INTERNAL_CONFIG_ANDROID_LOGWRITE
#endif

////////////////////////////////////////////////////////////////////////////////
// Not all Windows environments support SEH properly
#if defined(__MINGW32__)
#    define CATCH_INTERNAL_CONFIG_NO_WINDOWS_SEH
#endif
////////////////////////////////////////////////////////////////////////////////
// PS4
#if defined(__ORBIS__)
#    define CATCH_INTERNAL_CONFIG_NO_NEW_CAPTURE
#endif

////////////////////////////////////////////////////////////////////////////////
// Cygwin
#ifdef __CYGWIN__

// Required for some versions of Cygwin to declare gettimeofday
// see: http://stackoverflow.com/questions/36901803/gettimeofday-not-declared-in-this-scope-cygwin
#   define _BSD_SOURCE
// some versions of cygwin (most) do not support std::to_string. Use the libstd check.
// https://gcc.gnu.org/onlinedocs/gcc-4.8.2/libstdc++/api/a01053_source.html line 2812-2813
# if !((__cplusplus >= 201103L) && defined(_GLIBCXX_USE_C99) \
           && !defined(_GLIBCXX_HAVE_BROKEN_VSWPRINTF))

#    define CATCH_INTERNAL_CONFIG_NO_CPP11_TO_STRING

# endif
#endif // __CYGWIN__
////////////////////////////////////////////////////////////////////////////////
// Visual C++
#if defined(_MSC_VER)

// Universal Windows platform does not support SEH
// Or console colours (or console at all...)
#  if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#    define CATCH_CONFIG_COLOUR_NONE
#  else
#    define CATCH_INTERNAL_CONFIG_WINDOWS_SEH
#  endif

#  if !defined(__clang__) // Handle Clang masquerading for msvc

// MSVC traditional preprocessor needs some workaround for __VA_ARGS__
// _MSVC_TRADITIONAL == 0 means new conformant preprocessor
// _MSVC_TRADITIONAL == 1 means old traditional non-conformant preprocessor
#    if !defined(_MSVC_TRADITIONAL) || (defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL)
#      define CATCH_INTERNAL_CONFIG_TRADITIONAL_MSVC_PREPROCESSOR
#    endif // MSVC_TRADITIONAL

// Only do this if we're not using clang on Windows, which uses `diagnostic push` & `diagnostic pop`
#    define CATCH_INTERNAL_START_WARNINGS_SUPPRESSION __pragma( warning(push) )
#    define CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION  __pragma( warning(pop) )
#  endif // __clang__

#endif // _MSC_VER

#if defined(_REENTRANT) || defined(_MSC_VER)
// Enable async processing, as -pthread is specified or no additional linking is required
# define CATCH_INTERNAL_CONFIG_USE_ASYNC
#endif // _MSC_VER

////////////////////////////////////////////////////////////////////////////////
// Check if we are compiled with -fno-exceptions or equivalent
#if defined(__EXCEPTIONS) || defined(__cpp_exceptions) || defined(_CPPUNWIND)
#  define CATCH_INTERNAL_CONFIG_EXCEPTIONS_ENABLED
#endif
////////////////////////////////////////////////////////////////////////////////
// DJGPP
#ifdef __DJGPP__
#  define CATCH_INTERNAL_CONFIG_NO_WCHAR
#endif // __DJGPP__

////////////////////////////////////////////////////////////////////////////////
// Embarcadero C++Build
#if defined(__BORLANDC__)
    #define CATCH_INTERNAL_CONFIG_POLYFILL_ISNAN
#endif

////////////////////////////////////////////////////////////////////////////////

// Use of __COUNTER__ is suppressed during code analysis in
// CLion/AppCode 2017.2.x and former, because __COUNTER__ is not properly
// handled by it.
// Otherwise all supported compilers support COUNTER macro,
// but user still might want to turn it off
#if ( !defined(__JETBRAINS_IDE__) || __JETBRAINS_IDE__ >= 20170300L )
    #define CATCH_INTERNAL_CONFIG_COUNTER
#endif

////////////////////////////////////////////////////////////////////////////////

// RTX is a special version of Windows that is real time.
// This means that it is detected as Windows, but does not provide
// the same set of capabilities as real Windows does.
#if defined(UNDER_RTSS) || defined(RTX64_BUILD)
    #define CATCH_INTERNAL_CONFIG_NO_WINDOWS_SEH
    #define CATCH_INTERNAL_CONFIG_NO_ASYNC
    #define CATCH_CONFIG_COLOUR_NONE
#endif

#if !defined(_GLIBCXX_USE_C99_MATH_TR1)
#define CATCH_INTERNAL_CONFIG_GLOBAL_NEXTAFTER
#endif
// Various stdlib support checks that require __has_include
#if defined(__has_include)
  // Check if string_view is available and usable
  #if __has_include(<string_view>) && defined(CATCH_CPP17_OR_GREATER)
  #    define CATCH_INTERNAL_CONFIG_CPP17_STRING_VIEW
  #endif

  // Check if optional is available and usable
  #  if __has_include(<optional>) && defined(CATCH_CPP17_OR_GREATER)
  #    define CATCH_INTERNAL_CONFIG_CPP17_OPTIONAL
  #  endif // __has_include(<optional>) && defined(CATCH_CPP17_OR_GREATER)

  // Check if byte is available and usable
  #  if __has_include(<cstddef>) && defined(CATCH_CPP17_OR_GREATER)
  #    include <cstddef>
  #    if defined(__cpp_lib_byte) && (__cpp_lib_byte > 0)
  #      define CATCH_INTERNAL_CONFIG_CPP17_BYTE
  #    endif
  #  endif // __has_include(<cstddef>) && defined(CATCH_CPP17_OR_GREATER)

  // Check if variant is available and usable
  #  if __has_include(<variant>) && defined(CATCH_CPP17_OR_GREATER)
  #    if defined(__clang__) && (__clang_major__ < 8)
         // work around clang bug with libstdc++ https://bugs.llvm.org/show_bug.cgi?id=31852
         // fix should be in clang 8, workaround in libstdc++ 8.2
  #      include <ciso646>
  #      if defined(__GLIBCXX__) && defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE < 9)
  #        define CATCH_CONFIG_NO_CPP17_VARIANT
  #      else
  #        define CATCH_INTERNAL_CONFIG_CPP17_VARIANT
  #      endif // defined(__GLIBCXX__) && defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE < 9)
  #    else
  #      define CATCH_INTERNAL_CONFIG_CPP17_VARIANT
  #    endif // defined(__clang__) && (__clang_major__ < 8)
  #  endif // __has_include(<variant>) && defined(CATCH_CPP17_OR_GREATER)
#endif // defined(__has_include)
#if defined(CATCH_INTERNAL_CONFIG_COUNTER) && !defined(CATCH_CONFIG_NO_COUNTER) && !defined(CATCH_CONFIG_COUNTER)
#   define CATCH_CONFIG_COUNTER
#endif
#if defined(CATCH_INTERNAL_CONFIG_WINDOWS_SEH) && !defined(CATCH_CONFIG_NO_WINDOWS_SEH) && !defined(CATCH_CONFIG_WINDOWS_SEH) && !defined(CATCH_INTERNAL_CONFIG_NO_WINDOWS_SEH)
#   define CATCH_CONFIG_WINDOWS_SEH
#endif
// This is set by default, because we assume that unix compilers are posix-signal-compatible by default.
#if defined(CATCH_INTERNAL_CONFIG_POSIX_SIGNALS) && !defined(CATCH_INTERNAL_CONFIG_NO_POSIX_SIGNALS) && !defined(CATCH_CONFIG_NO_POSIX_SIGNALS) && !defined(CATCH_CONFIG_POSIX_SIGNALS)
#   define CATCH_CONFIG_POSIX_SIGNALS
#endif
// This is set by default, because we assume that compilers with no wchar_t support are just rare exceptions.
#if !defined(CATCH_INTERNAL_CONFIG_NO_WCHAR) && !defined(CATCH_CONFIG_NO_WCHAR) && !defined(CATCH_CONFIG_WCHAR)
#   define CATCH_CONFIG_WCHAR
#endif

#if !defined(CATCH_INTERNAL_CONFIG_NO_CPP11_TO_STRING) && !defined(CATCH_CONFIG_NO_CPP11_TO_STRING) && !defined(CATCH_CONFIG_CPP11_TO_STRING)
#    define CATCH_CONFIG_CPP11_TO_STRING
#endif

#if defined(CATCH_INTERNAL_CONFIG_CPP17_OPTIONAL) && !defined(CATCH_CONFIG_NO_CPP17_OPTIONAL) && !defined(CATCH_CONFIG_CPP17_OPTIONAL)
#  define CATCH_CONFIG_CPP17_OPTIONAL
#endif

#if defined(CATCH_INTERNAL_CONFIG_CPP17_STRING_VIEW) && !defined(CATCH_CONFIG_NO_CPP17_STRING_VIEW) && !defined(CATCH_CONFIG_CPP17_STRING_VIEW)
#  define CATCH_CONFIG_CPP17_STRING_VIEW
#endif

#if defined(CATCH_INTERNAL_CONFIG_CPP17_VARIANT) && !defined(CATCH_CONFIG_NO_CPP17_VARIANT) && !defined(CATCH_CONFIG_CPP17_VARIANT)
#  define CATCH_CONFIG_CPP17_VARIANT
#endif

#if defined(CATCH_INTERNAL_CONFIG_CPP17_BYTE) && !defined(CATCH_CONFIG_NO_CPP17_BYTE) && !defined(CATCH_CONFIG_CPP17_BYTE)
#  define CATCH_CONFIG_CPP17_BYTE
#endif

#if defined(CATCH_CONFIG_EXPERIMENTAL_REDIRECT)
#  define CATCH_INTERNAL_CONFIG_NEW_CAPTURE
#endif
#if defined(CATCH_INTERNAL_CONFIG_NEW_CAPTURE) && !defined(CATCH_INTERNAL_CONFIG_NO_NEW_CAPTURE) && !defined(CATCH_CONFIG_NO_NEW_CAPTURE) && !defined(CATCH_CONFIG_NEW_CAPTURE)
#  define CATCH_CONFIG_NEW_CAPTURE
#endif

#if !defined(CATCH_INTERNAL_CONFIG_EXCEPTIONS_ENABLED) && !defined(CATCH_CONFIG_DISABLE_EXCEPTIONS)
#  define CATCH_CONFIG_DISABLE_EXCEPTIONS
#endif

#if defined(CATCH_INTERNAL_CONFIG_POLYFILL_ISNAN) && !defined(CATCH_CONFIG_NO_POLYFILL_ISNAN) && !defined(CATCH_CONFIG_POLYFILL_ISNAN)
#  define CATCH_CONFIG_POLYFILL_ISNAN
#endif

#if defined(CATCH_INTERNAL_CONFIG_USE_ASYNC)  && !defined(CATCH_INTERNAL_CONFIG_NO_ASYNC) && !defined(CATCH_CONFIG_NO_USE_ASYNC) && !defined(CATCH_CONFIG_USE_ASYNC)
#  define CATCH_CONFIG_USE_ASYNC
#endif

#if defined(CATCH_INTERNAL_CONFIG_ANDROID_LOGWRITE) && !defined(CATCH_CONFIG_NO_ANDROID_LOGWRITE) && !defined(CATCH_CONFIG_ANDROID_LOGWRITE)
#  define CATCH_CONFIG_ANDROID_LOGWRITE
#endif

#if defined(CATCH_INTERNAL_CONFIG_GLOBAL_NEXTAFTER) && !defined(CATCH_CONFIG_NO_GLOBAL_NEXTAFTER) && !defined(CATCH_CONFIG_GLOBAL_NEXTAFTER)
#  define CATCH_CONFIG_GLOBAL_NEXTAFTER
#endif

// Even if we do not think the compiler has that warning, we still have
// to provide a macro that can be used by the code.
#if !defined(CATCH_INTERNAL_START_WARNINGS_SUPPRESSION)
#   define CATCH_INTERNAL_START_WARNINGS_SUPPRESSION
#endif
#if !defined(CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION)
#   define CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION
#endif
#if !defined(CATCH_INTERNAL_SUPPRESS_PARENTHESES_WARNINGS)
#   define CATCH_INTERNAL_SUPPRESS_PARENTHESES_WARNINGS
#endif
#if !defined(CATCH_INTERNAL_SUPPRESS_GLOBALS_WARNINGS)
#   define CATCH_INTERNAL_SUPPRESS_GLOBALS_WARNINGS
#endif
#if !defined(CATCH_INTERNAL_SUPPRESS_UNUSED_WARNINGS)
#   define CATCH_INTERNAL_SUPPRESS_UNUSED_WARNINGS
#endif
#if !defined(CATCH_INTERNAL_SUPPRESS_ZERO_VARIADIC_WARNINGS)
#   define CATCH_INTERNAL_SUPPRESS_ZERO_VARIADIC_WARNINGS
#endif
// The goal of this macro is to avoid evaluation of the arguments, but
// still have the compiler warn on problems inside...
#if !defined(CATCH_INTERNAL_IGNORE_BUT_WARN)
#   define CATCH_INTERNAL_IGNORE_BUT_WARN(...)
#endif

#if defined(__APPLE__) && defined(__apple_build_version__) && (__clang_major__ < 10)
#   undef CATCH_INTERNAL_SUPPRESS_UNUSED_TEMPLATE_WARNINGS
#elif defined(__clang__) && (__clang_major__ < 5)
#   undef CATCH_INTERNAL_SUPPRESS_UNUSED_TEMPLATE_WARNINGS
#endif

#if !defined(CATCH_INTERNAL_SUPPRESS_UNUSED_TEMPLATE_WARNINGS)
#   define CATCH_INTERNAL_SUPPRESS_UNUSED_TEMPLATE_WARNINGS
#endif

#if defined(CATCH_CONFIG_DISABLE_EXCEPTIONS)
#define CATCH_TRY if ((true))
#define CATCH_CATCH_ALL if ((false))
#define CATCH_CATCH_ANON(type) if ((false))
#else
#define CATCH_TRY try
#define CATCH_CATCH_ALL catch (...)
#define CATCH_CATCH_ANON(type) catch (type)
#endif

#if defined(CATCH_INTERNAL_CONFIG_TRADITIONAL_MSVC_PREPROCESSOR) && !defined(CATCH_CONFIG_NO_TRADITIONAL_MSVC_PREPROCESSOR) && !defined(CATCH_CONFIG_TRADITIONAL_MSVC_PREPROCESSOR)
#define CATCH_CONFIG_TRADITIONAL_MSVC_PREPROCESSOR
#endif
// end catch_compiler_capabilities.h
#define INTERNAL_CATCH_UNIQUE_NAME_LINE2( name, line ) name##line
#define INTERNAL_CATCH_UNIQUE_NAME_LINE( name, line ) INTERNAL_CATCH_UNIQUE_NAME_LINE2( name, line )
#ifdef CATCH_CONFIG_COUNTER
#  define INTERNAL_CATCH_UNIQUE_NAME( name ) INTERNAL_CATCH_UNIQUE_NAME_LINE( name, __COUNTER__ )
#else
#  define INTERNAL_CATCH_UNIQUE_NAME( name ) INTERNAL_CATCH_UNIQUE_NAME_LINE( name, __LINE__ )
#endif

#include <iosfwd>
#include <string>
#include <cstdint>

// We need a dummy global operator<< so we can bring it into Catch namespace later
struct Catch_global_namespace_dummy {};
std::ostream& operator<<(std::ostream&, Catch_global_namespace_dummy);

namespace Catch {

    struct CaseSensitive { enum Choice {
        Yes,
        No
    }; };

    class NonCopyable {
        NonCopyable( NonCopyable const& )              = delete;
        NonCopyable( NonCopyable && )                  = delete;
        NonCopyable& operator = ( NonCopyable const& ) = delete;
        NonCopyable& operator = ( NonCopyable && )     = delete;

    protected:
        NonCopyable();
        virtual ~NonCopyable();
    };

    struct SourceLineInfo {

        SourceLineInfo() = delete;
        SourceLineInfo( char const* _file, std::size_t _line ) noexcept
        :   file( _file ),
            line( _line )
        {}
        SourceLineInfo( SourceLineInfo const& other )            = default;
        SourceLineInfo& operator = ( SourceLineInfo const& )     = default;
        SourceLineInfo( SourceLineInfo&& )              noexcept = default;
        SourceLineInfo& operator = ( SourceLineInfo&& ) noexcept = default;

        bool empty() const noexcept { return file[0] == '\0'; }
        bool operator == ( SourceLineInfo const& other ) const noexcept;
        bool operator < ( SourceLineInfo const& other ) const noexcept;

        char const* file;
        std::size_t line;
    };

    std::ostream& operator << ( std::ostream& os, SourceLineInfo const& info );

    // Bring in operator<< from global namespace into Catch namespace
    // This is necessary because the overload of operator<< above makes
    // lookup stop at namespace Catch
    using ::operator<<;

    // Use this in variadic streaming macros to allow
    //    >> +StreamEndStop
    // as well as
    //    >> stuff +StreamEndStop
    struct StreamEndStop {
        std::string operator+() const;
    };
    template<typename T>
    T const& operator + ( T const& value, StreamEndStop ) {
        return value;
    }
}
#define CATCH_INTERNAL_LINEINFO \
    ::Catch::SourceLineInfo( __FILE__, static_cast<std::size_t>( __LINE__ ) )

// end catch_common.h
namespace Catch {

    struct RegistrarForTagAliases {
        RegistrarForTagAliases( char const* alias, char const* tag, SourceLineInfo const& lineInfo );
    };

} // end namespace Catch

#define CATCH_REGISTER_TAG_ALIAS( alias, spec ) \
    CATCH_INTERNAL_START_WARNINGS_SUPPRESSION \
    CATCH_INTERNAL_SUPPRESS_GLOBALS_WARNINGS \
    namespace{ Catch::RegistrarForTagAliases INTERNAL_CATCH_UNIQUE_NAME( AutoRegisterTagAlias )( alias, spec, CATCH_INTERNAL_LINEINFO ); } \
    CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION

// end catch_tag_alias_autoregistrar.h
// start catch_test_registry.h

// start catch_interfaces_testcase.h

#include <vector>

namespace Catch {

    class TestSpec;

    struct ITestInvoker {
        virtual void invoke () const = 0;
        virtual ~ITestInvoker();
    };

    class TestCase;
    struct IConfig;

    struct ITestCaseRegistry {
        virtual ~ITestCaseRegistry();
        virtual std::vector<TestCase> const& getAllTests() const = 0;
        virtual std::vector<TestCase> const& getAllTestsSorted( IConfig const& config ) const = 0;
    };

    bool isThrowSafe( TestCase const& testCase, IConfig const& config );
    bool matchTest( TestCase const& testCase, TestSpec const& testSpec, IConfig const& config );
    std::vector<TestCase> filterTests( std::vector<TestCase> const& testCases, TestSpec const& testSpec, IConfig const& config );
    std::vector<TestCase> const& getAllTestCasesSorted( IConfig const& config );

}

// end catch_interfaces_testcase.h
// start catch_stringref.h

#include <cstddef>
#include <string>
#include <iosfwd>
#include <cassert>

namespace Catch {

     /// A non-owning string class (similar to the forthcoming std::string_view)
    /// Note that, because a StringRef may be a substring of another string,
    /// it may not be null terminated.
    class StringRef {
    public:
        using size_type = std::size_t;
        using const_iterator = const char*;

    private:
        static constexpr char const* const s_empty = "";

        char const* m_start = s_empty;
        size_type m_size = 0;

    public: // construction
        constexpr StringRef() noexcept = default;

        StringRef( char const* rawChars ) noexcept;

        constexpr StringRef( char const* rawChars, size_type size ) noexcept
        :   m_start( rawChars ),
            m_size( size )
        {}

        StringRef( std::string const& stdString ) noexcept
        :   m_start( stdString.c_str() ),
            m_size( stdString.size() )
        {}

        explicit operator std::string() const {
            return std::string(m_start, m_size);
        }
     public: // operators
        auto operator == ( StringRef const& other ) const noexcept -> bool;
        auto operator != (StringRef const& other) const noexcept -> bool {
            return !(*this == other);
        }

        auto operator[] ( size_type index ) const noexcept -> char {
            assert(index < m_size);
            return m_start[index];
        }

    public: // named queries
        constexpr auto empty() const noexcept -> bool {
            return m_size == 0;
        }
        constexpr auto size() const noexcept -> size_type {
            return m_size;
        }

        // Returns the current start pointer. If the StringRef is not
        // null-terminated, throws std::domain_exception
        auto c_str() const -> char const*;

    public: // substrings and searches
        // Returns a substring of [start, start + length).
        // If start + length > size(), then the substring is [start, size()).
        // If start > size(), then the substring is empty.
        auto substr( size_type start, size_type length ) const noexcept -> StringRef;

        // Returns the current start pointer. May not be null-terminated.
        auto data() const noexcept -> char const*;

        constexpr auto isNullTerminated() const noexcept -> bool {
            return m_start[m_size] == '\0';
        }

    public: // iterators
        constexpr const_iterator begin() const { return m_start; }
        constexpr const_iterator end() const { return m_start + m_size; }
    };

    auto operator += ( std::string& lhs, StringRef const& sr ) -> std::string&;
    auto operator << ( std::ostream& os, StringRef const& sr ) -> std::ostream&;

    constexpr auto operator "" _sr( char const* rawChars, std::size_t size ) noexcept -> StringRef {
        return StringRef( rawChars, size );
    }
} // namespace Catch

constexpr auto operator "" _catch_sr( char const* rawChars, std::size_t size ) noexcept -> Catch::StringRef {
    return Catch::StringRef( rawChars, size );
}

// end catch_stringref.h
// start catch_preprocessor.hpp
#define CATCH_RECURSION_LEVEL0(...) __VA_ARGS__
#define CATCH_RECURSION_LEVEL1(...) CATCH_RECURSION_LEVEL0(CATCH_RECURSION_LEVEL0(CATCH_RECURSION_LEVEL0(__VA_ARGS__)))
#define CATCH_RECURSION_LEVEL2(...) CATCH_RECURSION_LEVEL1(CATCH_RECURSION_LEVEL1(CATCH_RECURSION_LEVEL1(__VA_ARGS__)))
#define CATCH_RECURSION_LEVEL3(...) CATCH_RECURSION_LEVEL2(CATCH_RECURSION_LEVEL2(CATCH_RECURSION_LEVEL2(__VA_ARGS__)))
#define CATCH_RECURSION_LEVEL4(...) CATCH_RECURSION_LEVEL3(CATCH_RECURSION_LEVEL3(CATCH_RECURSION_LEVEL3(__VA_ARGS__)))
#define CATCH_RECURSION_LEVEL5(...) CATCH_RECURSION_LEVEL4(CATCH_RECURSION_LEVEL4(CATCH_RECURSION_LEVEL4(__VA_ARGS__)))

#ifdef CATCH_CONFIG_TRADITIONAL_MSVC_PREPROCESSOR
#define INTERNAL_CATCH_EXPAND_VARGS(...) __VA_ARGS__
// MSVC needs more evaluations
#define CATCH_RECURSION_LEVEL6(...) CATCH_RECURSION_LEVEL5(CATCH_RECURSION_LEVEL5(CATCH_RECURSION_LEVEL5(__VA_ARGS__)))
#define CATCH_RECURSE(...)  CATCH_RECURSION_LEVEL6(CATCH_RECURSION_LEVEL6(__VA_ARGS__))
#else
#define CATCH_RECURSE(...)  CATCH_RECURSION_LEVEL5(__VA_ARGS__)
#endif

#define CATCH_REC_END(...)
#define CATCH_REC_OUT

#define CATCH_EMPTY()
#define CATCH_DEFER(id) id CATCH_EMPTY()

#define CATCH_REC_GET_END2() 0, CATCH_REC_END
#define CATCH_REC_GET_END1(...) CATCH_REC_GET_END2
#define CATCH_REC_GET_END(...) CATCH_REC_GET_END1
#define CATCH_REC_NEXT0(test, next, ...) next CATCH_REC_OUT
#define CATCH_REC_NEXT1(test, next) CATCH_DEFER ( CATCH_REC_NEXT0 ) ( test, next, 0)
#define CATCH_REC_NEXT(test, next)  CATCH_REC_NEXT1(CATCH_REC_GET_END test, next)

#define CATCH_REC_LIST0(f, x, peek, ...) , f(x) CATCH_DEFER ( CATCH_REC_NEXT(peek, CATCH_REC_LIST1) ) ( f, peek, __VA_ARGS__ )
#define CATCH_REC_LIST1(f, x, peek, ...) , f(x) CATCH_DEFER ( CATCH_REC_NEXT(peek, CATCH_REC_LIST0) ) ( f, peek, __VA_ARGS__ )
#define CATCH_REC_LIST2(f, x, peek, ...)   f(x) CATCH_DEFER ( CATCH_REC_NEXT(peek, CATCH_REC_LIST1) ) ( f, peek, __VA_ARGS__ )

#define CATCH_REC_LIST0_UD(f, userdata, x, peek, ...) , f(userdata, x) CATCH_DEFER ( CATCH_REC_NEXT(peek, CATCH_REC_LIST1_UD) ) ( f, userdata, peek, __VA_ARGS__ )
#define CATCH_REC_LIST1_UD(f, userdata, x, peek, ...) , f(userdata, x) CATCH_DEFER ( CATCH_REC_NEXT(peek, CATCH_REC_LIST0_UD) ) ( f, userdata, peek, __VA_ARGS__ )
#define CATCH_REC_LIST2_UD(f, userdata, x, peek, ...)   f(userdata, x) CATCH_DEFER ( CATCH_REC_NEXT(peek, CATCH_REC_LIST1_UD) ) ( f, userdata, peek, __VA_ARGS__ )

// Applies the function macro `f` to each of the remaining parameters, inserts commas between the results,
// and passes userdata as the first parameter to each invocation,
// e.g. CATCH_REC_LIST_UD(f, x, a, b, c) evaluates to f(x, a), f(x, b), f(x, c)
#define CATCH_REC_LIST_UD(f, userdata, ...) CATCH_RECURSE(CATCH_REC_LIST2_UD(f, userdata, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

#define CATCH_REC_LIST(f, ...) CATCH_RECURSE(CATCH_REC_LIST2(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

#define INTERNAL_CATCH_EXPAND1(param) INTERNAL_CATCH_EXPAND2(param)
#define INTERNAL_CATCH_EXPAND2(...) INTERNAL_CATCH_NO## __VA_ARGS__
#define INTERNAL_CATCH_DEF(...) INTERNAL_CATCH_DEF __VA_ARGS__
#define INTERNAL_CATCH_NOINTERNAL_CATCH_DEF
#define INTERNAL_CATCH_STRINGIZE(...) INTERNAL_CATCH_STRINGIZE2(__VA_ARGS__)
#ifndef CATCH_CONFIG_TRADITIONAL_MSVC_PREPROCESSOR
#define INTERNAL_CATCH_STRINGIZE2(...) #__VA_ARGS__
#define INTERNAL_CATCH_STRINGIZE_WITHOUT_PARENS(param) INTERNAL_CATCH_STRINGIZE(INTERNAL_CATCH_REMOVE_PARENS(param))
#else
// MSVC is adding extra space and needs another indirection to expand INTERNAL_CATCH_NOINTERNAL_CATCH_DEF
#define INTERNAL_CATCH_STRINGIZE2(...) INTERNAL_CATCH_STRINGIZE3(__VA_ARGS__)
#define INTERNAL_CATCH_STRINGIZE3(...) #__VA_ARGS__
#define INTERNAL_CATCH_STRINGIZE_WITHOUT_PARENS(param) (INTERNAL_CATCH_STRINGIZE(INTERNAL_CATCH_REMOVE_PARENS(param)) + 1)
#endif

#define INTERNAL_CATCH_MAKE_NAMESPACE2(...) ns_##__VA_ARGS__
#define INTERNAL_CATCH_MAKE_NAMESPACE(name) INTERNAL_CATCH_MAKE_NAMESPACE2(name)

#define INTERNAL_CATCH_REMOVE_PARENS(...) INTERNAL_CATCH_EXPAND1(INTERNAL_CATCH_DEF __VA_ARGS__)

#ifndef CATCH_CONFIG_TRADITIONAL_MSVC_PREPROCESSOR
#define INTERNAL_CATCH_MAKE_TYPE_LIST2(...) decltype(get_wrapper<INTERNAL_CATCH_REMOVE_PARENS_GEN(__VA_ARGS__)>())
#define INTERNAL_CATCH_MAKE_TYPE_LIST(...) INTERNAL_CATCH_MAKE_TYPE_LIST2(INTERNAL_CATCH_REMOVE_PARENS(__VA_ARGS__))
#else
#define INTERNAL_CATCH_MAKE_TYPE_LIST2(...) INTERNAL_CATCH_EXPAND_VARGS(decltype(get_wrapper<INTERNAL_CATCH_REMOVE_PARENS_GEN(__VA_ARGS__)>()))
#define INTERNAL_CATCH_MAKE_TYPE_LIST(...) INTERNAL_CATCH_EXPAND_VARGS(INTERNAL_CATCH_MAKE_TYPE_LIST2(INTERNAL_CATCH_REMOVE_PARENS(__VA_ARGS__)))
#endif
#define INTERNAL_CATCH_MAKE_TYPE_LISTS_FROM_TYPES(...)\
    CATCH_REC_LIST(INTERNAL_CATCH_MAKE_TYPE_LIST,__VA_ARGS__)

#define INTERNAL_CATCH_REMOVE_PARENS_1_ARG(_0) INTERNAL_CATCH_REMOVE_PARENS(_0)
#define INTERNAL_CATCH_REMOVE_PARENS_2_ARG(_0, _1) INTERNAL_CATCH_REMOVE_PARENS(_0), INTERNAL_CATCH_REMOVE_PARENS_1_ARG(_1)
#define INTERNAL_CATCH_REMOVE_PARENS_3_ARG(_0, _1, _2) INTERNAL_CATCH_REMOVE_PARENS(_0), INTERNAL_CATCH_REMOVE_PARENS_2_ARG(_1, _2)
#define INTERNAL_CATCH_REMOVE_PARENS_4_ARG(_0, _1, _2, _3) INTERNAL_CATCH_REMOVE_PARENS(_0), INTERNAL_CATCH_REMOVE_PARENS_3_ARG(_1, _2, _3)
#define INTERNAL_CATCH_REMOVE_PARENS_5_ARG(_0, _1, _2, _3, _4) INTERNAL_CATCH_REMOVE_PARENS(_0), INTERNAL_CATCH_REMOVE_PARENS_4_ARG(_1, _2, _3, _4)
#define INTERNAL_CATCH_REMOVE_PARENS_6_ARG(_0, _1, _2, _3, _4, _5) INTERNAL_CATCH_REMOVE_PARENS(_0), INTERNAL_CATCH_REMOVE_PARENS_5_ARG(_1, _2, _3, _4, _5)
#define INTERNAL_CATCH_REMOVE_PARENS_7_ARG(_0, _1, _2, _3, _4, _5, _6) INTERNAL_CATCH_REMOVE_PARENS(_0), INTERNAL_CATCH_REMOVE_PARENS_6_ARG(_1, _2, _3, _4, _5, _6)
#define INTERNAL_CATCH_REMOVE_PARENS_8_ARG(_0, _1, _2, _3, _4, _5, _6, _7) INTERNAL_CATCH_REMOVE_PARENS(_0), INTERNAL_CATCH_REMOVE_PARENS_7_ARG(_1, _2, _3, _4, _5, _6, _7)
#define INTERNAL_CATCH_REMOVE_PARENS_9_ARG(_0, _1, _2, _3, _4, _5, _6, _7, _8) INTERNAL_CATCH_REMOVE_PARENS(_0), INTERNAL_CATCH_REMOVE_PARENS_8_ARG(_1, _2, _3, _4, _5, _6, _7, _8)
#define INTERNAL_CATCH_REMOVE_PARENS_10_ARG(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9) INTERNAL_CATCH_REMOVE_PARENS(_0), INTERNAL_CATCH_REMOVE_PARENS_9_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9)
#define INTERNAL_CATCH_REMOVE_PARENS_11_ARG(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) INTERNAL_CATCH_REMOVE_PARENS(_0), INTERNAL_CATCH_REMOVE_PARENS_10_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10)
#define INTERNAL_CATCH_VA_NARGS_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N

#define INTERNAL_CATCH_TYPE_GEN\
    template<typename...> struct TypeList {};\
    template<typename...Ts>\
    constexpr auto get_wrapper() noexcept -> TypeList<Ts...> { return {}; }\
    template<template<typename...> class...> struct TemplateTypeList{};\
    template<template<typename...> class...Cs>\
    constexpr auto get_wrapper() noexcept -> TemplateTypeList<Cs...> { return {}; }\
    template<typename...>\
    struct append;\
    template<typename...>\
    struct rewrap;\
    template<template<typename...> class, typename...>\
    struct create;\
    template<template<typename...> class, typename>\
    struct convert;\
    \
    template<typename T> \
    struct append<T> { using type = T; };\
    template< template<typename...> class L1, typename...E1, template<typename...> class L2, typename...E2, typename...Rest>\
    struct append<L1<E1...>, L2<E2...>, Rest...> { using type = typename append<L1<E1...,E2...>, Rest...>::type; };\
    template< template<typename...> class L1, typename...E1, typename...Rest>\
    struct append<L1<E1...>, TypeList<mpl_::na>, Rest...> { using type = L1<E1...>; };\
    \
    template< template<typename...> class Container, template<typename...> class List, typename...elems>\
    struct rewrap<TemplateTypeList<Container>, List<elems...>> { using type = TypeList<Container<elems...>>; };\
    template< template<typename...> class Container, template<typename...> class List, class...Elems, typename...Elements>\
    struct rewrap<TemplateTypeList<Container>, List<Elems...>, Elements...> { using type = typename append<TypeList<Container<Elems...>>, typename rewrap<TemplateTypeList<Container>, Elements...>::type>::type; };\
    \
    template<template <typename...> class Final, template< typename...> class...Containers, typename...Types>\
    struct create<Final, TemplateTypeList<Containers...>, TypeList<Types...>> { using type = typename append<Final<>, typename rewrap<TemplateTypeList<Containers>, Types...>::type...>::type; };\
    template<template <typename...> class Final, template <typename...> class List, typename...Ts>\
    struct convert<Final, List<Ts...>> { using type = typename append<Final<>,TypeList<Ts>...>::type; };
#define INTERNAL_CATCH_NTTP_1(signature, ...)\
    template<INTERNAL_CATCH_REMOVE_PARENS(signature)> struct Nttp{};\
    template<INTERNAL_CATCH_REMOVE_PARENS(signature)>\
    constexpr auto get_wrapper() noexcept -> Nttp<__VA_ARGS__> { return {}; } \
    template<template<INTERNAL_CATCH_REMOVE_PARENS(signature)> class...> struct NttpTemplateTypeList{};\
    template<template<INTERNAL_CATCH_REMOVE_PARENS(signature)> class...Cs>\
    constexpr auto get_wrapper() noexcept -> NttpTemplateTypeList<Cs...> { return {}; } \
    \
    template< template<INTERNAL_CATCH_REMOVE_PARENS(signature)> class Container, template<INTERNAL_CATCH_REMOVE_PARENS(signature)> class List, INTERNAL_CATCH_REMOVE_PARENS(signature)>\
    struct rewrap<NttpTemplateTypeList<Container>, List<__VA_ARGS__>> { using type = TypeList<Container<__VA_ARGS__>>; };\
    template< template<INTERNAL_CATCH_REMOVE_PARENS(signature)> class Container, template<INTERNAL_CATCH_REMOVE_PARENS(signature)> class List, INTERNAL_CATCH_REMOVE_PARENS(signature), typename...Elements>\
    struct rewrap<NttpTemplateTypeList<Container>, List<__VA_ARGS__>, Elements...> { using type = typename append<TypeList<Container<__VA_ARGS__>>, typename rewrap<NttpTemplateTypeList<Container>, Elements...>::type>::type; };\
    template<template <typename...> class Final, template<INTERNAL_CATCH_REMOVE_PARENS(signature)> class...Containers, typename...Types>\
    struct create<Final, NttpTemplateTypeList<Containers...>, TypeList<Types...>> { using type = typename append<Final<>, typename rewrap<NttpTemplateTypeList<Containers>, Types...>::type...>::type; };

#define INTERNAL_CATCH_DECLARE_SIG_TEST0(TestName)
#define INTERNAL_CATCH_DECLARE_SIG_TEST1(TestName, signature)\
    template<INTERNAL_CATCH_REMOVE_PARENS(signature)>\
    static void TestName()
#define INTERNAL_CATCH_DECLARE_SIG_TEST_X(TestName, signature, ...)\
    template<INTERNAL_CATCH_REMOVE_PARENS(signature)>\
    static void TestName()

#define INTERNAL_CATCH_DEFINE_SIG_TEST0(TestName)
#define INTERNAL_CATCH_DEFINE_SIG_TEST1(TestName, signature)\
    template<INTERNAL_CATCH_REMOVE_PARENS(signature)>\
    static void TestName()
#define INTERNAL_CATCH_DEFINE_SIG_TEST_X(TestName, signature,...)\
    template<INTERNAL_CATCH_REMOVE_PARENS(signature)>\
    static void TestName()
#define INTERNAL_CATCH_NTTP_REGISTER0(TestFunc, signature)\
    template<typename Type>\
    void reg_test(TypeList<Type>, Catch::NameAndTags nameAndTags)\
    {\
        Catch::AutoReg( Catch::makeTestInvoker(&TestFunc<Type>), CATCH_INTERNAL_LINEINFO, Catch::StringRef(), nameAndTags);\
    }

#define INTERNAL_CATCH_NTTP_REGISTER(TestFunc, signature, ...)\
    template<INTERNAL_CATCH_REMOVE_PARENS(signature)>\
    void reg_test(Nttp<__VA_ARGS__>, Catch::NameAndTags nameAndTags)\
    {\
        Catch::AutoReg( Catch::makeTestInvoker(&TestFunc<__VA_ARGS__>), CATCH_INTERNAL_LINEINFO, Catch::StringRef(), nameAndTags);\
    }

#define INTERNAL_CATCH_NTTP_REGISTER_METHOD0(TestName, signature, ...)\
    template<typename Type>\
    void reg_test(TypeList<Type>, Catch::StringRef className, Catch::NameAndTags nameAndTags)\
    {\
        Catch::AutoReg( Catch::makeTestInvoker(&TestName<Type>::test), CATCH_INTERNAL_LINEINFO, className, nameAndTags);\
    }

#define INTERNAL_CATCH_NTTP_REGISTER_METHOD(TestName, signature, ...)\
    template<INTERNAL_CATCH_REMOVE_PARENS(signature)>\
    void reg_test(Nttp<__VA_ARGS__>, Catch::StringRef className, Catch::NameAndTags nameAndTags)\
    {\
        Catch::AutoReg( Catch::makeTestInvoker(&TestName<__VA_ARGS__>::test), CATCH_INTERNAL_LINEINFO, className, nameAndTags);\
    }

#define INTERNAL_CATCH_DECLARE_SIG_TEST_METHOD0(TestName, ClassName)
#define INTERNAL_CATCH_DECLARE_SIG_TEST_METHOD1(TestName, ClassName, signature)\
    template<typename TestType> \
    struct TestName : INTERNAL_CATCH_REMOVE_PARENS(ClassName)<TestType> { \
        void test();\
    }

#define INTERNAL_CATCH_DECLARE_SIG_TEST_METHOD_X(TestName, ClassName, signature, ...)\
    template<INTERNAL_CATCH_REMOVE_PARENS(signature)> \
    struct TestName : INTERNAL_CATCH_REMOVE_PARENS(ClassName)<__VA_ARGS__> { \
        void test();\
    }
