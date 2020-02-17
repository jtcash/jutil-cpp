#pragma once


#include <string_view>
#include <string>
#include <array>
#include <iostream>
#include <fstream>


///////////////////////////////////////////////////////////////////////////////////////////////////
// This utility is used to facilitate reading files on a block-by-block level
// Uses an ifstream opened for reading in binary
///////////////////////////////////////////////////////////////////////////////////////////////////




// NOTE: Some inspiration came from this StackOverflow post
//  https://stackoverflow.com/a/524843/6232717

namespace jeff{


  class FileReader{
    // Throw exceptions on errors, otherwise just write to cerr 
    inline static constexpr bool enable_exceptions = true; 

  private:
    std::string filename; // The path of the file to be read in
    std::ifstream file;   // An fstream object for reading the file into memory

  public:

    FileReader() : filename{}, file{} { }

    // JANK: Figure out a better way to integrate this
    FileReader(std::string filename, bool openOnConstruction) : filename{std::move(filename)}, file{} { 
      if(openOnConstruction)
        open();
    }

    // Opens the file on construction by default
    FileReader(std::string filename) : FileReader(filename, true) {  }

    // FileReader is move constructible
    FileReader(FileReader&& that) noexcept : 
      filename(std::move(that.filename)),
      file(std::move(that.file)) {  }

    // File reader is move assignable. // TODO: what is best for members? move, swap?
    FileReader& operator=(FileReader&& that) noexcept{
      if(this != &that){
        filename = std::move(that.filename);
        file = std::move(that.file);
        // std::swap(filename, that.filename);
        // std::swap(file, that.file);
      }
      return *this;
    }

    // File reader is not copy constructible
    FileReader(const FileReader&) = delete;
    // FileReader is not copy assignable
    FileReader& operator=(const FileReader&) = delete;



  
    // Open the file specified by filename for reading, will destruct the previous fstream if it exists
    bool open(){
      file = decltype(file)(filename, std::ios::in | std::ios::binary);
      if(!file)
        return error_message(false, "Failed to open");
      return true;
    }

    // Open a different file
    bool open(std::string filename){
      this->filename = std::move(filename);
      return open();
    }




    // Read in a block of data, storing in dest. Return a string_view pointing to the fresh data
    // template<std::size_t N>
    // std::string_view getBlock(std::array<std::ifstream::char_type,N>& dest);
    template<std::size_t N>
    std::string_view getBlock(std::array<std::ifstream::char_type,N>& dest){
      if(!file)
        return error_message(std::string_view(dest.data(), 0), "Attempting to read from a bad status file"); 

      file.read(dest.data(), N);

      // if(file.eof()) std::cerr << "NOTE: hit EOF after reading " << file.gcount() << " bytes" << std::endl;
      auto charsRead = file.gcount();
      if(charsRead < 0)
        return error_message(std::string_view(dest.data(), 0), "No characters read, you should check EOF before calling");

      return std::string_view(dest.data(), charsRead);
    }

    decltype(auto) eof() const{ return file.eof(); }
    decltype(auto) tellg() { return file.tellg(); }

    // Forward through the underlying fstream's operator bool()
    explicit operator bool() const{ return file.operator bool(); }

  private:
    // Throw or print an error message depending on the enable_exceptions setting
    template<class T>
    auto error_message(T&& t, std::string_view status_message) -> decltype(std::forward<T>(t)){
      if constexpr(enable_exceptions){
        std::string errorMessage{"FileReader ERROR: "};
        errorMessage += status_message;
        errorMessage += " for file \"" + filename + '"';
        throw std::runtime_error(errorMessage);
      } else {
        std::cerr << "ERROR: " << status_message << " for file \"" << filename << '"' << std::endl;
      }
      return std::forward<T>(t); // Not reachable, but I'll keep it here for now
    }
  };





  // template<std::size_t N>
  // std::string_view FileReader::getBlock(std::array<std::ifstream::char_type,N>& dest){
  //   if(!file)
  //     return error_message(std::string_view(dest.data(), 0), "Attempting to read from a bad status file"); 

  //   file.read(dest.data(), N);

  //   // if(file.eof()) std::cerr << "NOTE: hit EOF after reading " << file.gcount() << " bytes" << std::endl;
  //   auto charsRead = file.gcount();
  //   if(charsRead < 0)
  //     return error_message(std::string_view(dest.data(), 0), "No characters read, you should check EOF before calling");

  //   return std::string_view(dest.data(), charsRead);
  // }
  

} // end namespace jeff