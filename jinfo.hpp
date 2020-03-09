#pragma once
/**   jinfo.hpp requires C++17 features, and targets GCC, but support for MSVC is a goal
 *    A utility header that can be used to provide information useful for development or debugging.
 *   The focus here is compile-time functions and expressions.
 * 
 */


#include <string_view>






/// NOTE: Developed in https://godbolt.org/z/aSojwE



/** C PREPROCESSOR STRINGIFICATION */

#if  defined (JEFF_STRINGIZE) \
  || defined (JEFF_STRINGIFY) \
  || defined (JEFF_STRINGY)
#   error "jinfo requires all macros in JEFF_{STRINGIZE,STRINGIFY,STRINGY} to be undefined"
#endif
#define JEFF_STRINGIFY(...) JEFF_STRINGIZE(__VA_ARGS__)   /** NOTE: Does the oder of this line and **/
#define JEFF_STRINGIZE(...) #__VA_ARGS__                  /** this line matter? GCC documentation had it this way **/
#define JEFF_STRINGY(...) "STRINGY { " #__VA_ARGS__ " }  =  { " JEFF_STRINGIZE(__VA_ARGS__) " }"



/** PREP COMPILER DETECTION */

#if  defined (JEFF_COMPILER_ID_UNKNOWN) \
  || defined (JEFF_COMPILER_ID_GCC)     \
  || defined (JEFF_COMPILER_ID_CLANG)   \
  || defined (JEFF_COMPILER_ID_ICC)     \
  || defined (JEFF_COMPILER_ID_MSVC)
#   error "jinfo requires all of JEFF_COMPILER_ID_{UNKNOWN,GCC,CLANG,ICC,MSVC} to be undefined" 
#endif
#define JEFF_COMPILER_ID_UNKNOWN 0
#define JEFF_COMPILER_ID_GCC     1
#define JEFF_COMPILER_ID_CLANG   2
#define JEFF_COMPILER_ID_ICC     3
#define JEFF_COMPILER_ID_MSVC    4



/*** NOTE: Whitespace is allowed on a line before '#' for the preprocessor, of course,
 *    but it is also allowed AFTER the '#', between '#' and an identifier
 *     example:
 *       #    define OOF_OUCH_OWIE_OW MY_BONES
 * SOURCE: https://gcc.gnu.org/onlinedocs/cpp/The-preprocessing-language.html
 **//**/

#ifdef JEFF_COMPILER
#   error "jinfo requires JEFF_COMPILER to be undefined"
#endif

#if    defined (JEFF_COMPILER_UNKNOWN)  \
    || defined (JEFF_COMPILER_GCC)      \
    || defined (JEFF_COMPILER_CLANG)    \
    || defined (JEFF_COMPILER_ICC)      \
    || defined (JEFF_COMPILER_MSVC)
#   error "All JEFF_COMPILER_.* macros must be undefined at this point! Why would they be defined here?"
#endif



/** SIMPLE COMPILER DETECTION
 *  Define macros JEFF_COMPILER_ID and one of JEFF_COMPILER_{UNKNOWN,GCC,CLANG,ICC,MSVC}
 *
 *  NOTE: Not sure if I ever want to support any other compilers.
 *        Even Intel C++ Compiler is a hot mess that has made this super confusing to test
 *        Normally, when you discover a compiler bug, you're actually just a dummy and the compiler is fine.
 *        However, I've encountered 4 or 5 genuine compiler bugs in icc while working on this, where
 *        icc rejects or issues false warnings on code that I am 100% certain is valid C++17 code and 
 *        is accepted as expected by CLANG, GCC, and MSVC. I have noted some of these bugs in NOTE tags,
 *        but I am hesitant to report them because icc just seems hopeless :'(
 * 
 *  NOTE: This might have been handy: https://sourceforge.net/p/predef/wiki/Compilers/
 *
 *  NOTE: I am just going to assume all unknown compilers are GNUC compatible. This is guaranteed to blast
 *        errors everywhere when this assumption is wrong.
 */

#ifdef __INTEL_COMPILER
#   define JEFF_COMPILER_ID JEFF_COMPILER_ID_ICC
#   define JEFF_COMPILER_ICC
#   pragma message("WARNING: Intel C++ Compiler has lots of language bugs that reject valid C++14/C++17 code. WATCH OUT!")
#elif defined (_MSC_VER)
#   define JEFF_COMPILER_ID JEFF_COMPILER_ID_MSVC
#   define JEFF_COMPILER_MSVC
#elif defined (__clang__)
#   define JEFF_COMPILER_ID JEFF_COMPILER_ID_CLANG
#   define JEFF_COMPILER_CLANG
#elif defined (__GNUC__)
#   define JEFF_COMPILER_ID JEFF_COMPILER_ID_GCC
#   define JEFF_COMPILER_GCC
#else
#   define JEFF_COMPILER_ID JEFF_COMPILER_ID_UNKNOWN
#   define JEFF_COMPILER_UNKNOWN
#   pragma message("WARNING: Compiler not in {gcc,clang,msvc,icc}, so IDK if everything will behave")
#endif



/** Definitions for:
 *    JEFF_HAS_GNU_CPP11_ATTRIBUTES
 *    JEFF_HAS_GNU_ATTRIBUTES
 *    JEFF_HAS_PRETTY_FUNCTION
 *    JEFF_FANCY_FUNCTION
 *
 *  NOTE: All JEFF_HAS_.* macros get defined to true or false, so do not test them with ifdef/defined
 */
#if defined (JEFF_FANCY_FUNCTION)
#   error "jinfo requires JEFF_FANCY_FUNCTION to be undefined"
#endif

#ifdef JEFF_COMPILER_MSVC
#     define JEFF_HAS_CPP11_PRAGMA false
#     if _MSC_VER > 1924 || _MSVC_LANG > 201703L
#         pragma message("WARNING: the C++11 standard _Pragma() directive might be properly implemented in this version of MSVC." \
                          " If MSVC finally decides follows the language standard, my workarounds will break")
#     endif
#     ifdef _Pragma
#         pragma message("WARNING: I did not expect _Pragma to be a defined macro. This could mean MSVC decided to implement it" \
                          " as a macro and are finally following the C++11 standard, or something else is bizarre!")
#     endif
#     define JEFF_HAS_GNU_CPP11_ATTRIBUTES false
#     define JEFF_HAS_PRETTY_FUNCTION false
#     define JEFF_FANCY_FUNCTION __FUNCSIG__
#else
#     define JEFF_HAS_CPP11_PRAGMA true
#     define JEFF_HAS_GNU_CPP11_ATTRIBUTES true
#     define JEFF_HAS_PRETTY_FUNCTION true
#     define JEFF_FANCY_FUNCTION __PRETTY_FUNCTION__
#endif


/** Definitions for:
 *    JEFF_NOINLINE
 *    JEFF_INLINE
 */
#if JEFF_HAS_GNU_CPP11_ATTRIBUTES
#   define JEFF_NOINLINE  [[gnu::noinline]]
#   define JEFF_INLINE    [[gnu::always_inline]] inline
#elif defined (JEFF_COMPILER_MSVC)
#   define JEFF_NOINLINE  __declspec(noinline)
#   define JEFF_INLINE    __forceinline
#else
#   error "Unhandled because unreachable when first written"
#endif



#if defined (JEFF_PRAGMA_MESSAGE) || defined (JEFF_PRAGMA_MESSAGE_PREFIX)
#   error "jinfo requires JEFF_PRAGMA_MESSAGE and JEFF_PRAGMA_MESSAGE_PREFIX to be undefined"
#endif
#define JEFF_PRAGMA_MESSAGE_PREFIX __FILE__ ": Line " JEFF_STRINGIFY(__LINE__)

#if JEFF_HAS_CPP11_PRAGMA
#   define JEFF_PRAGMA_MESSAGE(...) _Pragma( JEFF_STRINGIZE(message( JEFF_PRAGMA_MESSAGE_PREFIX "\n\t" __VA_ARGS__) )   ) 
// #   define JEFF_PRAGMA_MESSAGE(...) _Pragma( JEFF_STRINGIZE(message( __FILE__ ": Line " JEFF_STRINGIFY(__LINE__) "\n\t" __VA_ARGS__) )   ) 
/// NOTE: BUG: Intel C++ Compiler has issues with either pragma message or c preprocessor ordering
///            Look at the warning caused by calling this macro and its message output
#elif defined (JEFF_COMPILER_MSVC)
#   define JEFF_PRAGMA_MESSAGE(msg) __pragma(message( JEFF_PRAGMA_MESSAGE_PREFIX "\n\t" msg ));
// #   define JEFF_PRAGMA_MESSAGE(msg) __pragma(message( __FILE__ ": Line " JEFF_STRINGIFY(__LINE__) ":\n\t" msg ))
#else
#   error "Unhandled because unreachable when first written"
#endif



/** jeff::compiler namespace can be used to access compiler details 
 *  TODO: Documentation
 */

namespace jeff{
  namespace compiler{
    enum class compiler_id : int {
      unknown = JEFF_COMPILER_ID_UNKNOWN, 
      gcc     = JEFF_COMPILER_ID_GCC, 
      clang   = JEFF_COMPILER_ID_CLANG, 
      icc     = JEFF_COMPILER_ID_ICC, 
      msvc    = JEFF_COMPILER_ID_MSVC,
      boof

    };

    /// Hold an identifier representing the current compiler
    inline constexpr compiler_id id{JEFF_COMPILER_ID}; // alias for identifier
    inline constexpr compiler_id identifier{id};

    /// Compartmentalize inline template variable get_name
    namespace helper{
      /// NOTE: Using std::string_view as return type for get_name because Intel C++ compiler has bug that
      /// rejects using decltype(auto) or reference auto types.
      // #ifdef JEFF_COMPILER_MSVC
      // #   define JEFF_GCN_TYPE inline static constexpr std::string_view
      // #else
      #   define JEFF_GCN_TYPE inline constexpr std::string_view
      // #endif

      template<compiler_id>
      inline constexpr std::string_view get_compiler_name{"invalid_compiler_id"};
      
      template<> inline constexpr std::string_view get_compiler_name<compiler_id::msvc>     { "msvc"};
      template<> inline constexpr std::string_view get_compiler_name<compiler_id::unknown>  { "unknown"};
      template<> inline constexpr std::string_view get_compiler_name<compiler_id::gcc>      { "gcc"};
      template<> inline constexpr std::string_view get_compiler_name<compiler_id::clang>    { "clang"};
      template<> inline constexpr std::string_view get_compiler_name<compiler_id::icc>      { "icc"};

      
      // // #define JEFF_GCN_TYPE inline constexpr const std::string_view
      // template<compiler_id>
      // JEFF_GCN_TYPE get_compiler_name = "invalid_compiler_id";
      // template<> JEFF_GCN_TYPE get_compiler_name<compiler_id::msvc> =  " msvc";
      // template<> JEFF_GCN_TYPE get_compiler_name<compiler_id::unknown> =  "unknown";
      // template<> JEFF_GCN_TYPE get_compiler_name<compiler_id::gcc> =      "gcc";
      // template<> JEFF_GCN_TYPE get_compiler_name<compiler_id::clang> =    "clang";
      // template<> JEFF_GCN_TYPE get_compiler_name<compiler_id::icc> =      "icc";
      #undef JEFF_GCN_TYPE
      // template<compiler_id>
      // inline constexpr const std::string_view get_compiler_name = "invalid_compiler_id";
      // template<> inline constexpr std::string_view get_compiler_name<compiler_id::msvc> =  " msvc";
      // template<> inline constexpr const std::string_view get_compiler_name<compiler_id::unknown> =  "unknown";
      // template<> inline constexpr const std::string_view get_compiler_name<compiler_id::gcc> =      "gcc";
      // template<> inline constexpr const std::string_view get_compiler_name<compiler_id::clang> =    "clang";
      // template<> inline constexpr const std::string_view get_compiler_name<compiler_id::icc> =      "icc";



    /*** NOTE: Found bug in intel c++ compiler: 
     * All definitions of WTF_ICC (other than first) cause compiler errors, despite the fact that it's valid
     * C++ code. This is accepted by other compilers as expectedc
      #define WTF_ICC std::string_view
      // #define WTF_ICC decltype(auto)
      // #define WTF_ICC const auto&
      // #define WTF_ICC auto&
      // #define WTF_ICC auto&&
      template<compiler_id> inline constexpr WTF_ICC get_name = "invalid_compiler_id";
      template<> inline constexpr WTF_ICC get_name<compiler_id::unknown> = "unknown";
      template<> inline constexpr WTF_ICC get_name<compiler_id::gcc> = "gcc";
      template<> inline constexpr WTF_ICC get_name<compiler_id::clang> = "clang";
      template<> inline constexpr WTF_ICC get_name<compiler_id::icc> = "icc";
      template<> inline constexpr WTF_ICC get_name<compiler_id::msvc> = "msvc";
      */

    }// end helper
    /// Get a string representation of the detected compiler
    #ifdef JEFF_COMPILER_MSVC
    inline constexpr std::string_view name{"msvc"}; // SHUT UP INTELLISENSE! YOU ARE WRONG!
    #else
    inline constexpr std::string_view name = helper::get_compiler_name<identifier>;
    #endif

  }//end compiler
}//end jeff
















/// TODO: Remove braces with type_sv always






///////////////////////////////////////////////////////////////////////////////////////////////////
//    The utility type_sv<Types...> can be used to get a compile-time string_view containing the
//  name(s) of types. type_sv<Type> returns just a sv f that type, but with 0 or >1 types, the
//  types will be encapsulated in braces. types_sv is the same thing as type_sv, but it still
//  wraps the type string in braces even if only one type is specified
//    argtype_sv(args...) is similar, but gets type_sv<ArgTypes...>, where ArgTypes are the types
//  of args that would be seeb in a forwarding context.
//    If you want the exact type of an expression in the current context, you have to use
//  sv_args<decltype(expression)> for now, as I am not sure if (or how) I can make a non macro
//  solution to allow this functionality in a clean package.
// NOTE: I still need to add MSVC support using __FUNCSIG__ instead
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace jeff{

  namespace helper{ /// Avoid namespace pollution

  #ifdef _MSC_VER
    template<class... Ts>
    inline constexpr std::string_view get_types_sv(){
      constexpr std::string_view pretty_func{__FUNCSIG__};
    constexpr std::string_view func{__FUNCTION__};

    constexpr auto withParenths = pretty_func.substr(pretty_func.find(func) + func.size());

    constexpr std::string_view toTrim{"(void)"};
    return withParenths.substr(0, withParenths.size() - toTrim.size());
    }
  #else
    template<class... Ts>
    inline constexpr std::string_view get_types_sv(){
      constexpr std::string_view pretty_func{__PRETTY_FUNCTION__};
      constexpr auto open_brace_pos = pretty_func.find('{'); // GCC wraps the types in braces
      return pretty_func.substr(open_brace_pos, pretty_func.rfind('}') - open_brace_pos + 1);
    }
    #endif

    template<class... Ts>
    inline constexpr std::string_view get_type_sv(){
      constexpr auto brace_wrapped = get_types_sv<Ts...>();
      //// Note, I have changed my mind and type_sv will always strip braces
      return brace_wrapped.substr(1, brace_wrapped.size()-2); 
      // if constexpr(sizeof...(Ts) == 1)  // Remove braces around type if there is exactly one
        // return brace_wrapped.substr(1, brace_wrapped.size()-2); 
      // return brace_wrapped;  
    }

  } // End namespace helper

  // Get a string_view representation of the passed type(s)
  template<class... Ts> 
  inline constexpr std::string_view type_sv = helper::get_type_sv<Ts...>();

  // Get a string_view representation of the passed type(s)
  template<class... Ts> 
  inline constexpr std::string_view types_sv = helper::get_types_sv<Ts...>();

  // Get a string_view for the type of the expressions passed in as arguments
  template<class... Ts> 
  constexpr decltype(auto) argtype_sv(Ts&&... ts){ return type_sv<decltype(ts)...>; }
  
  // Get a string_view for the type of the expressions passed in as arguments
  template<class... Ts> 
  constexpr decltype(auto) argtypes_sv(Ts&&... ts){ return types_sv<decltype(ts)...>; }

  // TODO: make sure I don't need to make these inline or static for use in multiple compilation units

}// End namespace jeff


