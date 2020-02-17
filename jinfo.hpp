#pragma once
/**   jinfo.hpp requires C++17 features, and targets GCC, but support for MSVC is a goal
 *    A utility header that can be used to provide information useful for development or debugging.
 *   The focus here is compile-time functions and expressions.
 * 
 */


#include <string_view>



// TODO: Remove braces with type_sv always



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


