#pragma once

#include <iostream>

#include <utility>
#include <tuple>

#include <glm/glm.hpp>



#include "jtype.hpp"
#include "jout.hpp" // for tabulator
#include "jfunc.hpp" // for sum 



//// Convert types to tuples
namespace jeff {

  namespace helper {

    /** glm vectors to tuple */

    template<auto N, class T, auto Q, class IdxType, IdxType... Idxs>
    constexpr auto as_tuple(const glm::vec<N, T, Q>& v, std::integer_sequence<IdxType, Idxs...> idxs) {
      return std::make_tuple(v[Idxs]...);
    }
    template<auto N, class T, auto Q>
    constexpr auto as_tuple(const glm::vec<N, T, Q>& v) {
      return as_tuple(v, std::make_integer_sequence<decltype(N), N>{});
    }


    /** glm matrices to tuples of tuples */
    template<auto N, auto M, class T, auto Q, class IdxType, IdxType... Idxs>
    constexpr auto as_tuple(const glm::mat<N, M, T, Q>& m, std::integer_sequence<IdxType, Idxs...> idxs) {
      return std::make_tuple(as_tuple(m[Idxs])...);
    }

    template<auto N, auto M, class T, auto Q>
    constexpr auto as_tuple(const glm::mat<N, M, T, Q>& m) {
      return as_tuple(m, std::make_integer_sequence<decltype(N), N>{});
    }

  }// end helper

  template<class T>
  constexpr auto as_tuple(T&& t) {
    return helper::as_tuple(std::forward<T>(t));
  }
} // end jeff



/** ALLOW STRUCTURED BINDINGS */

/// This was useful
namespace std {
  template <auto N, class T, auto Q>
  struct tuple_size<glm::vec<N, T, Q>> : std::integral_constant<std::size_t, N> { };

  template <std::size_t Idx, auto N, class T, auto Q>
  struct tuple_element<Idx, glm::vec<N, T, Q>> {
    using type = typename glm::vec<N, T, Q>::value_type;
  };



  template< std::size_t I, auto N, class T, auto Q>
  constexpr auto& get(glm::vec<N, T, Q>& v) {
    static_assert(I < N, "Index out of bounds in std::get of glm::vec");
    return v[I];
  }
  template< std::size_t I, auto N, class T, auto Q>
  constexpr const auto& get(const glm::vec<N, T, Q>& v) {
    static_assert(I < N, "Index out of bounds in std::get of glm::vec");
    return v[I];
  }

  template< std::size_t I, auto N, class T, auto Q>
  constexpr auto&& get(glm::vec<N, T, Q>&& v) {
    static_assert(I < N, "Index out of bounds in std::get of glm::vec");
    return std::move(v[I]);
  }



  template <auto N, auto M, class T, auto Q>
  struct tuple_size<glm::mat<N, M, T, Q>> : std::integral_constant<std::size_t, N> { };

  template <std::size_t Idx, auto N, auto M, class T, auto Q>
  struct tuple_element<Idx, glm::mat<N, M, T, Q>> {
    using type = typename glm::mat<N, M, T, Q>::col_type;
  };



  template< std::size_t I, auto N, auto M, class T, auto Q>
  constexpr auto& get(glm::mat<N, M, T, Q>& m) {
    static_assert(I < N, "Index out of bounds in std::get of glm::mat");
    return m[I];
  }
  template< std::size_t I, auto N, auto M, class T, auto Q>
  constexpr const auto& get(const glm::mat<N, M, T, Q>& m) {
    static_assert(I < N, "Index out of bounds in std::get of glm::mat");
    return m[I];
  }

  template< std::size_t I, auto N, auto M, class T, auto Q>
  constexpr auto&& get(glm::mat<N, M, T, Q>&& m) {
    static_assert(I < N, "Index out of bounds in std::get of glm::mat");
    return std::move(m[I]);
  }

}



namespace jeff {
  template <auto N, class T, auto Q>
  constexpr T dot(const glm::vec<N, T, Q>& a, const glm::vec<N, T, Q>& b) {
    return sum(as_tuple(a*b));
  }
}// end jeff


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

  return os << tab('|', '-');
  
}


namespace jeff {
  struct homomat4 : public glm::mat4x3 {
    using mat4x3 = glm::mat4x3;
    using col_type = typename mat4x3::col_type;
    //using col_type = typename glm::mat4x4::col_type;
    using row_type = typename mat4x3::row_type;


    using mat4x3::mat4x3;
    using mat4x3::length;
    using mat4x3::operator[];
    using mat4x3::operator=;
    //// NOTE: might not want to inherit arithmetic assignments
    //using mat4x3::operator+=; 
    //using mat4x3::operator-=;
    //using mat4x3::operator*=; // THIS IS WHAT WE ARE CHANGING
    //using mat4x3::operator/=;
  };
}

namespace std {
  template <>
  struct tuple_size<jeff::homomat4> : std::integral_constant<std::size_t, 4> { };

  template <std::size_t Idx>
  struct tuple_element<Idx, jeff::homomat4> {
    using type = typename jeff::homomat4::col_type;
  };



  template< std::size_t I>
  constexpr auto& get(jeff::homomat4& m) {
    static_assert(I < N, "Index out of bounds in std::get of glm::mat");
    return m[I];
  }
  template< std::size_t I>
  constexpr const auto& get(jeff::homomat4& m) {
    static_assert(I < N, "Index out of bounds in std::get of glm::mat");
    return m[I];
  }

  template< std::size_t I>
  constexpr auto&& get(jeff::homomat4& m) {
    static_assert(I < N, "Index out of bounds in std::get of glm::mat");
    return std::move(m[I]);
  }
}//end std

inline glm::vec4 operator*(const jeff::homomat4& m, const glm::vec4& v){
  return glm::vec4{
    *reinterpret_cast<const glm::mat4x3*>(&m) * v,
    v.w
  };
}


constexpr jeff::homomat4 operator*(const jeff::homomat4& A, const jeff::homomat4& B) {

  const auto& [c0, c1, c2, c3] = B;

  glm::vec3 r0{A[0][0], A[1][0], A[2][0]};
  glm::vec3 r1{A[0][1], A[1][1], A[2][1]};
  glm::vec3 r2{A[0][2], A[1][2], A[2][2]};


  /*glm::vec3 outC0{r0*c0, r1*c0, r2*c0};
  glm::vec3 outC1{r0*c1, r1*c1, r2*c1};
  glm::vec3 outC2{r0*c2, r1*c2, r2*c2};
  glm::vec3 outC3{r0*c3 + A[3][0], r1*c3 +A[3][1], r2*c3 + A[3][2]};*/

  glm::vec3 outC0{jeff::dot(r0,c0), jeff::dot(r1,c0), jeff::dot(r2,c0)};
  glm::vec3 outC1{jeff::dot(r0,c1), jeff::dot(r1,c1), jeff::dot(r2,c1)};
  glm::vec3 outC2{jeff::dot(r0,c2), jeff::dot(r1,c2), jeff::dot(r2,c2)};
  glm::vec3 outC3{jeff::dot(r0,c3) + A[3][0], jeff::dot(r1,c3) +A[3][1], jeff::dot(r2,c3) + A[3][2]};


  return{
   /* r0*c0, r1*c0, r2*c0,
    r0*c1, r1*c1, r2*c1,
    r0*c2, r1*c2, r2*c2,
    r0*c3 + A[3][0], r1*c3 +A[3][1], r2*c3 + A[3][2]*/
    outC0, outC1, outC2, outC3
  };
  



  //return {};
}
























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