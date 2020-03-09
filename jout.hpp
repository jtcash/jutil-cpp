#pragma once



#include <cstddef> // for std::byte

#include <utility>
#include <ostream>
#include <array>

#include <tuple>
#include <string_view>
#include <string>

// for development

#include <type_traits>


#include <vector>
#include <iostream>

#include <functional>

#include <numeric>

#include <sstream> // for tabulate

#include "jinfo.hpp"
#include "jtype.hpp"
// #include "jutil.hpp"







/// TODO: Documentation



/// NOTE: Also defines namespace jfmt




namespace jeff{
  // For development purposes; marks types with no os_putter specialization
  inline constexpr bool indicate_unspecialized_os_putter = !true;




  // TODO: Renaming
  // Goal to have something like https://en.cppreference.com/w/cpp/io/manip/endl
  // but flexible. This is just an alternative to using std::function or somethign
  namespace helper{
    template<class Func>
    struct lambda_carrier_t{
      const Func f;
      //std::basic_ostream<CharT, Traits>& endl( std::basic_ostream<CharT, Traits>& os );
      template<class Os>
      friend Os& operator<<(Os& os, const lambda_carrier_t& lc){
        return lc.f(os);
      }
    };
  }// end namespace helper

  template<class Func>
  constexpr decltype(auto) carry_lambda(Func f) noexcept{
    return helper::lambda_carrier_t<Func>{f};
  }

} // end namespace jeff






/** jfmt: class for ostream put formatting
 *
 * methods:
 *  x:  output as hex (WIP, uses lazy and unreliable solution) TODO: better impl
 *
 *  q:  Quoted. default to q<'"','"'>(arg), which wraps arg in '"' while putting into os
 *        These quote characters can be changed with the provided template specializations
 *        examples:
 *            os << q(expression);      // same as doing os << '"' << expression << '"'
 *            os << q<'|'>(10);         // same as doing os << '|' << 10 << '|'
 *            os << q<'{','}'>("e.g."); // same as doing os << '{' << "e.g." << '}'
 *
 *  l:  As list: prints each element from a range-expression on its own line, with index at start of line
 *
 *  t:  Thousands separator: for integral types, print the integer with thousands separators. TODO: floating types
 *
 *  pl: Pad left  TODO: details
 *  pr: pad right TODO: details
 *
 *  g: Group into something like "(a,b,c)""
 *  e: escaped TODO: impl and details
 *
 * */
namespace jfmt{
  using namespace jeff;

  // temp, jank solution and example of carry_lambda
  /*** x ***/
  template<class T>
  constexpr decltype(auto) x(T t) noexcept{ // by value, as this should only take scalars and is temporary anyway
    return carry_lambda([t](auto& os) -> decltype(os){
      return os << reinterpret_cast<const void*>(t);
    });
  }


  // q formatter for quoting allows specifying the quote character used


  /*** q ***/
  namespace helper{
    template<class FrontT, class BackT, class T>
    constexpr decltype(auto) q(FrontT front, BackT back, const T& t ) noexcept { // NOTE: const lval ref for capture reason
      return carry_lambda([=, &t](auto& os) -> decltype(os){
        return os << front << t << back;
      });
    }
  } // end namespace helper

  template<char Front, char Back, class T>
  constexpr decltype(auto) q(const T& t) noexcept{
    return helper::q(Front, Back, t);
  }
  template<char Wrapper, class T>
  constexpr decltype(auto) q(const T& t) noexcept{
    return q<Wrapper, Wrapper>(t);
  }
  template<class T>
  constexpr decltype(auto) q(const T& t) noexcept{
    return q<'"'>(t);
  }



  /*** l ***/ // LIST

  template<class T>
  constexpr decltype(auto) l(const T& t) noexcept { // NOTE: const lval ref for capture reason
    return carry_lambda([=, &t](auto& os) -> decltype(os){
      std::size_t ctr{};
      for(auto&& e : t)
        os << "\n  " << (ctr++) << ":\t" << e;
      return os << '\n';
    });
  }



  /*** t ***/ // Thousands separator
  namespace helper{
    namespace detail{
      template<class T> // No type check needed, as this is only called by put_thousands
      inline std::ostream& put_thousands_helper(std::ostream& os, T t){
        if(t >= T(1000)){
          const auto r = t%T(1000);
          using namespace std::literals;
          return put_thousands_helper(os, t/T(1000)) << ',' << "00"sv.substr(r<T(100)?(r<T(10)?0:1):2) << r;
        }
        return t ? os<<t : os;
      }
    }//end detail

    template<class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    inline std::ostream& put_thousands(std::ostream& os, T t){
      if(t == 0)
        return os << '0';
      else if(t < 0)
        return detail::put_thousands_helper(os << '-', -t);
      return detail::put_thousands_helper(os, t);
    }
  }// End helper


  template<class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
  constexpr decltype(auto) t(T t) noexcept{ // by value, as this should only take scalars and is temporary anyway
    return carry_lambda([t](auto& os) -> decltype(os){
      return helper::put_thousands(os, t);
    });
  }




  // pad method can be improved
  namespace helper{
    inline std::ostream& put_pad(std::ostream& os, std::size_t npad, char padc){
      return os << std::string(npad, padc);
    }
  }// end helper

  /*** pl ***/ // pad the left

  template<class T>
  constexpr decltype(auto) pl(const T& t, std::size_t npad, char padc = ' ') noexcept{ // by value, as this should only take scalars and is temporary anyway
    return carry_lambda([=, &t](auto& os) -> decltype(os){
      return helper::put_pad(os, npad, padc) << t;
    });
  }

  /*** pr ***/ // pad the right

  template<class T>
  constexpr decltype(auto) pr(const T& t, std::size_t npad, char padc = ' ') noexcept{ // by value, as this should only take scalars and is temporary anyway
    return carry_lambda([=, &t](auto& os) -> decltype(os){
      return helper::put_pad(os << t, npad, padc);
    });
  }



/// /NEW

  constexpr decltype(auto) eq(char c) noexcept{
    return carry_lambda([c](auto& os) -> decltype(os){
      return os << jeff::escaped_quoted(c);
      // return os << ( '\'' + jeff::escape(c) + '\'' );
    });
  }

  constexpr decltype(auto) eq(std::string_view sv) noexcept{
    return carry_lambda([sv](auto& os) -> decltype(os){
      return os << jeff::escaped_quoted(sv);
    });
  }


  namespace helper{
    template<class... Types>
    std::ostream& put_group(std::ostream& os, Types&&... types){ // should do const lvalue instead?
      std::size_t n{};
      os << '(';

      ( ( (n++ ? os << "," : os) << types   ), ...);


      return os << ')';
    }
  }// end helper


  template<class... Types>
  constexpr decltype(auto) g(const Types&... types) noexcept{ // by value, as this should only take scalars and is temporary anyway
    return carry_lambda([&types...](auto& os) -> decltype(os){
      return helper::put_group(os, types...);
    });
  }

} // end namespace jfmt

// Allow resolution of jfmt::helper::put_thousands in jeff
namespace jeff{
  using jfmt::helper::put_thousands;
}// end jeff






/// /// NOTE: Move  to jtype
namespace jeff{
  static_assert(sizeof(char) == 1, "I only made this lib to work with sizeof(char)=1");

  // constexpr bool is_printable(char c){
  //   return c <= '~' && c >= ' ';
  // }
  constexpr bool is_printable(std::byte b){
    return is_print(std::to_integer<char>(b));
    // return is_printable(std::to_integer<char>(b));
  }

  constexpr char nibble_to_hex(std::byte nib){
    return "0123456789abcdef"[std::to_integer<std::size_t>(nib & std::byte(0xf))];
  }
  constexpr std::array<char, 3> byte_to_hex(std::byte b){
    return { nibble_to_hex(b>>4), nibble_to_hex(b), '\0'};
  }

// template< class CharT, class Traits >
// std::basic_ostream<CharT, Traits>& endl( std::basic_ostream<CharT, Traits>& os );
	namespace helper{
    constexpr decltype(auto) byte_to_hex_for_os(std::byte b){
      return carry_lambda([b](auto& os) -> decltype(os){
        return os << nibble_to_hex(b>>4) << nibble_to_hex(b);
        // return os << "pls" << std::to_integer<uint32_t>(b);
      });
    }
  }
  template<class T, std::enable_if_t<sizeof(T)==1, int> = 0>
  constexpr decltype(auto) os_put_hex(T t){
    return helper::byte_to_hex_for_os(std::byte{t});
  }

}



//// std::ostream << std::tuple

//////////////////////////////////////////////////////////////////////////////////////////////
//// https://en.cppreference.com/w/cpp/utility/apply                                      ////
//// HOLY SHIT, PARAMETER PACK EXPANSION IS INCREDIBLE!!!!                                ////
//// https://en.cppreference.com/w/cpp/language/parameter_pack                            ////
////                                                                                      ////
//// WOW! I should use the comma operator more often!                                     ////
//// https://en.cppreference.com/w/cpp/language/operator_other#Built-in_comma_operator    ////
//////////////////////////////////////////////////////////////////////////////////////////////
template<class... OsTypes, typename... Types>
static std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os,
                                                  std::tuple<Types...> const& theTuple){
  std::apply(
    [&os](Types const&... tupleArgs){
        os << "[ ";
        std::size_t n{0};
        (  (os << tupleArgs << (++n != sizeof...(Types) ? ", " : " ]")), ...  );
    },
    theTuple
  );
  return os;
}

template<class... OsTypes, typename... Types>
inline std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, std::vector<Types...> const& vec){
  int ctr{};
  for(auto&& e : vec)
    os << (ctr++ == 0 ? "{ ": ", ") << e;
  return os << (ctr==0 ? "{ }" : " }");
}


template<class... OsTypes, std::size_t N>
inline std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, const std::array<uint8_t, N>& arr){
  os << "{ ";
  for(auto&& e : arr)
    os << jeff::os_put_hex(e) << ' ';

  return os << '}';
}



//// TODO: WHY DIDN'T I OVERLOAD FOR ALL STD::ARRAYS? /// OH! NEVERMIND 
template<class... OsTypes, std::size_t N>
inline std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, const std::array<char, N>& arr){
  os << "{ ";
  std::size_t ctr{};
  for(auto&& e : arr)
    (++ctr>1 ? os<<", " : os) <<  ("'" + jeff::escape(e) + "'");
    // os <<  ("'" + jeff::escape(e) + "'");

  return os << '}';
}

template<class... OsTypes>
static std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, const unsigned char* bp){
  return os << (const void*)(bp);
}

template<class... OsTypes>
static std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, unsigned char* bp){
  return os << (void*)(bp);
}


inline std::ostream& operator<<(std::ostream& os, const unsigned char* bp){
  return os << (void*)(bp);
}





/// RESOLVED: mostly i think?
// NOTE: To fix, is should have a specific namespace for ostream operator overloads, that way I can
// just use that namespace before a put that requires the overload. Or maybe even have a using namespace
// directive for that namespace by default.
// I need to look into the details, but either GCC or MSVC behaves improperly.
// MSVC does not look outside of the current namespace for overload, while GCC does
// this makes me think that MSVC is wrong because why the hell would you not be able to resolve
// the ostream put operator overload if you are inside of a namespace?

template<class... OsTypes, class T, class U>
inline std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, const std::pair<T,U>& p) {
  return os << '(' << p.first << ',' << p.second << ')';
}
template<class... OsTypes>
inline std::basic_ostream<OsTypes...>& operator<<(std::basic_ostream<OsTypes...>& os, bool b) {
  return os << (b ? "true" : "false");
}






namespace jeff{


  /** make tabular prints
   *
   *  add field with put operator (<<)
   *  add row blank row with unary + operator
   *  add row with the subscript operator, described at the bottom of this class
   *
   *  e.g.
   *    tabulator tab;
   *    tab << "these" << 9.123 << "will be on the first" << "row"
   *    +tab << 1234 << "now these are on the second row"
   *    +tab << "last row"
   *
   *  TODO: Genericize for putting into any stream type
   * */
  class tabulator{
      /// TODO: Make this configurable or something?
    inline static constexpr bool empty_row_is_divider = true;



  public:
    using row_type = std::vector<std::string>;

  private:
    std::vector<row_type> rows;
    std::vector<std::size_t> widths;

  private:
    tabulator& end_row(){
      rows.emplace_back();
      return *this;
    }

    tabulator& push_field(std::string str){
      auto& currentRow = rows.empty() ? rows.emplace_back() : rows.back();
      auto currentField = currentRow.size();


      // Set the appropriate width field if needed
      if(currentField >= widths.size())
        widths.emplace_back(str.size());
      else
        widths[currentField] = std::max(widths[currentField], str.size());

      // Move the string into the row
      currentRow.emplace_back(std::move(str));

      return *this;
    }

    tabulator& escape_and_push_field(std::string str){
      return push_field(jeff::escaped(str));
    }
  public:

    tabulator& operator<<(std::string str){
      return escape_and_push_field(str);
    }
    tabulator& operator<<(char c){
      // return push_field(jeff::escape(c));
      return push_field('\'' + jeff::escape(c) + '\'');
    }


    decltype(auto) size() const noexcept{
      return rows.size();
    }


    template<class T>
    tabulator& operator<<(T&& t){
      using ::operator<<; // make MSVC a happi boi

      std::ostringstream ss;
      ss << std::forward<T>(t);
      // return push_field(std::move(ss).str()); // :( this isnt useful until c++20
      // return push_field(std::move(*ss.rdbuf()).str()); // Equivalent orkaround to avoid copy
      return escape_and_push_field(std::move(*ss.rdbuf()).str());
    }



    // start a new row
    tabulator& operator+(){
      return end_row();
    }

    void clear(){
      rows.clear();
      widths.clear();
    }






  /**
   *  allow tabulation using a syntax like this
          jeff::tabulator t;
          t[0][1][2][3];
          t[4][5][6][7];
          t[8][9][10][11];
          t[12][13]["test"][15];

   * */
  private:
    class tab_helper{
      friend class tabulator;
      tabulator& owner;
      tab_helper(tabulator& owner) : owner{owner} { }
    public:
      ~tab_helper(){
        owner.end_row();
      }

      template<class T>
      tab_helper& operator[](T&& t){
        owner << std::forward<T>(t);
        return *this;
      }
    };
  public:

    template<class T>
    tab_helper operator[](T&& t){
      return tab_helper{operator<<(std::forward<T>(t))};
    }



/// TEMP: Start addition made on laptop

  private:
  /// TODO: think about this idea and decide whether or not to keep it
    tabulator& handleCharSpecial(char c){
      switch(c){
      case '\n':  return end_row();
      default:    return operator<<(c);
      }
    }
  public:
      /// GOAL: Have a special put operator that handles special values differently
    tabulator& operator<<=(char c){
      /// TODO: This is probably a bad idea, so evaluate it
      return  handleCharSpecial(c); 
    }
    
    
/// TEMP: End addition made on laptop


    tabulator& operator,(char c){ // tabulator& operator,(T&& t){
      return handleCharSpecial(c);
    }

    /// NOTE: This one is probably a bad idea, but I think the syntax looks pretty!
    //    e.g.  t << a, b, c, d;
    template<class T>
    tabulator& operator,(T&& t){
      return operator<<(std::forward<T>(t));
    }




  //// TODO: Get fancy with dynamic field separators and table borders
    std::ostream& os_putter(std::ostream& os) const{
      return print(os, "    ", '|', '-');
      // constexpr decltype(auto) field_sep = "    ";
      // for(const auto& row : rows){
      //   for(std::size_t i=0; i<row.size(); ++i){
      //     const auto& field = row[i];
      //     const auto width = widths[i];
      //     /// pad the right side for now          /// TODO: GET FANCY WITH PADDING OPTIONS
      //     (i==0 ? os : os<<field_sep)  << field << std::string(width - field.size(), ' ');
      //   }
      //   if(!row.empty())
      //     os << '\n';
      // }
      // return os;
    }

    friend std::ostream& operator<<(std::ostream& os, const tabulator& tab){
      return tab.os_putter(os << '\n');
    }



    /// TODO: Make it so that rows with missing fields still have a vsep
    /// right after the last printed field
    std::ostream& print(std::ostream& os, const std::string& sep, char vborder, char hborder) const{
      if(widths.empty())
        return os << "{[empty tabulator]}";

      /// NOTE: This could be made generic
      const std::string vborder_sep("  "); // Space between content and begin/end characters 

      const std::string vstart = vborder + vborder_sep; // Begin rows with this
      const std::string vstop = vborder_sep + vborder;  // End rows with this

      // The width of the content of a full row
      const std::size_t content_width = std::accumulate(widths.begin(), widths.end(), std::size_t{0})
                                        + (widths.size() - 1)*sep.size();
      // The width of an entire row
      const std::size_t total_width = content_width + vstart.size() + vstop.size();
      // Horizontal border line
      const std::string hline(total_width, hborder); 


      os << hline;

      // Write out the formatted rows
      for(std::size_t r=0; r<rows.size(); ++r){
        const auto& row = rows[r];
        
        const bool is_last_row = (r + 1 == rows.size());

        /// TODO: Integrate this chunk better
        if constexpr(empty_row_is_divider){
          if(row.empty() && !is_last_row){
            os << '\n' << vborder << std::string_view(hline).substr(2) << vborder;
            continue;
          }
        }
        /// Chunk ending here
        std::size_t row_width = 0;

        if(!(row.empty() && is_last_row)){ // skip last row if empty
        // if(!(row.empty() && r + 1 == rows.size())){ // skip last row if empty
          os << '\n' << vstart;
          for(std::size_t i=0; i<row.size(); ++i){
            const auto& field = row[i];

            if (i != 0){
              os << sep;
              row_width += sep.size();
            }
            /// TODO: Store space string to avoid this repeated allocation
            os << field << std::string(widths[i] - field.size(), ' ');

            row_width += widths[i];
          }
          if(row.size() < widths.size())
            os << std::string(content_width - row_width, ' ');
          os << vstop;
        }
      }
      return os << '\n' << hline << '\n';
    }


    private:
    class putter{
      friend class tabulator;
      tabulator& owner;
      const std::string& sep;
      char vborder;
      char hborder;
      putter(tabulator& owner, const std::string& sep, char vborder, char hborder) :
        owner{owner}, sep{sep}, vborder{vborder}, hborder{hborder}  {  }
      public:

      friend std::ostream& operator<<(std::ostream& os, const putter& putter){
        return putter.owner.print(os << '\n', putter.sep, putter.vborder, putter.hborder);
      }

    };
    public:

    putter operator()(const std::string& sep, char vborder = '|', char hborder = '-'){
      return {*this, sep, vborder, hborder};
    }
    auto operator()(char vborder, char hborder = '-'){
      static std::string default_sep{"    "};
      return operator()(default_sep, vborder, hborder);
    }
  };


} // end jeff








namespace jeff {
 
  template<class Os, class T>
  inline decltype(auto) os_putter(Os&& os, T&& t) {
    using ::operator<<; // holy shit fuck msvc
    // CREDIT: https://stackoverflow.com/a/32822359/6232717
    return std::forward<Os>(os) << std::forward<T>(t);
  }
  template<class Os>
  inline decltype(auto) os_putter(Os&& os, bool b) {
    using ::operator<<; // holy shit fuck msvc
    return std::forward<Os>(os) << (b ? "true" : "false");
  }


  template<class Os, class T, class U, class... Rest>
  static decltype(auto) os_putter(Os&& os, T&& t, U&& u, Rest&&... rest) {
    return os_putter(
      os_putter(std::forward<Os>(os), std::forward<T>(t)) << ", ", // separate, temp
      std::forward<U>(u),
      std::forward<Rest>(rest)...
    );
    //return std::forward<Os>(os) << std::forward<T>(t);
  }
}