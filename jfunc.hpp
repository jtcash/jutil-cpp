#pragma once

#include <cstdint>
#include <cstddef> // std::byte

#include <string_view>


#include <type_traits>
#include <tuple>

#include <optional>

#include "jtype.hpp"
/**
 *  A set of tools related to <functional> as well as a general functional programming paradigm
 * 
 *  Tools:
 *    * hash: like std::hash
 * 
 **/



namespace jos{


  template<class... OsTypes, class T>
  static std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, const std::optional<T>& ot) {
    if(ot.has_value())
      return os << '`' << ot.value() << '`';
    return os << "`{}`";
  }

}// end jos




namespace jeff{

  namespace helper{
    // uint32_t 

    

   


    // NOTE: I played around with different specialization methods
    // template<class T, auto...>
    // struct xorshifter_t;

    // template<auto ParamA, auto ParamB, auto ParamC>
    // struct xorshifter_t<std::uint32_t, ParamA, ParamB, ParamC> {
    //   using value_type = std::uint32_t;
    //   static constexpr value_type step(value_type x) noexcept{
    //     x ^= x << ParamA;
    //     x ^= x >> ParamB;
    //     x ^= x << ParamC;
    //     return x;
    //   }
    // };
    
    // // Default shift parameters
    // template<> struct xorshifter_t<std::uint32_t> : xorshifter_t<std::uint32_t, 13, 17, 5> { };

    // template<auto ParamA, auto ParamB, auto ParamC>
    // struct xorshifter_t<std::uint64_t, ParamA, ParamB, ParamC> {
    //   using value_type = std::uint64_t;
    //   static constexpr value_type step(value_type x) noexcept{
    //     x ^= x << ParamA;
    //     x ^= x >> ParamB;
    //     x ^= x << ParamC;
    //     return x;
    //   }
    // };
    
    // Default shift parameters
    // template<> struct xorshifter_t<std::uint64_t> : xorshifter_t<std::uint64_t, 13, 7, 17> { };

     namespace detail{
      // CREDIT: xorshift algorithm from https://doi.org/10.18637%2Fjss.v008.i14
      template<class T, auto ParamA, auto ParamB, auto ParamC>
      struct xorshifter_t{
        using value_type = T;

        [[nodiscard]] static constexpr decltype(auto) parameters() noexcept{
          return std::make_tuple(ParamA, ParamB, ParamC);
        }

        [[nodiscard]] static constexpr value_type step(value_type x) noexcept{
          x ^= x << ParamA;
          x ^= x >> ParamB;
          x ^= x << ParamC;
          return x;
        }
      };
    } // end namespace detail

    // Primary template
    template<class T, auto...> struct xorshifter_t;
    
    // Specializations for uint32_t and uint64_t
    template<auto A, auto B, auto C>
    struct xorshifter_t<std::uint32_t, A, B, C> : detail::xorshifter_t<std::uint32_t, A, B, C> { };

    template<auto A, auto B, auto C>
    struct xorshifter_t<std::uint64_t, A, B, C> : detail::xorshifter_t<std::uint64_t, A, B, C> { };


    // Default parameters
    template<> struct xorshifter_t<std::uint32_t> : xorshifter_t<std::uint32_t, 13,17,5>{};
    template<> struct xorshifter_t<std::uint64_t> : xorshifter_t<std::uint64_t, 13,7,17>{};

    // template<class T, auto... Params>
    // using xorshifter_resolver = xorshifter_t<jeff::remove_cvref_t<T>, Params...>;

    template<class T, auto... Params>
    struct xorshifter_resolver : xorshifter_t<
      std::make_unsigned_t<
        jeff::remove_cvref_t<T>
      >,
      Params...
    > { };
  }// end namespace helper


  // The two overloads for xorshfit:
  // One works by reference, the other works by value, which explains the return types
  template<class T>
  constexpr void xorshift(T& t){
    t = helper::xorshifter_resolver<T>::step(t);
  }
  template<class T> [[nodiscard]] 
  // constexpr T xorshift(T&& t){
  constexpr T xorshift(const T& t){
    return helper::xorshifter_resolver<T>::step(std::move(t));
  }


/**
 *  jeff::hash will be like std::hash, but the hashed result type is not necessary std::size_t
 * 
 *  The only rule is that the returned type must be implicitly convertible to std::size_t
 * 
 *  If you wish to have a hash result type something that is not implicitly convertible to std::size_t,
 * the best option might be to return a helper class that is implicitly convertible to both. However,
 * I have not yet planned this out. Basically, hash types should be unsigned integer types with
 * width <= std::size_t.
 */


  namespace helper{
    using string_hash_type = std::uint32_t; // using uint32_t, because collisions are not a primary concern


    // TODO: For much later in this project, if I keep using djb2 or similar algs, I could come up with a
    // method for handling multiple characters in one operation. However, this requires figuring out a
    // portable method to deal with endianness. Without C++20, this is surprisingly difficult.
    //  The issue is that the word-by-word hasher cannot be constexpr, because it requires
    // doing reinterpret_casts to grab blocks of memory or using some non-standard rubbish like a union
    // and not complying with the C++ standard, relying on compilers having defined results from technically
    // undefined behavior. The multichar trick works to detect endianness on gcc and msvc, but on gcc it emits
    // a warniing `-Wmultichar` which cannot be suppressed without changing the g++ command line arguments.
    // it seems like it's a bug dating back to 2013 from the last time I checked. The GCC diagnostic ignore
    // pragma straight-up ignores "-Wmultichar".
    //  I always compile my code with -Wall -Wextra -pedantic as a standard practice, so this limits me
    // from using a lot of compiler-specific tricks. But I love 99.65% of the decisions made in the standard,
    // so I like to stick with conformal c++




    // I am using djb2 for my hashing for no particular reason. I just like it and may look for alternatives later
    // Credits: https://en.wikipedia.org/wiki/Daniel_J._Bernstein
    //          http://www.cse.yorku.ca/~oz/hash.html
    // Rumor is that Daniel J. Bernstein is actually the original discoverer of the number 33, which eluded
    // the world's greatest number-discovering-people until 1991:
    //  Proof: https://groups.google.com/forum/#!msg/comp.lang.c/lSKWXiuNOAk/zstZ3SRhCjgJ
    inline constexpr string_hash_type string_hash_initial{5381u};
    
    // [[nodiscard]] constexpr string_hash_type string_hash_step(string_hash_type hash, std::uint8_t b) noexcept{
    //   return hash*decltype(hash){33} + b; // The compiler does not care how this is provided
    // }
    template<class T, std::enable_if_t<sizeof(T)==1, int> = 0>
    [[nodiscard]] constexpr string_hash_type string_hash_step(string_hash_type hash, T t) noexcept{
      return hash*decltype(hash){33} + t; // The compiler does not care how this is provided
      // NOTE: Will signedness of b affect anything as far as code generation goes?
    }
    // Byte specialize required
    [[nodiscard]] constexpr string_hash_type string_hash_step(string_hash_type hash, std::byte b) noexcept{
      return string_hash_step(hash, std::to_integer<std::uint8_t>(b));
    }


    // TODO: Noexcept evaluation. What if they have bad ranges or something?
    constexpr string_hash_type make_hash(std::string_view sv) noexcept {
      string_hash_type hash{string_hash_initial};
      for (auto c : sv)
        hash = string_hash_step(hash, c);
      return hash;
    }
    // NOTE: This also covers std:string, as it has implicit operator string_view()
    
    // NOTE: perhaps use proper basic types instead of specializations
    //  e.g. use template<class charT, class traits> std::basic_string_view<charT, traits>
    //  instead of std::string_view  
  

    template<std::size_t N>
    constexpr string_hash_type make_hash(const char (&arr)[N]) {
      return make_hash(std::string_view{arr, N-1}); // WARNING! This assumes the last character is '\0'
    }

    // NOTE: I went overboard testing assembly generation https://godbolt.org/z/iWUdZM
    //  This seems to be good for both msvc, gcc and clang
    constexpr string_hash_type make_hash(const char* str) {
      if(str){
        string_hash_type hash{string_hash_initial};
        for(char c=*str; c!='\0'; c=*++str)
          hash = string_hash_step(hash, c);          // hash = hash*33 + c;
        return hash;
      }
      return 0;
    }

  } // end namespace helper

  // Unspecialized catch-all.
  template<class...>
  struct hash;

  template<class T>
  struct hash<T> {
    constexpr decltype(auto) operator()(const T& t) noexcept(noexcept(helper::make_hash(t))){
      return helper::make_hash(t);
    }
  };

  template<class T>
  constexpr decltype(auto) make_hash(T&& t) noexcept(noexcept(helper::make_hash(std::forward<T>(t)))){ // lol, unnecessary?
    return helper::make_hash(std::forward<T>(t));
  } 
  // constexpr decltype(auto) make_hash(const T& t) noexcept(noexcept(helper::))
  // template<>
  // struct hash<std::string_view>{

  // };


} // End namespace jeff