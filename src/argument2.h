#pragma once
#include <string>
#include <string_view>
#include <initializer_list>
#include <vector>
#include <unordered_map>
#include <utility>
#include <cstdio>

namespace sapphire::commandline {
  const std::string kHeaders[] = { "--", "/" };
  enum class Joiner { Colon = ':', Equal = '=' };
  enum class Header { DblHorizontal = 0, Slash = 1 };

  bool IsDigit(char c) {
    return (c >= '0' && c <= '9');
  }

  bool IsAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
  }

  //TODO: argument processor
  //TODO: multi arguments for single option case
  //TODO: better string support(string surrounded with quotes)

  // Command argument container class
  using ValueList = std::vector<std::string>;
  using SelectorUnit = std::pair<std::string, ValueList>;

  enum class ElementType { Header, Option, Joiner, Value, Comma };
  //TODO: Provide an exception-free error facility
  struct ArgumentError : public std::exception {
  public:
    enum class ErrorCode {
      InvalidHeader, InvalidArgument,
      RedundantValue, RedundantArgument, MissingValue, MissingArgument
    };

  protected:
    ErrorCode code_;
    std::int64_t elem_idx_;

  public:
    ArgumentError() = delete;
    ArgumentError(ErrorCode code, std::int64_t elem_idx) : 
      code_(code), elem_idx_(elem_idx) {}

    char const *what() const override {
      char const *result = nullptr;
      switch (code_) {
      case ErrorCode::InvalidHeader:
        result = "Invalid argument header"; break;
      case ErrorCode::InvalidArgument:
        result = "Invalid argument string"; break;
      case ErrorCode::RedundantValue:
        result = "Unnecessary value for argument"; break;
      case ErrorCode::RedundantArgument:
        result = "Unnecessary argument for current options"; break;
      case ErrorCode::MissingValue:
        result = "Value is not found for current argument"; break;
      case ErrorCode::MissingArgument:
        result = "Required sub argument is not found for current options"; break;
      default:break;
      }

      return result;
    }
  };

  class ArgumentProcessor {
  public:
    using Selector = std::unordered_map<std::string, ValueList>;

  protected:
    int argc_;
    char **argv_;
    Joiner joiner_;
    Header header_;
    Selector selector_;
    std::string binary_;

  protected:
    //TODO:lexer unit
    template <Header header>
    bool IsHeader(std::string_view value) {
      return value == kHeaders[static_cast<size_t>(header)];
    }

    bool IsOption(std::string_view value) {
      for (const auto &unit : value) {
        if (!IsAlpha(unit)) return false;
      }
      return true;
    }
    
    template <Joiner joiner>
    bool IsJoiner(char value) {
      return value == static_cast<char>(joiner);
    }

    bool IsValue(std::string value) {
      //TODO:string checking
    }

    constexpr bool IsComma(char value) const { 
      return value == ','; 
    }

    void GeneratePlainData(int argc, char **argv);

  public:
    ArgumentProcessor() = delete;
    ArgumentProcessor(int argc, char **argv, Joiner joiner, Header header) noexcept :
      argc_(argc), argv_(argv), joiner_(joiner), header_(header),
      selector_(), binary_() {}
  };
}