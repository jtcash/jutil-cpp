#pragma once

#include <string_view>
#include <cstdint>
#include <cstddef> // std::byte

#include <type_traits>
/**
 *  A set of tools related to <functional> as well as a general functional programming paradigm
 * 
 *  Tools:
 *    * hash: like std::hash
 * 
 **/

namespace jeff{

  namespace helper{
    // uint32_t 
  }// end namespace helper



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




    // [[nodiscard]] constexpr string_hash_type string_hash_step(string_hash_type hash, std::int8_t c) noexcept{
    //   return string_hash_step(hash, static_cast<std::uint8_t>(c));
    // }

    // TODO: Noexcept evaluation. What if they have bad ranges or something?
    constexpr string_hash_type make_hash(std::string_view sv) {
      string_hash_type hash{string_hash_initial};
      for (auto c : sv)
        hash = string_hash_step(hash, c);
      return hash;
    }
    template<class T, std::size_t N>
    constexpr string_hash_type make_hash(const char (&arr)[N]) {
      return make_hash(std::string_view{arr, N-1}); // WARNING! This assumes the last character is '\0'
    }
    constexpr string_hash_type make_hash(const char* str) {
      // Note: This traverses the string twice: once to get length, once to calculate hash.
      // Maybe it would be better to just do while != '\0' loop
      // return make_hash(std::string_view(str));
      if(str){ // Consult https://godbolt.org/z/EPFn7t for performance differences with msvc
        string_hash_type hash{string_hash_initial};
        while(*str != '\0')
          hash = string_hash_step(hash, *str++);
      }
      return 0;
    }


  } // end namespace helper

  // Unspecialized catch-all.
  template<class...>
  struct hash;


  template<>
  struct hash<std::string_view>{

  };


} // End namespace jeff