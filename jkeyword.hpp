#pragma once

#include <string_view>
#include <string>

#include <ostream>

#include "jfunc.hpp" // for jeff::hash



#include <vector> // temp for debug
#include "jecho.hpp" // temp for debug
#include "jutil.hpp" // temp for debug
#ifndef __FUNCSIG__
#define __FUNCSIG__ __PRETTY_FUNCTION__
#endif


// Specifies jkeyword and jkeyword_rt
// These can be used to have constexpr hashed strings, for use as keywords
// The hashing method is Daniel J. Bernstein's djb2, for no particular reason
// TODO: Documentation

// NOTE: Documentation for jeff::literals

//template<typename... Types>
// inline std::ostream& operator<<(std::ostream& os, std::vector<Types...> const& vec){
//   int ctr{};
//   for(auto&& e : vec)
//     os << (ctr++ == 0 ? "{ ": ", ") << e;
//   return os << (ctr==0 ? "{ }" : " }");
// }
 //template<class T>
 //void printVecWTF(const std
namespace jeff{


  


  // jkeyword_rt is a runtime version of jkeyword, used for creating keywords from
  // strings that are not compile-time constants

  class jkeyword_rt; // Forward declare for access to keyword's members in keyword_rt's constructors
  class jkeyword {
    friend class jkeyword_rt;
    /*
      Notes: keywords with null characters might not work as expected, because I have put no
             consideration into this circumstance
    */

    using value_type = uint32_t;

    static constexpr std::string_view uninitialized_str = std::string_view{"jeff_uninitialized"};

  private:
    //// Uses the hash function djb2, by Daniel J. Bernstein
    // static constexpr value_type make_hash(const std::string_view& sv) {
    //   value_type hash(5381);
    //   for (auto&& c : sv)
    //     hash = ((hash << 5) + hash) + c;
    //   return hash;
    // }
  protected:
  //public:
    value_type hash;
    std::string_view sv;


    // For use only by jkeyword_rt
    constexpr jkeyword(value_type hash, std::string_view sv) noexcept : hash{hash}, sv{sv} { }
    constexpr jkeyword(value_type hash) noexcept : hash{hash}, sv{} { }
  public:



    constexpr jkeyword(std::string_view sv) noexcept : hash{jeff::make_hash(sv)}, sv{sv} { }
    constexpr jkeyword() noexcept : hash{}, sv{} { }
    //constexpr keyword(value_type hash) noexcept : hash{hash}, sv{""} { }
    constexpr jkeyword(const jkeyword& that) noexcept : hash{that.hash}, sv{that.sv} {}
    constexpr jkeyword(jkeyword&& that) noexcept : hash{std::move(that.hash)}, sv{std::move(that.sv)} {}

    template<size_t N>
    constexpr jkeyword(const char(&str)[N]) noexcept : jkeyword{std::string_view{str, N-1}} {}

    jkeyword(std::string) = delete; // Because strings have dynamically allocated mem, this will not work
    //keyword(const char *) = delete; // Interferes with char array reference

    constexpr jkeyword& operator=(std::string_view sv) noexcept {
      // hash = make_hash(sv);
      hash = jeff::make_hash(sv);
      this->sv = sv;
      return *this;
    }
    template<size_t N>
    constexpr jkeyword& operator=(const char(&str)[N]) noexcept {
      return operator=(std::string_view{str, N-1});
    }

    constexpr jkeyword& operator=(const jkeyword& that) noexcept {
      hash = that.hash;
      sv = that.sv;
      return *this;
    }
    constexpr jkeyword& operator=(jkeyword&& that) noexcept {
      hash = std::move(that.hash);
      sv = std::move(that.sv);
      return *this;
    }

    jkeyword& operator=(std::string) = delete;

    constexpr operator value_type() const noexcept { return hash; }


    friend constexpr bool operator==(jkeyword lhs, jkeyword rhs) noexcept { return lhs.hash == rhs.hash; }
    friend constexpr bool operator!=(jkeyword lhs, jkeyword rhs) noexcept { return lhs.hash != rhs.hash; }
    friend constexpr bool operator<=(jkeyword lhs, jkeyword rhs) noexcept { return lhs.hash <= rhs.hash; }
    friend constexpr bool operator>=(jkeyword lhs, jkeyword rhs) noexcept { return lhs.hash >= rhs.hash; }
    friend constexpr bool operator< (jkeyword lhs, jkeyword rhs) noexcept { return lhs.hash <  rhs.hash; }
    friend constexpr bool operator> (jkeyword lhs, jkeyword rhs) noexcept { return lhs.hash >  rhs.hash; }

    // friend std::ostream& operator<<(std::ostream& os, const jkeyword& kw);
    friend std::ostream& operator<<(std::ostream& os, const jkeyword& kw){
      return os << '[' << kw.hash << " : \"" << kw.sv << "\"]";
    }

    // TODO: Figure out if and how to make a tuple of rvalue refs from a temporary keyword

    // IDEA: https://en.cppreference.com/w/cpp/utility/optional/operator*
    // Seems like you can specialize for reference types?
    // https://en.cppreference.com/w/cpp/language/member_functions

  };

  namespace literals{
    constexpr jkeyword operator "" _kw(const char* str, std::size_t len) noexcept {
      //constexpr jkeyword operator "" _jk(const char* str, std::size_t len) noexcept {
      return jkeyword(std::string_view{str, len});
      // return jkeyword{std::string_view{str, len}};
      //using namespace std; // BUG IN MSVC! Thinks calling the sv converter is a declaration or something
      //return keyword{std::operator"" sv(str, len)};
    }
  }



  




  /* A derivation of struct keyword for use without compile-time constants
      This is necessary because a string_view is really nothing but a pointer and size.
      With this, we are able to allow for keyword functionality generalized to runtime
      generation while also keeping the full constexpr-ness of the keyword working.
      This has not been thorougly tested and may need some extentions and deletions for
      proper functionaliy
  */
  class jkeyword_rt : public jkeyword {
  protected:
    std::string str;


    //static constexpr bool error_tracking = false;
    //
    //std::vector<std::string> history;
    ////std::conditional_t<error_tracking, std::vector<std::string>, std::false_type> history;
    ////std::vector<std::string> history;
    //template<class T = void>
    //std::enable_if_t<error_tracking, T>
    //hist(std::string msg) {
    //  if constexpr (error_tracking) {
    //    history.emplace_back(msg);
    //  }
    //}
  public:

    jkeyword_rt(const std::string& str) {
      this->str = str;
      jkeyword&& tmp{static_cast<std::string_view>(this->str)};
      hash = std::move(tmp.hash);
      sv = std::move(tmp.sv);
      //if constexpr (error_tracking) { check(__FUNCSIG__); hist(__FUNCSIG__); }
    }
    jkeyword_rt(std::string&& str) {
      this->str = std::move(str);
      jkeyword&& tmp{static_cast<std::string_view>(this->str)};
      hash = std::move(tmp.hash);
      sv = std::move(tmp.sv);
      //if constexpr (error_tracking) { check(__FUNCSIG__); hist(__FUNCSIG__); }
    }
    jkeyword_rt(const char * str) : jkeyword_rt(std::string(str)) {
      //if constexpr (error_tracking) { check(__FUNCSIG__); hist(__FUNCSIG__); }
    }


    //jkeyword_rt(const jkeyword_rt& that) : jkeyword(hash, sv) {
    jkeyword_rt(const jkeyword_rt& that) : jkeyword{that.hash} {
      str = that.str;
      sv = std::string_view(str);

      //if constexpr (error_tracking) { check(__FUNCSIG__); hist(__FUNCSIG__); }
    }
    jkeyword_rt(jkeyword_rt&& that) noexcept  : jkeyword{std::move(that.hash)}  {
      // hash = that.hash;
      // sv = that.sv;
      //str = that.str;
      //hash = std::move(that.hash);
      str = std::move(that.str);
      sv = std::string_view(str);

      //if constexpr (error_tracking) { check(__FUNCSIG__); hist(__FUNCSIG__); }
    }
    ////jkeyword_rt(jkeyword_rt&& that) noexcept = default;
    //jkeyword_rt(jkeyword_rt&& that) noexcept :
    //  jkeyword{std::move(that.hash), std::move(that.sv)},
    //  str{std::move(that.str)}   {
    //  
    //  check(__FUNCSIG__);
    //  hist(__FUNCSIG__);
    //}
    // {
    //   hash = std::move(that.hash);
    //   sv = std::move(that.sv);
    //   str = std::move(that.str);
    // }
    // jkeyword_rt(jkeyword_rt&& that) noexcept {
    //   hash = std::move(that.hash);
    //   sv = std::move(that.sv);
    //   str = std::move(that.str);
    // }

    jkeyword_rt(const jkeyword&) = delete;
    jkeyword_rt(jkeyword&&) = delete;

    jkeyword_rt& operator=(const jkeyword_rt& that) {
      hash = that.hash;
      sv = that.sv;
      str = that.str;
      //if constexpr (error_tracking) { check(__FUNCSIG__); hist(__FUNCSIG__); }
      return *this;
    }
    jkeyword_rt& operator=(jkeyword_rt&& that) noexcept {
      if(this != &that){
        hash = std::move(that.hash);
        sv = std::move(that.sv);
        str = std::move(that.str);
      }
      //if constexpr (error_tracking) { check(__FUNCSIG__); hist(__FUNCSIG__); }
      return *this;
    }

    jkeyword_rt& operator=(const std::string& str) {
      this->str = str;
      jkeyword&& tmp{static_cast<std::string_view>(this->str)};
      hash = std::move(tmp.hash);
      sv = std::move(tmp.sv);
      //if constexpr (error_tracking) { check(__FUNCSIG__); hist(__FUNCSIG__); }
      return *this;
    }
    jkeyword_rt& operator=(std::string&& str) noexcept {
      this->str = std::move(str);
      jkeyword&& tmp{static_cast<std::string_view>(this->str)};
      hash = std::move(tmp.hash);
      sv = std::move(tmp.sv);
      //if constexpr (error_tracking) { check(__FUNCSIG__); hist(__FUNCSIG__); }
      return *this;
    }


    jkeyword_rt& operator=(jkeyword) = delete;

    // The most important bit, allow for implicit casting directly to a keyword 
    // TODO: Will this implicitly cast?
    // constexpr operator jkeyword() const noexcept {
    //   return static_cast<jkeyword>(*this);
    //   //return *this;
    // }


    const std::string& value() const noexcept { return str; }

    // friend std::ostream& operator<<(std::ostream& os, const jkeyword_rt& kw);
    // friend std::ostream& operator<<(std::ostream& os, const jkeyword_rt& kw){
    //   return os << static_cast<jkeyword>(*this);
    //   // return os << (operator jkeyword());
    // }

    friend std::ostream& operator<<(std::ostream& os, const jkeyword_rt& kw){
      //if constexpr(jkeyword_rt::error_tracking)          kw.check();
      return os << '[' << kw.hash << " : \"" << kw.value() << "\"]";
    }


    ////void check(std::string_view loc = "") const {
    //void check(std::string loc = "") const{
    //  if constexpr (error_tracking) {
    //    static std::vector<std::string_view> locs;

    //    if (sv.data() != str.data()) {
    //      jecho(loc);
    //      std::cerr << "DATA MISMATCH" << std::endl;
    //      jecho(jfmt::l(history));
    //      //std::cerr << "\t" << locs.size() << std::endl;
    //      jecho(locs);

    //      throw "cunt";
    //    }
    //    if (sv.size() != str.size()) {
    //      jecho(loc);

    //      std::cerr << "SIZE MISMATCH" << std::endl;
    //      jecho(jfmt::l(history));
    //      //std::cerr << "\t" << locs << std::endl;
    //      jecho(locs);
    //      throw "m8 u srs?";
    //    }

    //    locs.emplace_back(loc);
    //  }
    //}

  };




  constexpr jkeyword make_keyword(std::string_view sv){
    return {sv};
  }
  template<std::size_t N>
  constexpr jkeyword make_keyword(const char (&str)[N]){
    return make_keyword(std::string_view(str, N-1));
  }
  inline jkeyword_rt make_keyword(std::string str){
    return {str};
  }


  // inline void check(const jkeyword_rt& kw){
  //   //kw.check();
  // }


  using keyword = jkeyword;
  using keyword_rt = jkeyword_rt;

} // end namespace jeff