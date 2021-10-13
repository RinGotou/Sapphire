#pragma once
#include "common.h"

namespace sapphire {
  enum class TokenType {
    Identifier,
    String,
    Int,
    Float,
    Bool,
    Symbol,
    Whitespace,
    LineBreak,
    Invalid
  };

  using Token = pair<string, TokenType>;

  enum class Operation {
    Assert,
    Local,
    Load,
    For,
    In,
    NullObj,
    Swap,
    ExpList,
    Fn,
    If,
    Elif,
    End,
    Else,
    Bind,
    Delivering,
    ConstraintArrow,
    While,
    Plus,
    Minus,
    Times,
    Divide,
    Equals,
    LessOrEqual,
    GreaterOrEqual,
    NotEqual,
    Greater,
    Less,
    Return,
    And,
    Or,
    Not,
    Increase,
    Decrease,
    InitialArray,
    Continue,
    Break,
    Case,
    When,
    TypeId,
    Using,
    Struct,
    Module,
    DomainAssertCommand,
    Include, 
    Super,
    IsVariableParam,
    Attribute,
    Print,
    PrintLine,
    Sleep,
    Null
  };

  enum class Terminator {
    Assign,
    Comma,
    LeftBracket,
    Dot,
    LeftParen,
    RightSqrBracket,
    RightBracket,
    LeftBrace,
    RightCurBracket,
    MonoOperator,
    BinaryOperator,
    Fn,
    Struct,
    Module,
    For,
    In,
    Arrow,
    Null
  };

  // Language Keywords
  const string
    kStrAssert         = "assert",
    kStrStruct         = "struct",
    kStrModule         = "module",
    kStrInclude        = "include",
    kStrLocal          = "local",
    kStrIf             = "if",
    kStrFn             = "fn",
    kStrEnd            = "end",
    kStrFor            = "for",
    kStrIn             = "in",
    kStrElse           = "else",
    kStrElif           = "elif",
    kStrWhile          = "while",
    kStrWith           = "with",
    kStrContinue       = "continue",
    kStrBreak          = "break",
    kStrCase           = "case",
    kStrWhen           = "when",
    kStrReturn         = "return",
    kStrVariable       = "variable",
    kStrTrue           = "true",
    kStrFalse          = "false",
    kStrUsing          = "using",
    kStrMe             = "me";

  //Symbols
  const string 
    kStrPlus           = "+",
    kStrMinus          = "-",
    kStrTimes          = "*",
    kStrDiv            = "/",
    kStrEquals         = "==",
    kStrAnd            = "&&",
    kStrOr             = "||",
    kStrNot            = "!",
    kStrLessOrEqual    = "<=",
    kStrGreaterOrEqual = ">=",
    kStrNotEqual       = "!=",
    kStrGreater        = ">",
    kStrLess           = "<",
    kStrIncrease       = "+=",
    kStrDecrease       = "-=",
    kStrConstraintArrow = "->";

  // Misc
  const string
    kStrRootScope      = "!root",
    kStrCaseObj        = "!case",
    kStrIteratorObj    = "!iterator",
    kStrContainerKeepAliveSlot = "!container_keepalive",
    kStrCommentBegin   = "=begin",
    kStrCommentEnd     = "=end",
    kStrHead           = "head",
    kStrTail           = "tail",
    kStrPrint          = "print",
    kStrPrintLine      = "println",
    kStrSleep          = "sleep",
    kStrUserFunc       = "__func",
    kStrTypeId         = "typeid",
    kStrSwap           = "swap",
    kStrSize           = "size",
    kStrEmpty          = "empty",
    kStrCompare        = "compare",
    kStrAt             = "at",
    kStrRightHandSide  = "rhs",
    kStrLeftHandSide   = "lhs",
    kStrInitializer    = "initializer",
    kStrStructId       = "__struct_id",
    kStrSuperStruct    = "!super_struct",
    kStrModuleList     = "!module_list",
    kStrSuper          = "super",
    kStrIsVariableParam = "is_variable_param",
    kStrAttribute          = "attribute",
    kStrReturnValueConstrantObj = "!rt_constaint";


  wstring s2ws(const string &s);
  string ws2s(const wstring &s);
}

namespace sapphire::lexical {
  Terminator GetTerminatorCode(string src);
  bool IsBinaryOperator(Operation token);
  bool IsMonoOperator(Operation token);
  bool IsOperator(Operation token);
  int GetTokenPriority(Operation token);
  Operation GetKeywordCode(string src);
  string GetRawString(string target);
  bool IsString(string target);
  bool IsIdentifier(string target);
  bool IsInteger(string target);
  bool IsFloat(string target);
  bool IsBlank(string target);
  bool IsSymbol(string target);
  bool IsBoolean(string target);
  TokenType GetStringType(string target, bool ignore_symbol_rule = false);

  char GetEscapeChar(char target);
  wchar_t GetEscapeCharW(wchar_t target);
  bool IsWideString(string target);
  string MakeBoolean(bool origin);
  bool IsDigit(char c);
  bool IsAlpha(char c);
  bool IsPlainType(string type_id);
  string ToUpper(string source);
  string ToLower(string source);
  string ReplaceInvalidChar(string source);
}