#pragma once

#include <iostream>

#include <utility>

#include <glm/glm.hpp>



#include "jtype.hpp"
#include "jout.hpp" // for tabulator


// template<class... OsTypes>
// inline std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, const glm::vec4& v) {
//   return os << '(' << (v.x) << ", " << (v.y) << ", " << (v.z) << ", " << (v.w) << ')';
// }
// template<class... OsTypes>
// inline std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, const glm::vec3& v) {
//   return os << '(' << (v.x) << ", " << (v.y) << ", " << (v.z) << ')';
// }
// template<class... OsTypes>
// inline std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, const glm::vec2& v) {
//   return os << '(' << (v.x) << ", " << (v.y) << ')';
// }


namespace jeff{
  template<class... OsTypes, auto N, class T, auto Q, class Sep>
  inline std::basic_ostream<OsTypes...>& glm_printer(std::basic_ostream<OsTypes...>& os, 
            const glm::vec<N, T, Q>& v,
            Sep&& sep) {
    os << v[0];
    if constexpr (N > 1)
      for (auto n = 1; n<N; ++n)
        os << std::forward<Sep>(sep) << v[n];
    return os;
  }
}
template<class... OsTypes, auto N, class T, auto Q>
inline std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, const glm::vec<N,T,Q>& v) {
  return jeff::glm_printer(os << "< ", v, ", ") << " >";
}



// M rows; N columns 
template<class... OsTypes, auto N, auto M, class T, auto Q>
inline std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, const glm::mat<N, M, T, Q>& mat) {
  using size_type = typename glm::mat<N,M,T,Q>::length_type;
  jeff::tabulator tab;
  for (size_type m=0; m<M; ++m) {
    for (size_type n=0; n<N; ++n) {
      tab << mat[n][m];
    }
    +tab;
  }

  return os << tab;
  
}
////
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
  inline constexpr void parse_vec3(Toki& toki, Dest& dest) {
    using float_type = Dest::value_type;
    auto parsed = toki.getTokensAs<float_type, float_type, float_type>();
    if (!parsed)
      throw std::runtime_error("Failed to parse vec3");
    dest = std::make_from_tuple<glm::vec3>(*parsed);
  }
  template<class Toki, class Dest>
  inline constexpr void parse_vec2(Toki& toki, Dest& dest) {
    using float_type = Dest::value_type;
    auto parsed = toki.getTokensAs<float_type, float_type>();
    if (!parsed)
      throw std::runtime_error("Failed to parse vec2");
    dest = std::make_from_tuple<glm::vec2>(*parsed);
  }
  template<class Toki, class Dest>
  inline constexpr void parse_vec4(Toki& toki, Dest& dest) {
    using float_type = Dest::value_type;
    auto parsed = toki.getTokensAs<float_type, float_type, float_type, float_type>();
    if (!parsed)
      throw std::runtime_error("Failed to parse vec4");
    dest = std::make_from_tuple<glm::vec4>(*parsed);
  }

  template<class Toki, class Dest>
  inline constexpr void parse_vec(Toki& toki, Dest& dest) {
    if constexpr (jeff::is_same_nocvref_v<Dest, glm::vec2>)
      return parse_vec2(toki, dest);
    else if constexpr (jeff::is_same_nocvref_v<Dest, glm::vec3>)
      return parse_vec3(toki, dest);
    else if constexpr (jeff::is_same_nocvref_v<Dest, glm::vec4>)
      return parse_vec4(toki, dest);
    else
      throw "TODO: Implement other types";
  }

}// end jeff