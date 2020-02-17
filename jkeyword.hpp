#pragma once

#include <string_view>
#include <string>



// Specifies jkeyword and jkeyword_rt
// TODO: Documentation

// NOTE: Documentation for jeff::literals

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
    static constexpr value_type make_hash(const std::string_view& sv) {
      value_type hash(5381);
      for (auto&& c : sv)
        hash = ((hash << 5) + hash) + c;
      return hash;
    }
  protected:
  //public:
    value_type hash;
    std::string_view sv;


    // For use only by jkeyword_rt
    constexpr jkeyword(value_type hash, std::string_view sv) noexcept : hash{hash}, sv{sv} { }

  public:



    constexpr jkeyword(std::string_view sv) noexcept : hash{make_hash(sv)}, sv{sv} { }
    constexpr jkeyword() noexcept : hash{}, sv{} { }
    //constexpr keyword(value_type hash) noexcept : hash{hash}, sv{""} { }
    constexpr jkeyword(const jkeyword& that) noexcept : hash{that.hash}, sv{that.sv} {}
    constexpr jkeyword(jkeyword&& that) noexcept : hash{std::move(that.hash)}, sv{std::move(that.sv)} {}

    template<size_t N>
    constexpr jkeyword(const char(&str)[N]) noexcept : jkeyword{std::string_view{str, N-1}} {}

    jkeyword(std::string) = delete; // Because strings have dynamically allocated mem, this will not work
    //keyword(const char *) = delete; // Interferes with char array reference

    constexpr jkeyword& operator=(std::string_view sv) noexcept {
      hash = make_hash(sv);
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
  public:

    jkeyword_rt(const std::string& str) {
      this->str = str;
      jkeyword&& tmp{static_cast<std::string_view>(this->str)};
      hash = std::move(tmp.hash);
      sv = std::move(tmp.sv);
    }
    jkeyword_rt(std::string&& str) {
      this->str = std::move(str);
      jkeyword&& tmp{static_cast<std::string_view>(this->str)};
      hash = std::move(tmp.hash);
      sv = std::move(tmp.sv);
    }
    jkeyword_rt(const char * str) : jkeyword_rt(std::string(str)) {  }


    jkeyword_rt(const jkeyword_rt& that) : jkeyword(hash, sv) {
      // hash = that.hash;
      // sv = that.sv;
      str = that.str;
    }
    jkeyword_rt(jkeyword_rt&& that) noexcept {
      hash = std::move(that.hash);
      sv = std::move(that.sv);
      str = std::move(that.str);
    }

    jkeyword_rt(const jkeyword&) = delete;
    jkeyword_rt(jkeyword&&) = delete;

    jkeyword_rt& operator=(const jkeyword_rt& that) {
      hash = that.hash;
      sv = that.sv;
      str = that.str;
      return *this;
    }
    jkeyword_rt& operator=(jkeyword_rt&& that) noexcept {
      hash = std::move(that.hash);
      sv = std::move(that.sv);
      str = std::move(that.str);
      return *this;
    }

    jkeyword_rt& operator=(const std::string& str) {
      this->str = str;
      jkeyword&& tmp{static_cast<std::string_view>(this->str)};
      hash = std::move(tmp.hash);
      sv = std::move(tmp.sv);
      return *this;
    }
    jkeyword_rt& operator=(std::string&& str) noexcept {
      this->str = std::move(str);
      jkeyword&& tmp{static_cast<std::string_view>(this->str)};
      hash = std::move(tmp.hash);
      sv = std::move(tmp.sv);
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

  };

} // end namespace jeff