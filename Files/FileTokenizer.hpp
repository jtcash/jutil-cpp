#pragma once




#include <charconv>

#include <iostream>

#include <utility>
#include <vector>
#include <array>

#include <string_view>
#include <string>

#include <functional>

#include <type_traits>
#include <algorithm>

#include <optional>

#include <cstdlib>

#include <memory> // shared_ptr


#include "FileReader.hpp" 


#include "../jutil.hpp"
#include "../jout.hpp"
#include "../jtype.hpp"


#include "../jkeyword.hpp"



// FileReader abstracts away chunked reads from a file or stream


namespace jeff{


  // Allow for different tokenizers with different in types
  struct TokenizerBase{
    static constexpr std::string_view default_delims{" \n\f\t\v\r"};

    virtual ~TokenizerBase() = default;
    virtual std::string_view getToken(std::string_view delimiters) = 0;
    virtual inline std::string_view getToken(){ return getToken(default_delims); }
    
    template<class... Args>
    inline jeff::jkeyword getKeyword(Args&&... args){
      return {getToken(std::forward<Args>(args)...)};
    }

    template<class AsType>
    [[nodiscard]]
    std::enable_if_t<is_one_of_v<AsType, int, float, double, long double>, 
    std::optional<AsType> > getTokenAs(){
      auto tok = getToken();
      return jeff::chars_to<AsType>(tok); // return jeff::chars_to_fast<AsType>(tok);
    }


    template<class... AsTypes>
    [[nodiscard]]
    std::optional<std::tuple<AsTypes...>> getTokensAs(){
      

      try{
        return std::make_optional(std::make_tuple(*getTokenAs<AsTypes>()...));
      } catch(const std::bad_optional_access&) {
        return std::nullopt;
      }

      // auto tok = getToken();

      // std::tuple<std::optional<AsType>, std::optional<Types>...> tup

      // return jeff::chars_to<AsType>(tok); // return jeff::chars_to_fast<AsType>(tok);
    }
    // // template<class AsType, class... Args>
    // [[nodiscard]]
    // std::enable_if_t<is_one_of_v<AsType, int, float, double, long double>, 
    // std::optional<AsType> > getTokenAs(Args&&... args){
    //   auto tok = getToken(std::forward<Args>(args)...);
    //   return jeff::chars_to<AsType>(tok); // return jeff::chars_to_fast<AsType>(tok);
    // }

    std::string_view operator()() { return getToken(); }
    virtual explicit operator bool() const = 0;
  };


  template<class T, std::size_t BlockSize = 4096>
  struct Tokenizer : public TokenizerBase{
    using source_type = T; // Requires string_view getBlock(buf), operator bool()
    static constexpr std::size_t buffer_size = BlockSize;

    // I want this class to be able to accept pointers, references to
    // or actual readers moved in to this struct, so I have to do a bit of trickery
    using source_type_naked = 
      std::conditional_t<std::is_pointer_v<source_type>,
        std::remove_pointer_t<source_type>,
        std::remove_reference_t<source_type>
      >;

    virtual ~Tokenizer() = default;  // Ensure that the source is cleaned up if it is not a ptr or ref


    std::array<std::array<char, buffer_size>, 2> buffers; // Where the actual data is stored
    std::string_view buf_sv;                              // Acts as a more useful pointer into buffers
    std::string_view delimiter_sv;                        // The active delimiters used by this
    source_type _source;                                  // The source used to populate buffers

    Tokenizer(source_type_naked& src, std::string_view delimiters = default_delims) : 
      buf_sv{}, 
      delimiter_sv{delimiters}, 
      _source(src) {  }

    Tokenizer(source_type_naked&& src, std::string_view delimiters = default_delims) :
      buf_sv{}, 
      delimiter_sv{delimiters}, 
      _source(std::move(src)) {  }


    Tokenizer(Tokenizer&& that) noexcept = default;
    Tokenizer& operator=(Tokenizer&& that) noexcept = default;

    
    [[nodiscard]]
    std::string_view getToken() override{ return getToken(delimiter_sv); }

    // TODO: Variable token delims
    [[nodiscard]]
    std::string_view getToken(std::string_view delimiters) override{
      if(buf_sv.empty() && !refillBuffer()){  // Check on empty buffer
        buf_sv = {};  // Failed to refill buffer, so just return empty
        return {};
      }
      
      // Find where a token starts
      auto tbegin_loc = buf_sv.find_first_not_of(delimiters);
      if(tbegin_loc == std::string_view::npos)
        return handleNoOpenDelimFound(delimiters);  // Could not find the start of a token
      
      // Find where this token ends
      auto tend_loc = buf_sv.find_first_of(delimiters, tbegin_loc+1);
      if(tend_loc == std::string_view::npos)  
        return handleNoCloseDelimFound(delimiters); // Could not find the an end to the token
      
      // Extract the token and update the buffer pointer to exclude this token
      auto token = buf_sv.substr(tbegin_loc, tend_loc - tbegin_loc);
      buf_sv.remove_prefix(tbegin_loc + token.size() + 1); // +1 because it removes the delim?? Test if i am right
      return token;
    }



    [[nodiscard]]
    explicit operator bool() const override{
      if(!bufferContains<1>(buf_sv) && bufferContains<0>(buf_sv)){
        std::cerr << "ERROR: Need to rethink my operator bool() method" << std::endl;
      }
      return  bufferContains<1>(buf_sv) || source().operator bool();
    }
    


    // Temp method for development
    [[nodiscard]]
    std::vector<std::string> getAllTokens(){
      std::vector<std::string> toks;
      echo(operator bool());
      while(operator bool()){
        auto tok = getToken();
        if(!tok.empty())
        toks.emplace_back(tok);
      }
      return toks;
    }
    [[nodiscard]]
    std::vector<std::string> getAllTokens(std::string_view delimiters){
      delimiter_sv = delimiters;
      return getAllTokens();
    }


    constexpr void setDelimiters(std::string_view delimiters){ this->delimiter_sv = delimiters; }
    constexpr std::string_view getDelimiters() const{ return delimiter_sv; }



    // template<class... Args>
    // [[nodiscard]]
    // jkeyword getKeyword(Args&&... args){
    //   return {getToken(std::forward<Args>(args)...)};
    // }

    // template<class AsType, class... Args>
    // [[nodiscard]]
    // std::enable_if_t<is_one_of_v<AsType, int, float, double, long double>, 
    // std::optional<AsType> > getToken(Args&&... args){
    //   auto tok = getToken(std::forward<Args>(args)...);
    //   return jeff::chars_to_fast<AsType>(tok);
    // }
    

  // #ifdef _MSC_VER  
    // template<class AsType, class... Args>
    // std::enable_if_t<is_one_of_v<AsType, int, float, double, long double>, 
    // std::optional<AsType> > getToken(Args&&... args){
    //   auto tok = getToken(std::forward<Args>(args)...);

    //   AsType value;
    //   // auto [p, ec] = std::from_chars(tok.data(), tok.data()+tok.size(), value);
    //   auto result = std::from_chars(tok.data(), tok.data()+tok.size(), value);
    //   if(result.ec == std::errc{})
    //     return std::nullopt;
    //   return std::make_optional(value);
    // }
  // #else
  //   // template<class AsType, class... Args>
  //   // std::optional<AsType> getToken(Args&&... args){
  //   template<class AsType, class... Args>
  //   std::enable_if_t<is_one_of_v<AsType, int, float, double, long double>, 
  //   std::optional<AsType> > getToken(Args&&... args){
  //     auto tok = getToken(std::forward<Args>(args)...);
  //     if(tok.empty())
  //       return std::make_optional(static_cast<AsType>(-42)); // TODO: Safe fallback for when from_chars DNE
  //     return std::nullopt;
  //   }
  // #endif
    



  private:
    [[nodiscard]]
    bool refillBuffer(){
      // std::cout << "refilling buffer" << std::endl;
      if(!source())
        return false;
      
      auto read_sv = source().getBlock(buffers[1]);
      if(read_sv.empty()){ // std::cerr << "read_sv is empty! File is likely done" << std::endl;
        if(source())
          std::cerr << "WARNING! File read failed despite not being at EOF" << std::endl;
        return false;
      }
      // If buffer is already pointing in the 0th buffer, that means it contains bits of previous
      // data and we must have the string_view bridge these
      if(bufferContains<0>(buf_sv))
        buf_sv = std::string_view( buf_sv.data(), buf_sv.size() + read_sv.size() );
      else
        buf_sv = read_sv;
      return true;
    }
    [[nodiscard]]
    bool refillBufferAfterCopy(){
      auto bufdest = buffers[0].end() - buf_sv.size();
      std::copy(buf_sv.begin(), buf_sv.end(), bufdest);
      // NOTE: MSVC does not have std::array<char,N>::begin() return char*
      auto bufdest_ptr = buffers[0].data() + buffers[0].size() - buf_sv.size();
      buf_sv = std::string_view(bufdest_ptr, buf_sv.size());
      //buf_sv = std::string_view(bufdest, buf_sv.size());
      return refillBuffer();
    }

    // Helpers for getToken
    [[nodiscard]]
    std::string_view handleNoOpenDelimFound(std::string_view delimiters){
      std::cerr << "ERROR: No token character found" << std::endl;
      if(refillBuffer())
        return getToken(delimiters);
      // This could mean that we have reached the end of the file
      std::cerr << "refillBufferAfterCopy() failed loc1" << std::endl;
      buf_sv = {};
      return {};
    }
    [[nodiscard]]
    std::string_view handleNoCloseDelimFound(std::string_view delimiters){
      if(!operator bool()){ // TODO: I think I can remove this
        std::cout << "operator bool failed on the file, something is wrong" << std::endl;
        return {};
      }
      // Must copy what's left of the active buffer into the end of the backup buffer
      // and then call for a refill, as there is still some bits of a token in the active buffer
      if(refillBufferAfterCopy())
        return getToken(delimiters);
      if(buf_sv.empty()) std::cerr << "tmp: Not sure if this is possible" << std::endl;
      // This could be reached when the final token of the file does not have a terminiting delim
      // so just return what is currently in buf_sv and clear it;
      return std::exchange(buf_sv, {});
    }


    // Helper to check if the buffer pointer is pointing within a buffer
    template<std::size_t Idx> [[nodiscard]]
    constexpr bool bufferContains(std::string_view sv) const{
      static_assert(Idx == 0 || Idx == 1);
      //return sv.data() >= buffers[Idx].begin() && sv.data() < buffers[Idx].end();

      // Why doesn't array::data return pointer  in msvc :(
      const auto begin = buffers[Idx].data();
      const auto end = buffers[Idx].data() + buffers[Idx].size();
      return sv.data() >= begin && sv.data() < end;
      //return sv.data() >= buffers[Idx].begin() && sv.data() < buffers[Idx].end();
      //return sv.data() >= buffers[Idx].begin() && sv.data() < buffers[Idx].end();
    }

    // Source needs to be accessed different ways depending on its type
    [[nodiscard]]
    source_type_naked& source(){
      if constexpr(std::is_pointer_v<source_type>) { return *_source; } else { return _source; }
    }
    [[nodiscard]]
    const source_type_naked& source() const{
      if constexpr(std::is_pointer_v<source_type>) { return *_source; } else { return _source; }
    }

  };

  // Allow construction of tokenizers from different input types
  // The underlying type must have 
  template<class Source>
  inline decltype(auto) makeTokenizer(Source&& source){
    return Tokenizer<Source>(std::forward<Source>(source));
  }
  template<std::size_t BlockSize, class Source>
  inline decltype(auto) makeTokenizer(Source&& source){
    return Tokenizer<Source, BlockSize>(std::forward<Source>(source));
  }

  template<std::size_t BlockSize = 4096>   // TODO: Error handling
  inline Tokenizer<FileReader, BlockSize> makeFileTokenizer(std::string filename){
    return makeTokenizer<BlockSize>(FileReader(std::move(filename)));
  }




  using TokenizerPointer = std::shared_ptr<TokenizerBase>;
  template<std::size_t BlockSize = 4096>   // TODO: Error handling
  inline TokenizerPointer newFileTokenizer(std::string filename) {
    using TokType = Tokenizer<FileReader, BlockSize>;
    return std::shared_ptr<TokType>(new TokType(makeFileTokenizer<BlockSize>(filename)));
    //return std::make_shared<tokenizer<FileReader, BlockSize>>(FileReader(std::move(filename)));
  }


  template<std::size_t BlockSize = 4096>
  std::vector<std::string> tokenizeFile(std::string filename, std::string_view delimiters = TokenizerBase::default_delims){
    return makeFileTokenizer<BlockSize>(filename).getAllTokens(delimiters);
  }

   
  constexpr bool is_word(std::string_view sv) {
    for (auto c : sv)
      //if (!(std::isalnum(c) || c == '_'))
        return false;
    return true;
  }




} // end namespace jeff




// int main(){
//   // using namespace std::literals;

//   // auto toki = makeFileTokenizer("../tmp.txt");
//   // auto toki = makeFileTokenizer("../jout.hpp");
//   // echo(toki.operator bool());  


//   // auto toks = toki.getAllTokens();
//   // // auto toks = toki.getAllTokens("\n 0");
//   // echo(toks.size());

//   // echo(toks);

//   // toks = tokenizeFile("../jutil.hpp");
//   // echo(toks.size());

//   auto toks = tokenizeFile("floats.txt");
//   echo(toks.size());

//   return 0; 
// } 

