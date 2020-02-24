#pragma once

#include <type_traits>



// TODO: Relocate this maybe
#include <string_view>


namespace jeff{
  constexpr bool is_lower(char c){
    return c >= 'a' && c <= 'z';
  }
  constexpr bool is_upper(char c){
    return c >= 'A' && c <= 'Z';
  }
  constexpr bool is_digit(char c){
    return c >= '0' && c <= '9';
  }

  constexpr bool is_alpha(char c){
    return is_lower(c) || is_alpha(c);
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