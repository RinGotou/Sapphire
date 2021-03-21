#pragma once
#include "common.h"

namespace sapphire {
  enum class LiteralType {
    Identifier,
    String,
    Int,
    Float,
    Bool,
    Symbol,
    Whitespace,
    Invalid
  };

  using Token = pair<string, LiteralType>;

  enum class Operation {
    Assert,
    Local,
    Load,
    Ext,
    Hash,
    For,
    In,
    NullObj,
    Destroy,
    ToString,
    Time,
    Version,
    CodeName,
    Swap,
    SwapIf,
    CSwapIf,
    ObjectAt,
    ExpList,
    Fn,
    If,
    Elif,
    End,
    Else,
    Bind,
    Delivering,
    Constaint,
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
    Exist,
    Methods,
    Using,
    Struct,
    Module,
    DomainAssertCommand,
    Include, 
    Super,
    IsBaseOf,
    HasBehavior,
    IsVariableParam,
    IsSameCopy,
    Attribute,
    Print,
    PrintLine,
    Input,
    Conole, 
    GetChar,
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

  const string
    kStrAssert         = "assert",
    kStrStruct         = "struct",
    kStrModule         = "module",
    kStrInclude        = "include",
    kStrRootScope      = "!root",
    kStrLocal          = "local",
    kStrExt            = "ext",
    kStrHash           = "hash",
    kStrIf             = "if",
    kStrFn             = "fn",
    kStrToString       = "to_string",
    kStrEnd            = "end",
    kStrCaseObj        = "!case",
    kStrIteratorObj    = "!iterator",
    kStrContainerKeepAliveSlot = "!container_keepalive",
    kStrCommentBegin   = "=begin",
    kStrCommentEnd     = "=end",
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
    kStrHead           = "head",
    kStrTail           = "tail",
    kStrUsing          = "using",
    kStrPrint          = "print",
    kStrPrintLine      = "println",
    kStrSleep          = "sleep",
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
    kStrConstraintArrow = "->",
    kStrUserFunc       = "__func",
    kStrTypeId         = "typeid",
    kStrMethodsCmd     = "methods",
    kStrExist          = "exist",
    kStrSwap           = "swap",
    kStrSwapIf         = "swap_if",
    kStrTrue           = "true",
    kStrFalse          = "false",
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
    kStrHasBehavior    = "has_behavior",
    kStrIsVariableParam = "is_variable_param",
    kStrAttribute          = "attribute",
    kStrReturnValueConstrantObj = "!rt_constaint",
    kStrMe             = "me";

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
  LiteralType GetStringType(string target, bool ignore_symbol_rule = false);

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