#pragma once

#include <iterator>
#include <utility>

// Iterator-related utilities

namespace jeff{
  namespace helper{
    // Helper class for skip_first
    template<class T>
    struct skip_first_helper{
      T value;
      const std::size_t n;

      constexpr skip_first_helper(T value, std::size_t n) : value{value}, n{n} {  }

      constexpr decltype(auto) begin() const {
        decltype(auto) be = std::begin(value);
        size_t value_size( std::distance(be, std::end(value)) );
        return std::next( be, std::min(n, value_size) );
      }
      // NOTE: Not sure if there is an idiomatic or proper way to get rid of annoying code 
      // repetition issues like this

      constexpr decltype(auto) begin()  {
        decltype(auto) be = std::begin(value);
        size_t value_size( std::distance(be, std::end(value)) );
        return std::next( be, std::min(n, value_size) );
      }

      constexpr decltype(auto) end() { return std::end(value); }
      constexpr decltype(auto) end() const { return std::end(value); }


      constexpr decltype(auto) size() const{
        decltype(auto) siz = std::size(value);
        return decltype(siz){n >= siz ? 0 : siz - n};
      }
      // private:

    };

    // Skip the first n elements of a lvalue reference range_expression
    template<class T>
    inline constexpr helper::skip_first_helper<T&> skip_first(T& t, size_t n){ return {t, n}; }
    // constexpr helper::skip_first_helper<T&> skip_first(T& t, size_t n = 1){ return {t, n}; }

    // Skip the first n elements of a const lvalue reference range_expression
    template<class T>
    inline constexpr helper::skip_first_helper<const T&> skip_first(const T& t, size_t n){ return {t, n}; }
    // constexpr helper::skip_first_helper<const T&> skip_first(const T& t, size_t n = 1){ return {t, n}; }

    // Skip the first n elements of a rvalue reference range_expression
    template<class T>
    inline constexpr helper::skip_first_helper<T> skip_first(T&& t, size_t n){ return {std::move(t), n}; }

  } // end namespace helper


  /**
   *    skip_first can be used in the context of range-based for loops to not iterate through
   *  the first n elements of a given range_expression. Works with temporaries.
   * 
   *    Note: n is clamped to make sure begin() is either valid or begin() == end()
   *  This might have an impact on performance or optimization, or something. I am also not sure
   *  if I handled temporaries in a 100% proper way, but I think I extended its lifetime properly
   *  without doing any unnecessary copies, assignments or constructions.
   *  
   *    examples:
   *        for(auto&& e : skip_first(std::vector<int>{0,1,2})
   *            // in the first iteration, e == 0
   *        
   *        for(auto&& e : skip_first(2, std::vector<int>{0,1,2})
   *            // Only 2 is ever seen here
   *        
   *        for(auto&& e : skip_first(4, std::vector<int>{0,1,2}))
   *            // The loop is never entered
   *      
   * 
   *  *   Range-based for loop info:            
   *    https://en.cppreference.com/w/cpp/language/range-for
   * */

  // Skip the first n elements of a range_expression
  template<class T>
  constexpr decltype(auto) skip_first(size_t n, T&& t){ return helper::skip_first(std::forward<T>(t), n); }

  // An alias match the syntax of things like std::next
  template<class T>
  constexpr decltype(auto) skip_first(T&& t, size_t n){ return helper::skip_first(std::forward<T>(t), n); }

  // Skip the first element of a range_expression
  template<class T>
  constexpr decltype(auto) skip_first(T&& t){ return helper::skip_first(std::forward<T>(t), 1); }

  // TODO: Figure out if I should have the rvalue reference specialization constructor

}// end namespace jeff





/**   PLANS AND TODOS
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */



//// NOTE: I believe this contained something that was destined for jutil
// https://godbolt.org/z/8D-AJb // I checked on this, and it seems like it
// was just me playing around with methods for doing a zip-like operation
// and zipping vectors together. Reference this if you ever want to know
// how not to start making an iterable type that joins two or more containers.