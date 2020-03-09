#pragma once

#include <type_traits>



#include <string_view>
#include <string>
#include <charconv>

// TODO: Relocate this maybe
namespace jeff{
  // INTERESTING: Something I noticed while checking ASM, is that GCC does not optimize certain things the same way
  // are encapsulated in a constexpr function: https://godbolt.org/z/2YAAJ3


  //// Played around here: https://godbolt.org/z/TdjQPg
  constexpr bool is_cntrl(char c){
    return (c <= '\x1f' && c >= '\0') || c == '\x7f';
    // return static_cast<unsigned char>(c) <= '\x1f' || c == '\x7f'; // same asm generation, gcc be schmahrt
  }

  constexpr bool is_print(char c){
    return c >= ' ' && c <= '~';
  }

  constexpr bool is_space(char c){
    // This seems the best and is branchless:  https://godbolt.org/z/inyHpf
    return c == ' ' || (c >= '\t' && c <= '\r'); 
    // return c >= '\t' && (c == ' ' || c <= '\r'); //x
  }

  constexpr bool is_blank(char c){
    return c == '\t' || c == ' '; // ILP checks both at once
  }

  constexpr bool is_graph(char c){
    return c > ' ' && c <= '~';
  }


  // // TODOOZY: subtractive
  constexpr bool is_punct(char c){
    return (c>='!' && c<='/') 
        || (c>=':' && c<='@') 
        || (c>='[' && c<='`')
        || (c>='{' && c<='~');
  }




  constexpr bool is_lower(char c){
    return c >= 'a' && c <= 'z';
  }
  constexpr bool is_upper(char c){
    return c >= 'A' && c <= 'Z';
  }
  constexpr bool is_digit(char c){
    return c >= '0' && c <= '9';
  }
  constexpr bool is_xdigit(char c){
    return (c >= '0' && c <= '9')
        || (c >= 'A' && c <= 'F')
        || (c >= 'a' && c <= 'f');
  }






  constexpr bool is_alpha(char c){
    return is_lower(c) || is_upper(c);
  }

  constexpr bool is_alnum(char c){
    return is_digit(c) || is_alpha(c);
  }


  constexpr bool is_name(char c){
    return is_alnum(c) || c == '_';
  }
  


  constexpr bool is_name(std::string_view sv){
    for(auto c : sv)
      if(!is_name(c))
        return false;
    return true;
  }


  // NOTE: This is only for pretty printing
  // TODO: Bad performance fix
  namespace helper{
    inline std::string escape_nonprintable(char c){
      switch(c){
      case '\0': return "\\0";
      case '\t': return "\\t";
      case '\n': return "\\n";
      case '\v': return "\\v";
      case '\f': return "\\f";
      case '\r': return "\\r";
      case '\b': return "\\b";
      case '\a': return "\\a";
      default:
        std::string str(2, '\0');
        auto [p, ec] = std::to_chars(str.data(), str.data()+str.size(), static_cast<unsigned char>(c), 16);
        if(ec != std::errc())// this should never happen
          return "{??}";
        return "\\x" + str;
      }
    }
  }// end helper
  inline std::string escape(char c){
    if(is_print(c))
      return std::string(1,c);
    return helper::escape_nonprintable(c);
  }

  inline std::string escaped(std::string_view sv){
    std::string str;

    for(auto c : sv){
      if(is_print(c))
        str += c;
      else
        str += helper::escape_nonprintable(c);
    }

    return str;
  }

  /// TODO: Fix escaped/escape naming confusion
  inline std::string escaped_quoted(char c){
    if(is_print(c))
      return {'\'', c, '\''};
    return '\'' + helper::escape_nonprintable(c) + '\'';
  }
  inline std::string escaped_quoted(std::string_view sv){
    std::string toret{'"'};
    toret.reserve(sv.size()+2);
    
    for(auto c : sv){
      if(is_print(c))
        toret += c;
      else
        toret += helper::escape_nonprintable(c);
    }
    return toret += '"';
  }
  


}// end jeff



namespace jeff{
  /*** remove_cvref ***/
  template<class...>
  struct remove_cvref{};



  template<class T> 
  struct remove_cvref<T> :
     std::remove_cv< std::remove_reference_t<T> >{};
  
  template<class T>
  using remove_cvref_t = typename remove_cvref<T>::type;


  /*** or_type ***/
  // If CondType is true_type, evaluated to true_type, else evaluates to ElseType
  template<class CondType, class ElseType> 
  struct or_type : 
    std::conditional< CondType::value, std::true_type, ElseType >::type{};

  template<class CondType, class ElseType>
  using or_type_t = typename or_type<CondType, ElseType>::type;

  /*** and_type ***/
  template<class CondType, class ThenType> 
  struct and_type : 
    std::conditional< CondType::value, ThenType, std::false_type >::type{};

  template<class CondType, class ThenType>
  using and_type_t = typename and_type<CondType, ThenType>::type;



  /*** is_same_nocvref ***/
  template<class T, class U>
  struct is_same_nocvref : 
    std::is_same< remove_cvref_t<T>, remove_cvref_t<U> >{};

  template<class T, class U>
  inline constexpr bool is_same_nocvref_v = is_same_nocvref<T,U>::value;



  /*** is_same_any ***/
  template<class...> struct is_same_any;

  template<class T>
  struct is_same_any<T> : 
    std::true_type {};

  template<class T, class U>
  struct is_same_any<T,U> : 
    std::is_same<T,U> {};


  template<class T, class U, class... Rest>
  struct is_same_any<T,U, Rest...> :
    or_type< std::is_same<T,U>, is_same_any<T, Rest...> >{};

  template<class T, class... Types>
  inline constexpr bool is_same_any_v = is_same_any<T, Types...>::value;


  // A more intelligible alias for is_same_any
  template<class T, class... Types>
  struct is_one_of : is_same_any<T, Types...>{};
  template<class T, class... Types>
  inline constexpr bool is_one_of_v = is_one_of<T, Types...>::value;
  


  /*** is_same_nocvref_any  ***/
  template<class...> struct is_same_nocvref_any;

  template<class T>
  struct is_same_nocvref_any<T> : 
    std::true_type {};

  template<class T, class U>
  struct is_same_nocvref_any<T,U> : 
    is_same_nocvref<T,U> {};

  template<class T, class U, class... Rest>
  struct is_same_nocvref_any<T,U, Rest...> : 
    or_type< is_same_nocvref<T,U>, is_same_nocvref_any<T,Rest...> >{};

  template<class T, class... Types>
  inline constexpr bool is_same_nocvref_any_v = is_same_nocvref_any<T, Types...>::value;




  // A more intelligible alias for is_same_nocvref_any
  template<class T, class... Types>
  struct is_one_of_nocvref : is_same_nocvref_any<T, Types...>{};
  template<class T, class... Types> 
  inline constexpr bool is_one_of_nocvref_v = is_one_of_nocvref<T, Types...>::value;




  /*** is_array ***/
  template<class T>
  struct is_array : 
    std::is_array< remove_cvref_t<T> > {};

  template< class T >
  inline constexpr bool is_array_v = is_array<T>::value;





  // TESTING: Basically same as std::common_type_t, but but no use of decay?
  template<class... Types>
  using common_type_nocvref_t = std::common_type_t<jeff::remove_cvref_t<Types>...>;



}