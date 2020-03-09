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
    

    /// TODO: Moveme
    // template<char C>
    // constexpr std::string_view as_excape_hex(){
      
    // }



    inline std::string escape_nonprintable(char c){
    // inline std::string_view escape_nonprintable(char c){
    // constexpr std::string_view escape_nonprintable(char c){
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
        // Return a hex representation of the string
        std::string str(2, '\0');
        auto [p, ec] = std::to_chars(str.data(), str.data()+str.size(), static_cast<unsigned char>(c), 16);
        if(ec != std::errc())// this should never happen
          return "{??}";
        return "\\x" + str;
      }
    }
  }// end helper

  /*  old version
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
  */
  // inline std::string escape(char c){
  //   if(is_print(c))
  //     return std::string(1,c);
  //   return helper::escape_nonprintable(c);
  // }
  inline std::string escape(char c){
    if(is_print(c))
      return std::string(1,c);
    return std::string(helper::escape_nonprintable(c));
  }

  // inline std::string escape_s(char c){
  //   if(is_print(c))
  //     return std::string(1,c);
  //   return helper::escape_nonprintable(c);
  // }

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

  


}// end jeff


/** TODO: Document
 * Index:
 *  remove_cvref_t
 *  or_type_t
 *  and_type_t
 *  is_same_nocvref_v
 *  is_same_any_v / is_one_of_v
 *  is_same_nocvref_any_v / is_one_of_nocvref_v
 *  is_array_v
 *  
 * 
 * TODO: Consider moving to to nocvr instead of nocvref
 * 
 * 
 * TODO: Consider changing most class keywords to typename to satisfy common practice 
 * /// Discussion here led me to this decision: https://stackoverflow.com/q/213121/6232717
 * 
 * NOTE: I might be playing around here with tricks to prevent too large an increase in
 * compilation time. For an example, look at MSVC's std::is_same/is_same_v implementation
 * in comparison to gcc's. MSVC seems to like having the is_same_v not rely on a template type
 * at all. Perhaps this is actually the better way to go, as it involves less evaluation and 
 * type expansions? Regardless, this whole idea is moot unless this header were to actually cause
 * a noticeable increase in compilation time
 * */





//
namespace jeff{



  


  /// NOTE: Use local true_type and false_type here to clean up implementations
  using true_type = std::true_type;
  using false_type = std::false_type;

  template<bool Val>
  using bool_constant = std::bool_constant<Val>;


  /// CREDIT: https://en.cppreference.com/w/cpp/language/if#Constexpr_If
  template<typename T>
  struct dependent_false : false_type{};




  // NOTE: Should i use std::is_same, or roll my own?
  /// CREDIT: https://en.cppreference.com/w/cpp/types/is_same
  /*
    template<class T, class U>
    struct is_same : std::false_type{};
    template<class T>
    struct is_same<T,T> : std::true_type{};
    template<class T, class U>
    inline constexpr bool is_same_v = is_same<T,U>::value;
  */ /// But i want to try rolling my own a different way
  template<typename T, typename U> 
  struct is_same : false_type{};

  template<typename T>
  struct is_same<T,T> : true_type{};

  template<typename, typename>
  inline constexpr bool is_same_v = false;
  
  template<typename T>
  inline constexpr bool is_same_v<T,T> = true;
  /// NOTE: Both the template struct and inline var implementations should
  /// not be defined for even slightly complicated things






  /*** remove_cvref ***/
  template<typename...>
  struct remove_cvref{};



  template<typename T> 
  struct remove_cvref<T> :
     std::remove_cv< std::remove_reference_t<T> >{};
  
  template<typename T>
  using remove_cvref_t = typename remove_cvref<T>::type;


  /*** or_type ***/
  // If CondType is true_type, evaluated to true_type, else evaluates to ElseType

  /// NOTE: Changed or_type syntax!
  //  template<class CondType, class ElseType> 
  // struct or_type : 
  //   std::conditional< CondType::value, std::true_type, ElseType >::type{};
  // template<class CondType, class ElseType>
  // using or_type_t = typename or_type<CondType, ElseType>::type;


  // namespace helper{
  //   template<>
  //   struct or_type
  // }// end helper
  // template<class CondType, class ElseType> 
  // struct or_type : 
  //   std::conditional<CondType, std::true_type, ElseType >::type{};

  template<bool Cond, typename ElseType> 
    struct or_type : std::conditional<Cond, true_type, ElseType >::type{};

  // template<Cond, class ElseType> 

  // struct or_type : or_type<std::bool_constant<Cond>, ElseType>{};
    // std::conditional<Cond, std::true_type, ElseType >::type{};

  // template<class CondType, class ElseType>
  // struct or_type<CondType::value, ElseType> : or_type<CondType::value, ElseType>{};

  template<bool Cond, typename ElseType>
  using or_type_t = typename or_type<Cond, ElseType>::type;

  


  /*** and_type ***/

  /// NOTE: Changed and_type syntax!
  // template<class CondType, class ThenType> 
  // struct and_type : 
  //   std::conditional< CondType::value, ThenType, std::false_type >::type{};
  // template<class CondType, class ThenType>
  // using and_type_t = typename and_type<CondType, ThenType>::type;
  
  template<bool Cond, typename ThenType> 
  struct and_type : 
    std::conditional< Cond, ThenType, false_type >::type{};

  template<bool Cond, typename ThenType>
  using and_type_t = typename and_type<Cond, ThenType>::type;

  



  





  /*** is_same_nocvref ***/
  template<typename T, typename U>
  struct is_same_nocvref : 
    is_same< remove_cvref_t<T>, remove_cvref_t<U> >{};
    // std::is_same< remove_cvref_t<T>, remove_cvref_t<U> >{};

  template<typename T, typename U>
  inline constexpr bool is_same_nocvref_v = is_same_nocvref<T,U>::value;



  /*** is_same_any ***/
  template<typename...> struct is_same_any;

  template<typename T>
  struct is_same_any<T> : 
    std::true_type {};

  template<typename T, typename U>
  struct is_same_any<T,U> : 
    std::is_same<T,U> {};


  template<typename T, typename U, typename... Rest>
  struct is_same_any<T,U, Rest...> :
    or_type< std::is_same_v<T,U>, is_same_any<T, Rest...> >{};
    // // or_type< std::is_same<T,U>, is_same_any<T, Rest...> >{};

  template<typename T, typename... Types>
  inline constexpr bool is_same_any_v = is_same_any<T, Types...>::value;


  // A more intelligible alias for is_same_any
  template<typename T, typename... Types>
  struct is_one_of : is_same_any<T, Types...>{};

  template<typename T, typename... Types>
  inline constexpr bool is_one_of_v = is_one_of<T, Types...>::value;
  


  /*** is_same_nocvref_any  ***/
  template<typename...> struct is_same_nocvref_any; 

  template<typename T>
  struct is_same_nocvref_any<T> : true_type {};

  template<typename T, typename U>
  struct is_same_nocvref_any<T,U> : is_same_nocvref<T,U> {};


  template<typename T, typename U, typename... Rest>
  struct is_same_nocvref_any<T,U, Rest...> : 
    or_type< is_same_nocvref<T,U>::value, is_same_nocvref_any<T,Rest...> >{};
    // or_type< is_same_nocvref<T,U>, is_same_nocvref_any<T,Rest...> >{};

  template<typename T, typename... Types>
  inline constexpr bool is_same_nocvref_any_v = is_same_nocvref_any<T, Types...>::value;




  // A more intelligible alias for is_same_nocvref_any
  template<typename T, typename... Types>
  struct is_one_of_nocvref : is_same_nocvref_any<T, Types...>{};
  template<typename T, typename... Types> 
  inline constexpr bool is_one_of_nocvref_v = is_one_of_nocvref<T, Types...>::value;




  /*** is_array ***/
  template<typename T>
  struct is_array : 
    std::is_array< remove_cvref_t<T> > {};

  template< typename T >
  inline constexpr bool is_array_v = is_array<T>::value;





  // TESTING: Basically same as std::common_type_t, but but no use of decay?
  template<typename... Types>
  using common_type_nocvref_t = std::common_type_t<jeff::remove_cvref_t<Types>...>;



  /// TODO: Document
  /// Checks for stringy types

  /// NOTE: Only checks for string literals with char type
  template<typename T>
  struct is_string_literal : false_type{};

  template<std::size_t N> 
  struct is_string_literal<const char [N]> : true_type{};
  
  template<std::size_t N> 
  struct is_string_literal<const char (&)[N]> : true_type{};

  template<typename T>
  inline constexpr bool is_string_literal_v = is_string_literal<T>::value;


  // template<typename...> struct has_property;
  // template<bool Val> struct has_property : std::bool_constant<Val>{};
  
  
  // template<bool B, class T, class F>
  // struct conditional { typedef T type; };
 
  // template<class T, class F>
  // struct conditional<false, T, F> { typedef F type; };
  
  // template< bool B, class T, class F >
  // using conditional_t = typename conditional<B,T,F>::type;


  // template<bool B, typename F>
  // struct jor_type : F{};

  // template<typename F>
  // struct jor_type<true, F> : true_type{};




  template<bool B, typename F, typename... Rest>
  struct jor_type2;

  
  template<typename F, typename... Rest>
  struct jor_type2<true, F, Rest...> : true_type{}; 

  template<typename F, typename... Rest>
  struct jor_type2<false, F, Rest...> : jor_type2<F::value, Rest...>{};
  

//  // template<bool B, typename... Rest>
//   // struct foldl_or;

//   // template<typename F, typename... Rest>
//   // struct foldl_or<true, F, Rest...> : true_type{}; 

//   template<bool B, typename... Rest>
//   struct foldl_or : std::bool_constant<B>{};

//   // template<typename F, typename... Rest>
//   // struct foldl_or<true, F, Rest...> : true_type{}; 

//   template<>
//   struct foldl_or<false> : false_type{};

//   template<typename F, typename... Rest>
//   struct foldl_or<false, F, Rest...> : foldl_or<F::value, Rest...>{};
  // template<bool B, typename... Rest>
  // struct foldl_or;

  // template<typename F, typename... Rest>
  // struct foldl_or<true, F, Rest...> : true_type{}; 




  /// Evaluates to false_type or true_type
  // template<class ValueType, typename... Rest>
  // struct foldl_or : ValueType{};

  // template<bool Value, typename... Rest>
  // struct foldl_or<bool_constant<Value>, Rest...> : BoolConstant<Value>{};


  template<bool B, typename... Rest>
  struct foldl_or : std::bool_constant<B>{};


  /// NOTE: I think this is a pretty neat trick I discovered, though I bet it is actually very
  /// common and probably even has its own name (I guess I probably Colombus-discovered it.).
  /// The primary template's first parameter is a bool non-type, so if we explicitly specialize
  /// for false, that means everything we don't catch here will be caught by the primary template.
  ///
  /// Here, we leverage this trick to make the following defer to itself until the parameter pack 
  /// is exhausted or First::value==true. The overall expression resolves to true_type if
  /// any type parameter has value==true, false_type otherwise.
  //
  template<typename First, typename... Rest>
  struct foldl_or<false, First, Rest...> : foldl_or<First::value, Rest...>{};


  template<bool B, typename... Rest>
  inline constexpr bool foldl_or_v = foldl_or<B, Rest...>::value;

  /// NOTE: Still want



  
  // template<bool B, typename... Rest>
  // struct foldl_or : std::bool_constant<B>{};

  // template<typename F, typename... Rest>
  // struct foldl_or<false, F, Rest...> : foldl_or<F::value, Rest...>{};

  // template<bool B, typename... Rest>
  // inline constexpr bool foldl_or_v = foldl_or<B, Rest...>::value;





  // template<auto Value, typename... Rest>
  // struct foldl_or2 : std::bool_constant<B>{};

  // template<>
  // struct foldl_or2<false> : false_type{};

  // template<typename F, typename... Rest>
  // struct foldl_or<false, F, Rest...> : foldl_or<F::value, Rest...>{};



  // namespace helper{
  //   template<bool B, typename... Rest>
  //   struct foldl_or ;

    

  // }//end helper

 






  // // template<std::size_t Depth, bool B, typename F, typename... Rest>
  // // struct _jor_type4;

  // // template<std::size_t Depth, typename F, typename... Rest>
  // // struct _jor_type4<Depth, true, F, Rest...> : true_type {
  // //   static inline constexpr std::size_t depth = Depth;
  // // };
  // template<std::size_t Depth, bool B, typename F, typename... Rest>
  // struct _jor_type4{
  //   static inline constexpr std::size_t depth = Depth;
  // };

  // template<std::size_t Depth, typename F, typename... Rest>
  // struct _jor_type4<Depth, true, F, Rest...> : true_type 
  // // template<typename F, typename... Rest>
  // // struct jor_type4<true, F, Rest...> : true_type{
  // //   static inline constexpr std::size_t depth;
  // // }; 

  // template<std::size_t Depth, typename F, typename... Rest>
  // struct _jor_type4<Depth, false, F, Rest...> : _jor_type4<Depth+1, F::value, Rest...>{};




  // template<bool B, typename F, typename... Rest>
  // struct jor_type4 : _jor_type4<0, B, F, Rest...> ;

  // // template<typename F, typename... Rest>
  // // struct jor_type4<B,F,Rest...> : _jor_type4<0, B, F, Rest...>{};
  // template<typename F, typename... Rest>
  // struct jor_type4<true, F, Rest...> : _jor_type4<0, true, F, Rest...>{};



  // template<typename F, typename... Rest>
  // struct jor_type4<false, F, Rest...> : _jor_type4<0, F::value, Rest...>{};


  // template<bool Result>
  // struct or_foldl : bool_constant<Result> {};






  // template<typename...>
  // struct or_foldl : false_type{};

  // template<typename Cond>
  // struct or_foldl<Cond> : bool_constant<Cond::value>{};

  // template<typename FirstCond, typename... RestConds>
  // struct or_foldl<FirstCond, RestConds...> : 
  // template<class Check>


  
  template<typename T, template<typename> class... PropTypes>
  struct has_property : std::false_type{};

  template<typename T, template<typename> class PropType>
  struct has_property<T,PropType> : bool_constant<PropType<T>::value>{};
  // struct has_property<T,PropType> : PropType<T>{};

  // template<typename T, template<typename> class PropType>
  // inline constexpr bool 



  template<typename T, template<typename> class FirstProp, template<typename> class... PropTypes>
  struct has_property<T,FirstProp, PropTypes...> : 
    std::conditional_t<has_property<T, FirstProp>::value, 
      std::true_type, 
      has_property<T, PropTypes...>
    >{};

  


  // template<typename T, template<typename> class Prop>
  // struct has_property : Prop<T>{};


  // template<class T>
  // struct is_stringy : std::conditional_t<is_string_literal_v<>>


  // template<class T>
  // struct is_stringy : std::false_type{};


  

  // template<class T>
  // struct is_stringy<std::> : 

}