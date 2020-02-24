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



namespace jeff {

  template<class Toki, class Dest>
  constexpr static void parse_vec3(Toki& toki, Dest& dest) {
    using float_type = Dest::value_type;
    auto parsed = toki.getTokensAs<float_type, float_type, float_type>();
    if (!parsed)
      throw std::runtime_error("Failed to parse vec3");
    dest = std::make_from_tuple<glm::vec3>(*parsed);
  }
  template<class Toki, class Dest>
  constexpr static void parse_vec2(Toki& toki, Dest& dest) {
    using float_type = Dest::value_type;
    auto parsed = toki.getTokensAs<float_type, float_type>();
    if (!parsed)
      throw std::runtime_error("Failed to parse vec2");
    dest = std::make_from_tuple<glm::vec2>(*parsed);
  }
}// end jeff