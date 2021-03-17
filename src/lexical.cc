#include "lexical.h"

namespace sapphire {
  wstring s2ws(const string &s) {
    if (s.empty()) return wstring();
    size_t length = s.size();
    //wchar_t *wc = (wchar_t *)malloc(sizeof(wchar_t) * (length + 2));
    wchar_t *wc = (wchar_t *)calloc(length + 64, sizeof(wchar_t));
    auto res = mbstowcs(wc, s.data(), s.length() + 1);
    wstring str(wc);
    free(wc);
    return str;
  }

  string ws2s(const wstring &s) {
    if (s.empty()) return string();
    size_t length = s.size();
    //char *c = (char *)malloc(sizeof(char) * (length + 1) * 2);
    char *c = (char *)calloc((length + 64) * 2, sizeof(char));
    auto res = wcstombs(c, s.data(), (length + 64) * 2);
    string result(c);
    free(c);
    return result;
  }
}

namespace sapphire::lexical {
  Terminator GetTerminatorCode(string src) {
    if (IsBinaryOperator(GetKeywordCode(src))) {
      return Terminator::BinaryOperator;
    }

    if (IsMonoOperator(GetKeywordCode(src))) {
      return Terminator::MonoOperator;
    }

    if (src == "=")   return Terminator::Assign;
    if (src == ",")   return Terminator::Comma;
    if (src == "[")   return Terminator::LeftBracket;
    if (src == ".")   return Terminator::Dot;
    if (src == "(")   return Terminator::LeftParen;
    if (src == "]")   return Terminator::RightSqrBracket;
    if (src == ")")   return Terminator::RightBracket;
    if (src == "{")   return Terminator::LeftBrace;
    if (src == "}")   return Terminator::RightCurBracket;
    if (src == "fn")  return Terminator::Fn;
    if (src == "struct") return Terminator::Struct;
    if (src == "module") return Terminator::Module;
    if (src == "for") return Terminator::For;
    if (src == "in")  return Terminator::In;
    if (src == "<-")  return Terminator::Arrow;
    return Terminator::Null;
  }

  bool IsBinaryOperator(Operation token) {
    bool result;
    switch (token) {
    case Operation::Bind:
    case Operation::Delivering:
    case Operation::Plus:
    case Operation::Minus:
    case Operation::Times:
    case Operation::Divide:
    case Operation::Equals:
    case Operation::LessOrEqual:
    case Operation::GreaterOrEqual:
    case Operation::NotEqual:
    case Operation::Greater:
    case Operation::Less:
    case Operation::And:
    case Operation::Or:
    case Operation::Increase:
    case Operation::Decrease:
      result = true;
      break;
    default:
      result = false;
      break;
    }
    return result;
  }

  bool IsMonoOperator(Operation token) {
    bool result;
    switch (token) {
    case Operation::Not:
      result = true;
      break;
    default:
      result = false;
      break;
    }

    return result;
  }

  bool IsOperator(Operation token) {
    return IsBinaryOperator(token) || IsMonoOperator(token);
  }

  int GetTokenPriority(Operation token) {
    int result;
    switch (token) {
    case Operation::Bind:
    case Operation::Delivering:
      result = 0;
      break;
    case Operation::And:
    case Operation::Or:
      result = 1;
      break;
    case Operation::Equals:
    case Operation::LessOrEqual:
    case Operation::GreaterOrEqual:
    case Operation::NotEqual:
    case Operation::Greater:
    case Operation::Less:
      result = 2;
      break;
    case Operation::Plus:
    case Operation::Minus:
      result = 3;
      break;
    case Operation::Times:
    case Operation::Divide:
      result = 4;
      break;
    default:
      result = 5;
      break;
    }

    return result;
  }

  unordered_map<string, Operation> &GetKeywordBase() {
    using T = pair<string, Operation>;
    static unordered_map<string, Operation> base = {
      T(kStrAssert         ,Operation::Assert),
      T(kStrLocal          ,Operation::Local),
      T(kStrHash           ,Operation::Hash),
      T(kStrFor            ,Operation::For),
      T(kStrIn             ,Operation::In),
      T(kStrNullObj        ,Operation::NullObj),
      T(kStrToString       ,Operation::ToString),
      T(kStrTime           ,Operation::Time),
      T(kStrVersion        ,Operation::Version),
      T(kStrCodeNameCmd    ,Operation::CodeName),
      T(kStrSwap           ,Operation::Swap),
      T(kStrSwapIf         ,Operation::SwapIf),
      T(kStrIf             ,Operation::If),
      T(kStrFn             ,Operation::Fn),
      T(kStrEnd            ,Operation::End),
      T(kStrElse           ,Operation::Else),
      T(kStrElif           ,Operation::Elif),
      T(kStrWhile          ,Operation::While),
      T(kStrPlus           ,Operation::Plus),
      T(kStrMinus          ,Operation::Minus),
      T(kStrTimes          ,Operation::Times),
      T(kStrDiv            ,Operation::Divide),
      T(kStrEquals         ,Operation::Equals),
      T(kStrAnd            ,Operation::And),
      T(kStrOr             ,Operation::Or),
      T(kStrNot            ,Operation::Not),
      T(kStrIncrease       ,Operation::Increase),
      T(kStrDecrease       ,Operation::Decrease),
      T(kStrLessOrEqual    ,Operation::LessOrEqual),
      T(kStrGreaterOrEqual ,Operation::GreaterOrEqual),
      T(kStrNotEqual       ,Operation::NotEqual),
      T(kStrGreater        ,Operation::Greater),
      T(kStrLess           ,Operation::Less),
      T(kStrReturn         ,Operation::Return),
      T(kStrContinue       ,Operation::Continue),
      T(kStrBreak          ,Operation::Break),
      T(kStrCase           ,Operation::Case),
      T(kStrWhen           ,Operation::When),
      T(kStrTypeId         ,Operation::TypeId),
      T(kStrMethodsCmd     ,Operation::Methods),
      T(kStrUsing          ,Operation::Using),
      T(kStrExist          ,Operation::Exist),
      T(kStrStruct         ,Operation::Struct),
      T(kStrModule         ,Operation::Module),
      T(kStrInclude        ,Operation::Include),
      T(kStrSuper          ,Operation::Super),
      T(kStrIsBaseOf       ,Operation::IsBaseOf),
      T(kStrHasBehavior    ,Operation::HasBehavior),
      T(kStrIsVariableParam ,Operation::IsVariableParam),
      T(kStrAttribute          ,Operation::Attribute),
      T(kStrConstraintArrow    ,Operation::Constaint),
      T(kStrPrint              ,Operation::Print),
      T(kStrPrintLine          ,Operation::PrintLine),
      T(kStrInput              ,Operation::Input),
      T(kStrConsole            ,Operation::Conole),
      T(kStrGetChar            ,Operation::GetChar),
      T(kStrSleep              ,Operation::Sleep)
    };
    return base;
  }

  Operation GetKeywordCode(string src) {
    auto &base = GetKeywordBase();
    auto it = base.find(src);
    if (it != base.end()) return it->second;
    return Operation::Null;
  }

  bool IsString(string target) {
    if (target.empty()) return false;
    if (target.size() == 1) return false;
    return(target.front() == '\'' && target.back() == '\'');
  }

  bool IsIdentifier(string target) {
    if (target.empty()) return false;
    const auto head = target.front();

    if (!IsAlpha(head) && head != '_') return false;

    bool result = true;
    for (auto &unit : target) {
      if (!IsDigit(unit) && !IsAlpha(unit) && unit != '_') {
        result = false;
        break;
      }

    }
    return result;
  }

  bool IsInteger(string target) {
    if (target.empty()) return false;
    const auto head = target.front();

    if (compare(head, '-', '+') && target.size() == 1) {
      return false;
    }

    if (!IsDigit(head) && !compare(head, '-', '+')) {
      return false;
    }

    bool result = true;
    for (size_t i = 1; i < target.size(); ++i) {
      if (!IsDigit(target[i])) {
        result = false;
        break;
      }
    }
    return result;
  }

  bool IsFloat(string target) {
    if (target.empty()) return false;
    const auto head = target.front();
    bool dot = false;

    if (compare(head, '-', '+') && target.size() == 1) {
      return false;
    }

    if (!IsDigit(head) && !compare(head, '-', '+')) {
      return false;
    }

    bool result = true;
    for (size_t i = 1; i < target.size(); ++i) {
      if (target[i] == '.' && dot == false) {
        dot = true;
      }
      else if (target[i] == '.' && dot == true) {
        result = false;
        break;
      }

      if (!IsDigit(target[i]) &&
        target[i] != '.') {
        result = false;
        break;
      }

      if (i == target.size() - 1 && !IsDigit(target[i])) {
        result = false;
        break;
      }
    }
    return result;
  }

  bool IsBlank(string target) {
    if (target.empty()) return false;
    bool result = true;
    for (auto &unit : target) {
      if (!compare(unit, ' ', '\t', '\r', '\n')) {
        result = false;
        break;
      }
    }
    return result;
  }


  bool IsSymbol(string target) {
    static const unordered_set<string> symbols = {
      "+", "-", "*", "/", ">", ">=", "<", "<=", "<-",
      "!=", "&&", "||", "&", "|", "!", "(", ")", "{", "}", "=", "==",
      "[", "]", ",", ".", "'", ";", "_", "-=", "+=", "->"
    };

    if (target.empty()) return false;
    bool result = (symbols.find(target) != symbols.end());
    return result;
  }

  bool IsBoolean(string target) {
    return compare(target, "true", "false");
  }

  LiteralType GetStringType(string src, bool ignore_symbol_rule) {
    LiteralType type = LiteralType::Invalid;
    if (src.empty())              type = LiteralType::Invalid;
    else if (IsBoolean(src))      type = LiteralType::Bool;
    else if (IsIdentifier(src))   type = LiteralType::Identifier;
    else if (IsInteger(src))      type = LiteralType::Int;
    else if (IsFloat(src))        type = LiteralType::Float;
    else if (IsBlank(src))        type = LiteralType::Whitespace;
    else if (IsString(src))       type = LiteralType::String;

    if (!ignore_symbol_rule) {
      if (IsSymbol(src)) type = LiteralType::Symbol;
    }
    return type;
  }

  char GetEscapeChar(char target) {
    char result;
    switch (target) {
    case 't':result = '\t'; break;
    case 'n':result = '\n'; break;
    case 'r':result = '\r'; break;
    default:result = target; break;
    }
    return result;
  }

  wchar_t GetEscapeCharW(wchar_t target) {
    wchar_t result;
    switch (target) {
    case L't':result = L'\t'; break;
    case L'n':result = L'\n'; break;
    case L'r':result = L'\r'; break;
    default:result = target; break;
    }
    return result;
  }

  bool IsWideString(string target) {
    auto result = false;
    for (auto &unit : target) {
      if (unit < 0 || unit > 127) {
        result = true;
        break;
      }
    }
    return result;
  }

  string GetRawString(string target) {
    string str = target.substr(1, target.size() - 2);
    string output;
    bool escape = false;
    for (size_t i = 0; i < str.size(); ++i) {
      if (str[i] == '\\' && !escape) {
        escape = true;
        continue;
      }
      if (escape) {
        output.append(1, GetEscapeChar(str[i]));
        escape = false;
      }
      else {
        output.append(1, str[i]);
      }
    }
    return output;
  }

  string MakeBoolean(bool origin) {
    return origin ? kStrTrue : kStrFalse;
  }

  bool IsDigit(char c) {
    return (c >= '0' && c <= '9');
  }

  bool IsAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
  }

  bool IsPlainType(string type_id) {
    return type_id == kTypeIdInt ||
      type_id == kTypeIdFloat ||
      type_id == kTypeIdString || 
      type_id == kTypeIdBool;
  }

  string ToUpper(string source) {
    string result;
    for (auto &unit : source) {
      result.append(1, std::toupper(unit));
    }

    return result;
  }

  string ToLower(string source) {
    string result;
    for (auto &unit : source) {
      result.append(1, std::tolower(unit));
    }

    return result;
  }

  string ReplaceInvalidChar(string source) {
    string result;
    for (auto &unit : source) {
      if (!IsAlpha(unit) && unit != '_') {
        result.append("_");
        continue;
      }

      result.append(1, unit);
    }

    return result;
  }
}