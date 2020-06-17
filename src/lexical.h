#pragma once
#include "common.h"

namespace sapphire {
  enum StringType {
    kStringTypeIdentifier,
    kStringTypeLiteralStr,
    kStringTypeInt,
    kStringTypeFloat,
    kStringTypeBool,
    kStringTypeSymbol,
    kStringTypeBlank,
    kStringTypeNull
  };

  using Token = pair<string, StringType>;

  /* Reserved keywords mark / VMCode commands */
  enum Keyword {
    kKeywordAssert,
    kKeywordLocal,
    kKeywordExt,
    kKeywordHash,
    kKeywordFor,
    kKeywordIn,
    kKeywordNullObj,
    kKeywordDestroy,
    kKeywordToString,
    kKeywordTime,
    kKeywordVersion,
    kKeywordCodeName,
    kKeywordSwap,
    kKeywordSwapIf,
    kKeywordCSwapIf,
    kKeywordObjectAt,
    kKeywordExpList,
    kKeywordFn,
    kKeywordIf,
    kKeywordElif,
    kKeywordEnd,
    kKeywordElse,
    kKeywordBind,
    kKeywordDelivering,
    kKeywordConstaint,
    kKeywordWhile,
    kKeywordPlus,
    kKeywordMinus,
    kKeywordTimes,
    kKeywordDivide,
    kKeywordEquals,
    kKeywordLessOrEqual,
    kKeywordGreaterOrEqual,
    kKeywordNotEqual,
    kKeywordGreater,
    kKeywordLess,
    kKeywordReturn,
    kKeywordAnd,
    kKeywordOr,
    kKeywordNot,
    kKeywordIncrease,
    kKeywordDecrease,
    kKeywordInitialArray,
    kKeywordContinue,
    kKeywordBreak,
    kKeywordCase,
    kKeywordWhen,
    kKeywordTypeId,
    kKeywordExist,
    kKeywordMethods,
    kKeywordUsing,
    kKeywordStruct,
    kKeywordModule,
    kKeywordDomainAssertCommand,
    kKeywordInclude, 
    kKeywordSuper,
    kKeywordIsBaseOf,
    kKeywordHasBehavior,
    kKeywordIsVariableParam,
    kKeywordIsOptionalParam,
    kKeywordOptionalParamRange,
    kKeywordIsSameCopy,
    kKeywordAttribute,
    kKeywordNull
  };

  enum Terminator {
    kTerminatorAssign,
    kTerminatorComma,
    kTerminatorLeftBracket,
    kTerminatorDot,
    kTerminatorLeftParen,
    kTerminatorRightSqrBracket,
    kTerminatorRightBracket,
    kTerminatorLeftBrace,
    kTerminatorRightCurBracket,
    kTerminatorMonoOperator,
    kTerminatorBinaryOperator,
    kTerminatorFn,
    kTerminatorStruct,
    kTerminatorModule,
    kTerminatorFor,
    kTerminatorIn,
    kTerminatorArrow,
    kTerminatorNull
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
    kStrNullObj        = "null_obj",
    kStrToString       = "to_string",
    kStrGetStr         = "get_str", //deprecated
    kStrTime           = "time", //deprecated
    kStrVersion        = "version",
    kStrCodeNameCmd    = "codename",
    kStrEnd            = "end",
    kStrPrint          = "print",
    kStrSwitchLine     = "!switch_line",
    kStrCaseObj        = "!case",
    kStrIteratorObj    = "!iterator",
    kStrContainerKeepAliveSlot = "!containter_keepalive",
    kStrCommentBegin   = "=begin",
    kStrCommentEnd     = "=end",
    kStrFor            = "for",
    kStrIn             = "in",
    kStrElse           = "else",
    kStrElif           = "elif",
    kStrWhile          = "while",
    kStrContinue       = "continue",
    kStrBreak          = "break",
    kStrCase           = "case",
    kStrWhen           = "when",
    kStrReturn         = "return",
    kStrOptional       = "optional",
    kStrVariable       = "variable",
    kStrHead           = "head",
    kStrTail           = "tail",
    kStrUsing          = "using",
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
    kStrSuperStructId  = "!super_struct_id", //deprecated?
    kStrSuperStructInitializer = "!super_initializer", //deprecated?
    kStrModuleList     = "!module_list",
    kStrSuper          = "super",
    kStrIsBaseOf       = "is_base_of",
    kStrHasBehavior    = "has_behavior",
    kStrIsVariableParam = "is_variable_param",
    kStrIsOptionalParam = "is_optional_param",
    kStrOptionalParamRange = "optional_param_range",
    kStrAttribute          = "attribute",
    kStrReturnValueConstrantObj = "!rt_constaint",
    kStrMe             = "me";

  wstring s2ws(const string &s);
  string ws2s(const wstring &s);
}

namespace sapphire::lexical {
  Terminator GetTerminatorCode(string src);
  bool IsBinaryOperator(Keyword token);
  bool IsMonoOperator(Keyword token);
  bool IsOperator(Keyword token);
  int GetTokenPriority(Keyword token);
  Keyword GetKeywordCode(string src);
  string GetRawString(string target);
  bool IsString(string target);
  bool IsIdentifier(string target);
  bool IsInteger(string target);
  bool IsFloat(string target);
  bool IsBlank(string target);
  bool IsSymbol(string target);
  bool IsBoolean(string target);
  StringType GetStringType(string target, bool ignore_symbol_rule = false);

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