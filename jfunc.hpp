#pragma once

#include <cstdint>
#include <cstddef> // std::byte

#include <string_view>


#include <type_traits>
#include <tuple>

#include <optional>
#include <random>


#include <charconv> // for temp





#include "jtype.hpp"
/**
 *  A set of tools related to <functional> as well as a general functional programming paradigm
 * 
 *  Tools:
 *    * hash: like std::hash
 * 
 **/


#include <cmath> // for HUGE_VAL



namespace jeff{
  namespace helper{
      
    // Original crack_atof version is at http://crackprogramming.blogspot.sg/2012/10/implement-atof.html
    // But it cannot convert floating point with high +/- exponent.
    // The version below by Tian Bo fixes that problem and improves performance by 10%
    // http://coliru.stacked-crooked.com/a/2e28f0d71f47ca5e
    // Oliver Schonrock: I picked this code up from
    // https://www.codeproject.com/Articles/1130262/Cplusplus-string-view-Conversion-to-Integral-Types
    // See there for benchmarking. It's blistering fast. 
    // I am sure it's not 10000% "correct", but when summing 1'000'000 parsed doubles for me in a test,
    // it obtained the exact same result as the vastly slower std::stod. Good enough for me.
    // I recfactored it slightly, changing the signature, see below.
    template<class Floating>
    constexpr Floating pow10(int n) {
      Floating ret = 1.0;
      Floating r   = 10.0;
      if (n < 0) {
        n = -n;
        r = 0.1;
      }

      while (n) {
        if (n & 1) {
          ret *= r;
        }
        r *= r;
        n >>= 1;
      }
      return ret;
    }

    // this is the same signature as from_chars (which doesn't work for float on gcc/clang)
    // ie it is a [start, end)  (not including *end). Well suited to parsing read only memorymappedfile
    template<class Floating>
    constexpr Floating crack_atof(const char* num, const char* const end) {
      if (!num || !end || end <= num) {
        return 0;
      }

      int sign         = 1;
      Floating int_part  = 0.0;
      Floating frac_part = 0.0;
      bool has_frac    = false;
      bool has_exp     = false;

      // +/- sign
      if (*num == '-') {
        ++num;
        sign = -1;
      } else if (*num == '+') {
        ++num;
      }

      while (num != end) {
        if (*num >= '0' && *num <= '9') {
          int_part = int_part * 10 + (*num - '0');
        } else if (*num == '.') {
          has_frac = true;
          ++num;
          break;
        } else if (*num == 'e') {
          has_exp = true;
          ++num;
          break;
        } else {
          return sign * int_part;
        }
        ++num;
      }

      if (has_frac) {
        Floating frac_exp = 0.1;

        while (num != end) {
          if (*num >= '0' && *num <= '9') {
            frac_part += frac_exp * (*num - '0');
            frac_exp *= 0.1;
          } else if (*num == 'e') {
            has_exp = true;
            ++num;
            break;
          } else {
            return sign * (int_part + frac_part);
          }
          ++num;
        }
      }

      // parsing exponent part
      Floating exp_part = 1.0;
      if (num != end && has_exp) {
        int exp_sign = 1;
        if (*num == '-') {
          exp_sign = -1;
          ++num;
        } else if (*num == '+') {
          ++num;
        }

        int e = 0;
        while (num != end && *num >= '0' && *num <= '9') {
          e = e * 10 + *num - '0';
          ++num;
        }

        exp_part = pow10<Floating>(exp_sign * e);
      }

      return sign * (int_part + frac_part) * exp_part;
    }

    template<class Floating>
    constexpr Floating crack_atof(std::string_view sv) {
      return crack_atof<Floating>(sv.data(), sv.data()+sv.size());
    }

  }// end helper 
}//end jeff 

// from_chars

namespace jeff{
#ifdef _MSC_VER

  template<class T>
  std::optional<T> chars_to(std::string_view sv){
    T result;
    if(auto [p, ec] = std::from_chars(sv.data(), sv.data()+sv.size(), result); ec == std::errc())
      return std::make_optional(result);
    return std::nullopt;
  }  

#else

  // GCC defines the integral type
  template<class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
  std::optional<T> chars_to(std::string_view sv){
    T result;
    if(auto [p, ec] = std::from_chars(sv.data(), sv.data()+sv.size(), result); ec == std::errc())
      return std::make_optional(result);
    return std::nullopt;
  }


  namespace helper{
    template<class T>
    struct huge_val{};
    
    template<>
    struct huge_val<float>{
      static constexpr float value = HUGE_VALF;
    };
    template<>
    struct huge_val<double>{
      static constexpr double value = HUGE_VAL;
    };
    template<>
    struct huge_val<long double>{
      static constexpr long double value = HUGE_VALL;
    };

    
  }//end helper

  template<class T>
  inline constexpr T huge_value = helper::huge_val<T>::value;


  template<class T, std::enable_if_t<jeff::is_one_of_v<T, float, double, long double>, int> = 0>
  [[nodiscard]] std::optional<T> chars_to_fast(std::string_view sv){
    return helper::crack_atof<T>(sv);
  }
  template<class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
  [[nodiscard]] std::optional<T> chars_to_fast(std::string_view sv){
    T result;
    if(auto [p, ec] = std::from_chars(sv.data(), sv.data()+sv.size(), result); ec == std::errc())
      return result;
    return std::nullopt;
  }

  #ifdef _MSC_VER
  template<class T, std::enable_if_t<std::is_floating_point_v<T> || std::is_integral_v<T>, int> = 0>
  [[nodiscard]] std::optional<T> chars_to(std::string_view sv){
    T result;
    if(auto [p, ec] = std::from_chars(sv.data(), sv.data()+sv.size(), result); ec == std::errc())
      return result;
    return std::nullopt;
  }

  #else
  // template<class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
  template<class T, std::enable_if_t<jeff::is_one_of_v<T, float, double, long double>, int> = 0>
  [[nodiscard]] std::optional<T> chars_to(std::string_view sv){

    if(sv.empty())
      return std::nullopt;
    const char * const str = sv.data();
    // const char * end = sv.data() + sv.size();
    char * end = nullptr;
    // auto endp = &end;
    // try{
    T result;
    if constexpr(std::is_same_v<T, float>){
      result = std::strtof(str, &end);
    } else if constexpr(std::is_same_v<T, double>){
      // return helper::crack_atof(str, str + sv.size());
      result = std::strtod(str, &end);
    } else if constexpr(std::is_same_v<T, long double>){
      result = std::strtold(str, &end);
    } else { 
      // return;
      return std::numeric_limits<T>::signaling_NaN(); // This should be impossible, but static_assert(false) fails
    }


    // if(result == huge_value<T>)
    if(result == huge_value<T> || end == str)
      return std::nullopt;

    return std::make_optional(result);
  }
  #endif


  // template<class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
  // decltype()


#endif
}//end jeff










  template<class... OsTypes, class T>
  static std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, const std::optional<T>& ot) {
    if(ot.has_value())
      return os << '`' << ot.value() << '`';
    return os << "`{}`";
  }







/** GENERATORS */



namespace jeff{
  namespace detail{

    // One random device for all generators
    struct randgen_base{
      inline static std::mt19937 gen = std::mt19937(std::random_device{}());
    };
    // std::mt19937 randgen_base::gen(std::random_device{}()); 

    template<class T>
    using resolve_dist_type_t = std::conditional_t<
      std::is_floating_point_v<T>,
      std::uniform_real_distribution<T>,
      std::uniform_int_distribution<T>
    >;

    template<class T, class dist_type = resolve_dist_type_t<T>>
    struct randgen_t : detail::randgen_base, public dist_type{
      using dist_type::dist_type;
      using typename dist_type::result_type;

      static_assert(std::is_arithmetic_v<result_type>);

      result_type operator()() noexcept{
        return dist_type::operator()(gen);
      }
    };

  } // end detail

  // Make a generator functor that produces random values in range [t, u]
  // the generator inherits from uniform_{int,real}_distribution depending on the type
  template<class T, class U>
  inline auto randgen(T a, U b){
    return detail::randgen_t<std::common_type_t<T,U>>(a, b);
  }
  // See randgen(a,b) for details, same thing but b is decided by the distribution type 
  template<class T>
  inline auto randgen(T t){
    return detail::randgen_t<std::common_type_t<T>>(t); // common_type is used to decay T
  }



  /** a simple generator for seqs */
namespace detail{
  template<class T>
  struct rangen_t{
    T curr;
    const T step;
    T operator()() noexcept{
      return std::exchange(curr, curr + step);
    }
  };
}// end detail
template<class T, class U = T>
constexpr auto rangen(T start, U step = T(1)) noexcept{
  return detail::rangen_t<std::common_type_t<T,U>>{start, step};
}
constexpr auto rangen() noexcept{
  return rangen(0);
}



} // end jeff




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