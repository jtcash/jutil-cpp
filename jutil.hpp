#pragma once

/**
 *  A set of header files to provide useful tools to aid in development and debugging.
 *  jutil is expected to rely heavily on C++17 features, and targets GCC, though MSVC support is
 * expected for most features.
 *  Everything, except for the occasional macro definition when unavoidable, is contained in
 * the `jeff` namespace or the `jfmt` namespace for ostream put formatters like jfmt::x for "as hex"
 * jfmt::q for "quoted"... etc.
 **/



#include "jinfo.hpp"  
#include "jecho.hpp"
#include "jfunc.hpp"
#include "jtype.hpp"
#include "jiter.hpp"
#include "jkeyword.hpp"

#include "jout.hpp"


/* Recommended compiler flags for GCC
https://kristerw.blogspot.com/2017/09/useful-gcc-warning-options-not-enabled.html
https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html

-Wduplicated-cond
-Wduplicated-branches
-Wlogical-op
-Wnull-dereference
-Wold-style-cast              // CHECK: maybe non-existent, need to test
-Wuseless-cast

-Wdouble-promotion        // alert if implicitly promoting floating point types

-Wformat=2 // security checks for format strings, but should not be using those in c++, lol
*/















/*  Things to check on, or things I am not entirely certain about:

// UNCLEAR: The full details on how the compiler looks at functions declared
// static, inline or constexpr. The main concepts are obvious, but I sometimes find myself
// what would be the best option from time to time and second guessing my decsions.

// If a function in a header is just static, the compiler might alert about it being unused.
// whereas if it is static inline, this will not occur. I am not fully certain why this is, but the 
// 


// VERY_HELPFUL: Difference between an inline function and static inline function
//                https://stackoverflow.com/a/12836392/6232717

//  A non-static inline function refers to same function definition across all translation units.
// This means that wherever this function is defined, the body must be identical (actually, what about things
// like noexcept specifiers?). 
//  A static inline function signature can refer to a different function in each translation unit.
//
//  WOW!!! This came across soemthing I thought about a long time ago but never looked into. 
// When you have a static function, static local variables declared in the function body will actually
// not be shared among the (probably identical, in most cases) other versions of that function
// existing in other translation units. Hmm, I can't immediately think of any good use for this
// other than for some type of obfuscation where you have a program with multiple translation units
// that does not behave as one might expect at first glance. 


// Constexpr confusion
// constexpr functions are implicitly inline. Being inline, they have internal linkage
// and can be defined in other translation units without violating the ODR


// inline constexpr non-member variables have internal linkage and can have
// multiple definitions across different translation units. I have found these very useful
// for template metaprogramming, such as in type_sv<Types...>, in the same vein that inline 
// constexpr variables are used by the STL for things like std::is_const_v<Type>



// Some useful references
//https://en.cppreference.com/w/cpp/language/inline
//https://en.cppreference.com/w/cpp/language/storage_duration
//https://en.cppreference.com/w/cpp/language/constexpr
//https://en.cppreference.com/w/cpp/language/definition#One_Definition_Rule

*/