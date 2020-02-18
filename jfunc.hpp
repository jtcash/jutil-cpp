#pragma once

#include <string_view>


namespace jeff{

  namespace helper{
    // uint32_t 
  }// end namespace helper


  // Unspecialized catch-all
  template<class...>
  struct hash;


  template<>
  struct hash<std::string_view>{

  };


} // End namespace jeff