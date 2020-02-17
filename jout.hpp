#pragma once

#include <cstddef> // for std::byte

#include <utility>
#include <ostream>
#include <array>

#include <tuple>
#include <string_view>
#include <string>

// for development

#include <type_traits>


#include <vector>
#include <iostream>

#include <functional>

#include "jinfo.hpp"
#include "jtype.hpp"
// #include "jutil.hpp"




// move to jtype

namespace jeff{
  namespace helper{
    // template<class T>
    // struct quote_helper_t{
    //   T&& t;

    // };


    template<class... Types>
    struct wrap_t{
      std::tuple<Types...> types;
    };

  }
  template<class... Types>
  helper::wrap_t<Types...> wrap(Types&&... types){
    return {std::forward<Types>(types)...};
  }
  

}

namespace jeff{
  // For development purposes; marks types with no os_putter specialization
  inline constexpr bool indicate_unspecialized_os_putter = !true;



} // end namespace jeff

// Os overloading



/// /// NOTE: Move  to jtype
namespace jeff{
  static_assert(sizeof(char) == 1, "I only made this lib to work with sizeof(char)=1");

  constexpr bool is_printable(char c){
    return c <= '~' && c >= ' ';
  }
  constexpr bool is_printable(std::byte b){
    return is_printable(std::to_integer<char>(b));
  }

  constexpr char nibble_to_hex(std::byte nib){
    return "0123456789abcdef"[std::to_integer<std::size_t>(nib & std::byte(0xf))];
  }
  constexpr std::array<char, 3> byte_to_hex(std::byte b){
    return { nibble_to_hex(b>>4), nibble_to_hex(b), '\0'};
  }
}

// NOTE: String literals pass-through os_putter

namespace jeff{
  
  namespace helper{
    // Will basically never be constexpr under regular uses, as it's mainly for stream types
    template<class Os, class T>
    constexpr decltype(auto) os_putter(Os&& os, T&& t){
      if constexpr(indicate_unspecialized_os_putter)
        return std::forward<Os>(os) << '[' << std::forward<T>(t) << ']' << types_sv<T&&>;
        // return std::forward<Os>(os) << '[' << std::forward<T>(t) << ']';
      return std::forward<Os>(os) << std::forward<T>(t);
    }

    // Prevent C-style array types from being snatched (also string literals)
    // template<class Os, class T, size_t N>
    // constexpr decltype(auto) os_putter(Os&& os,  const T (&t)[N]){
    //   return std::forward<Os>(os) << std::forward<decltype(t)>(t);
    // }
    // template<class Os, class T, size_t N>
    // constexpr decltype(auto) os_putter(Os&& os,  T (&&t)[N]){
    //   return std::forward<Os>(os) << std::move(t);
    // }
    // template<class Os, class T, size_t N>
    // constexpr decltype(auto) os_putter(Os&& os,  const  T (&t)[N]){
    //   return std::forward<Os>(os) << std::forward<decltype(t)>(t);
    // }
    // template<class Os, class T, size_t N>
    // constexpr decltype(auto) os_putter(Os&& os, const  T (&&t)[N]){
    //   return std::forward<Os>(os) << std::move(t);
    // }


    
    template<class Os, class First, class... Rest>
    constexpr decltype(auto) os_putter(Os&& os, First&& first, Rest&&... rest){
      return os_putter(os_putter(std::forward<Os>(os), std::forward<First>(first)), std::forward<Rest>(rest)...);
    }
  }




  
} // end namespace jeff



namespace jeff{

  namespace helper{
    template<class Os>
    constexpr decltype(auto) os_putter(Os&& os, bool b){
      return std::forward<Os>(os) << (b ? "true" : "false");
    }

    template<class Os>
    constexpr decltype(auto) os_putter(Os&& os, const char* str){
      return std::forward<Os>(os) << '"' << str << '"';
    }
    template<class Os, class... SvTypes>
    constexpr decltype(auto) os_putter(Os&& os, std::basic_string_view<SvTypes...> sv){
      return std::forward<Os>(os) << '"' << sv << "\"sv";
    }
  }
  // template<class Os, class... SvTypes>
  // constexpr decltype(auto) os_putter(Os&& os, std::basic_string_view<SvTypes...>&& sv){
  //   return std::forward<Os>(os) << '"' << sv << "\"sv";
  // }


  // The string types are a mess and require loads of overloads to capture them all

  // Helper to deal with basic_string types consistently
  


  namespace helper{
    namespace helper2{
      template<class Os, class String>
      constexpr decltype(auto) os_putter_string(Os&& os, String&& s){
        return std::forward<Os>(os) << '"' << std::forward<String>(s) << "\"s";
      }
    }
    template<class Os, class... STypes>
    constexpr decltype(auto) os_putter(Os&& os, std::basic_string<STypes...>&& s){
      return helper2::os_putter_string(std::forward<Os>(os), std::move(s));
    }
    template<class Os, class... STypes>
    constexpr decltype(auto) os_putter(Os&& os,  const std::basic_string<STypes...>&& s){
      return helper2::os_putter_string(std::forward<Os>(os), std::move(s));
    }
    template<class Os, class... STypes>
    constexpr decltype(auto) os_putter(Os&& os, std::basic_string<STypes...>& s){
      return helper2::os_putter_string(std::forward<Os>(os), s);
    }
    template<class Os, class... STypes>
    constexpr decltype(auto) os_putter(Os&& os, const std::basic_string<STypes...>& s){
      return helper2::os_putter_string(std::forward<Os>(os), s);
    }

  
    

    template<class Os, auto N>
    constexpr decltype(auto) os_putter(Os&& os, const char (&test)[N] ){
      return std::forward<Os>(os) << '`' << test << '`';
    }


    
    // template<class... OsTypes>
    // static std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, std::byte b){
    

    // This might not be needed, at least for use with ostream
    template<class Os>
    constexpr decltype(auto) os_putter(Os&& os, std::byte b){
      auto tmp = jeff::byte_to_hex(b);
      return std::forward<Os>(os) << "0x" << tmp.data();
    }

    template<class Os>
    constexpr decltype(auto) os_putter(Os&& os, char c){
      if(is_printable(c))
        return std::forward<Os>(os) << '\'' << c << '\'';
      auto tmp = byte_to_hex(std::byte(c));
      return std::forward<Os>(os) << "'\\x" << tmp.data() << '\'';
    }


  }

}










/*
template< class CharT, class Traits >
std::basic_ostream<CharT, Traits>& endl( std::basic_ostream<CharT, Traits>& os );
	
  */

namespace jeff{
  

  // Pass-through C-style arrays
  template<class Os, class First> constexpr 
  std::enable_if_t<is_array_v<First>, Os&&> os_putter(Os&& os, First&& first){
    return std::forward<Os>(os) << std::forward<First>(first);
  }

  template<class Os, class First> constexpr 
  std::enable_if_t<!is_array_v<First>, Os&&> os_putter(Os&& os, First&& first){
    return helper::os_putter(std::forward<Os>(os), std::forward<First>(first));
  }

  template<class Os, class First, class... Rest>
  constexpr decltype(auto) os_putter(Os&& os, First&& first, Rest&&... rest){
    return os_putter( os_putter(std::forward<Os>(os), std::forward<First>(first)), 
      std::forward<Rest>(rest)... ); 
  }
  // template<class Os, class First, class Second, class... Rest>
  // constexpr decltype(auto) os_putter(Os&& os, First&& first, Second&& second,  Rest&&... rest){
  //   return os_putter( os_putter(std::forward<Os>(os), std::forward<First>(first)), 
  //     std::forward<Second>(second), std::forward<Rest>(rest)... ); 
  // }


  

  template<class Os, class... Args>
  constexpr decltype(auto) put_args(Os&& os, Args&&... args){
    int ctr{};
    return ( (helper::os_putter( ctr++?os<<", ":os , std::forward<Args>(args))), ...); 
  }

// template< class CharT, class Traits >
// std::basic_ostream<CharT, Traits>& endl( std::basic_ostream<CharT, Traits>& os );
  
  // template<class T>
  // std::function<std::ostream&(std::ostream&)> put(const T& t){
  //   return [&t](std::ostream& os){
  //     return os_putter(os, t);
  //   };
  // }



} // end namespace jeff

//// std::ostream << std::tuple

//////////////////////////////////////////////////////////////////////////////////////////////
//// https://en.cppreference.com/w/cpp/utility/apply                                      ////
//// HOLY SHIT, PARAMETER PACK EXPANSION IS INCREDIBLE!!!!                                ////
//// https://en.cppreference.com/w/cpp/language/parameter_pack                            ////
////                                                                                      ////
//// WOW! I should use the comma operator more often!                                     ////
//// https://en.cppreference.com/w/cpp/language/operator_other#Built-in_comma_operator    ////
//////////////////////////////////////////////////////////////////////////////////////////////
template<class... OsTypes, typename... Types>
static std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os,
                                                  std::tuple<Types...> const& theTuple){
  std::apply(
    [&os](Types const&... tupleArgs){
        os << "[ ";
        std::size_t n{0};
        (  (os << tupleArgs << (++n != sizeof...(Types) ? ", " : " ]")), ...  );
    }, 
    theTuple
  );
  return os;
}

template<class... OsTypes, typename... Types>
static std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, std::vector<Types...> const& vec){
  int ctr{};
  for(auto&& e : vec)
    os << (ctr++ == 0 ? "{ ": ", ") << e;
  return os << (ctr==0 ? "{ }" : " }");
}






