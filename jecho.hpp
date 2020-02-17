#pragma once

#include <iostream>
#include <string_view>

#include "jout.hpp"





namespace jeff{
  namespace helper{
    #ifdef _MSV_VER
    inline constexpr char file_separator_char = '\\';
    #else
    inline constexpr char file_separator_char = '/';
    #endif


   
  }

  // TODO: Variable number of chunks to display
  constexpr std::string_view simplify_file_path(std::string_view filePath, char sep = helper::file_separator_char){
    auto slashLoc = filePath.find_last_of(sep);
    if(slashLoc == std::string_view::npos)
      return filePath;
    auto slashLoc2 = filePath.find_last_of(sep, slashLoc-1);
    if(slashLoc2 == std::string_view::npos)
      return filePath;
    return filePath.substr(slashLoc2+1);
  }
}




#ifdef JECHO_HELPER
#error JECHO_HELPER is already defined. `jecho.hpp` requires JECHO_HELPER to be undefined
#else
#define JECHO_HELPER(os, label, args) jeff::os_putter(os << label << " = ", args)
#endif



// Make sure the definitions are available
#if defined(jecho) || defined(jechot) || defined(jlecho) || defined(jlechot)
#pragma message("jout.hpp defines `jecho`, `jechot`, `jlecho` and `jlechot`; at least one is already defined" \
"and will not be redefined in here")
#endif

#ifndef jecho
#define jecho(...)  (  JECHO_HELPER(std::cout, #__VA_ARGS__, (__VA_ARGS__)) << std::endl  )
#endif

#ifndef jechot
#define jechot(...) (  JECHO_HELPER(std::cout, #__VA_ARGS__, (__VA_ARGS__)) << "; \t"  )
#endif

#ifndef jlecho
#define jlecho(...)  (  JECHO_HELPER(std::cout << , #__VA_ARGS__, (__VA_ARGS__)) << std::endl  )
#endif

#ifndef jlechot
#define jlechot(...) (  JECHO_HELPER(std::cout, #__VA_ARGS__, (__VA_ARGS__)) << "; \t"  )
#endif





// The main macros, TODO: move to jecho.hpp
// #define jecho(...) jeff::os_putter(std::cout, #__VA_ARGS__, " = ", ( __VA_ARGS__ )) << std::endl 
// #define jechot(...) jeff::os_putter(std::cout, #__VA_ARGS__, " = ", ( __VA_ARGS__ ), "; \t")
// #define jecho(...)  ( jeff::os_putter(std::cout << #__VA_ARGS__ << " = ", ( __VA_ARGS__ )) << std::endl )
// #define jechot(...) ( jeff::os_putter(std::cout << #__VA_ARGS__ << " = ", ( __VA_ARGS__ )) << "; \t"    )

