#pragma once

#include <iostream>


#include <glm/glm.hpp>




template<class... OsTypes>
inline std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, const glm::vec3& v) {
  return os << '(' << (v.x) << ", " << (v.y) << ", " << (v.z) << ')';
}
template<class... OsTypes>
inline std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, const glm::vec2& v) {
  return os << '(' << (v.x) << ", " << (v.y) << ')';
}
//
//namespace jeff {
//  template<class Os>
//  decltype(auto) os_putter(Os&& os, const glm::vec3& v) {
//    return std::forward<Os>(os) << v;
//  }
//  template<class Os>
//  decltype(auto) os_putter(Os&& os, const glm::vec2& v) {
//    return std::forward<Os>(os) << v;
//  }
//}
//
//namespace jos {
//  inline constexpr bool included_jlm = true;
//
//  template<class... OsTypes>
//  inline std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, const glm::vec3& v) {
//    return os << '(' << (v.x) << ", " << (v.y) << ", " << (v.z) << ')';
//  }
//  template<class... OsTypes>
//  inline std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, const glm::vec2& v) {
//    return os << '(' << (v.x) << ", " << (v.y) << ')';
//  }
//}