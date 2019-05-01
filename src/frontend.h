#pragma once
#include "trace.h"
#include "filestream.h"

//Frontend version: hatsuki

namespace kagami {
  using CombinedCodeline = pair<size_t, string>;

  const map<string, string> kBracketPairs = {
    pair<string,string>(")", "("),
    pair<string,string>("]", "["),
    pair<string,string>("}", "{")
  };

  const vector<Keyword> kReservedWordStore = {
    kKeywordIf, kKeywordElif, kKeywordWhile, kKeywordReturn,
    kKeywordWhen, kKeywordCase
  };

  const vector<Keyword> kSingleWordStore = {
    kKeywordEnd, kKeywordElse, kKeywordContinue, kKeywordBreak
  };

  const vector<Keyword> nest_flag_collection = {
    kKeywordIf,kKeywordWhile,kKeywordFn,kKeywordCase,kKeywordFor
  };

  struct ParserBlock {
    deque<Argument> args;
    deque<Request> symbol;
    bool fn_expr;
    bool foreach_expr;
    bool local_object;
    Token current;
    Token next;
    Token next_2;
    Token last;
    Argument domain;
    int forward_priority;

    ParserBlock() :
      args(),
      symbol(),
      fn_expr(false),
      foreach_expr(false),
      local_object(false),
      current(),
      next(),
      next_2(),
      last(),
      domain(),
      forward_priority(0) {}
  };

  class LineParser {
    bool do_next_expr_;
    size_t index_;
    size_t continue_point_;
    vector<Token> tokens_;
    VMCode action_base_;
    string error_string_;

    vector<string> Scanning(string target);
    Message Tokenizer(vector<string> target);

    void ProduceVMCode(ParserBlock *blk);
    void BindExpr(ParserBlock *blk);
    void DotExpr(ParserBlock *blk);
    void UnaryExpr(ParserBlock *blk);
    void FuncInvokingExpr(ParserBlock *blk);
    bool IndexExpr(ParserBlock *blk);
    bool ArrayExpr(ParserBlock *blk);
    bool FunctionAndObject(ParserBlock *blk);
    void OtherToken(ParserBlock *blk);
    void BinaryExpr(ParserBlock *blk);
    bool CleanupStack(ParserBlock *blk);
    
    Message Parse();
  public:
    LineParser() : 
      do_next_expr_(false), 
      index_(0),
      continue_point_(0) {}
    VMCode &GetOutput() { return action_base_; }

    Keyword GetASTRoot() {
      if (action_base_.empty()) {
        return kKeywordNull;
      }
      return action_base_.back().first.keyword_value;
    }

    void Clear();
    Message Make(CombinedCodeline &line);
  };

  class VMCodeFactory {
  private:
    VMCode *dest_;
    string path_;
    stack<size_t> nest_;
    stack<size_t> nest_origin_;
    stack<Keyword> nest_type_;

  private:
    bool ReadScript(list<CombinedCodeline> &dest);

  public:
    VMCodeFactory() = delete;
    VMCodeFactory(string path, VMCode &dest) :
      dest_(&dest), path_(path) {}
    
    bool Start();
  };
}
