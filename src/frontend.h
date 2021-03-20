#pragma once
#include "trace.h"
#include "filestream.h"

// !!! This module is deprecated and will be destroyed in the future.

#define INVALID_TOKEN Token(string(), LiteralType::Invalid)

namespace sapphire {
  class Message {
  private:
    StateLevel level_;
    string detail_;
    size_t idx_;

  public:
    Message() :
      level_(StateLevel::Normal), detail_(""), idx_(0) {}
    Message(Message &msg) :
      level_(msg.level_), detail_(msg.detail_), idx_(msg.idx_) {}
    Message(Message &&msg) :
      level_(msg.level_), detail_(std::forward<string>(msg.detail_)), idx_(msg.idx_) {}
    Message(string detail, StateLevel level = StateLevel::Normal) :
      level_(level), detail_(detail), idx_(0) {}

    Message &operator=(Message &msg) {
      level_ = msg.level_;
      detail_ = msg.detail_;
      idx_ = msg.idx_;
      return *this;
    }

    Message &operator=(Message &&msg) {
      return this->operator=(msg);
    }

    StateLevel GetLevel() const { return level_; }
    string GetDetail() const { return detail_; }
    size_t GetIndex() const { return idx_; }

    Message &SetLevel(StateLevel level) {
      level_ = level;
      return *this;
    }

    Message &SetDetail(const string &detail) {
      detail_ = detail;
      return *this;
    }

    Message &SetIndex(const size_t index) {
      idx_ = index;
      return *this;
    }

    void Clear() {
      level_ = StateLevel::Normal;
      detail_.clear();
      detail_.shrink_to_fit();
      idx_ = 0;
    }
  };

  using CombinedCodeline = pair<size_t, string>;
  using CombinedToken = pair<size_t, deque<Token>>;

  class LexicalAnalysis {
  private:
    StandardLogger *logger_;

  private:
    deque<CombinedToken> *dest_;

    void Scan(deque<string> &output, string target);
  public:
    LexicalAnalysis() = delete;
    LexicalAnalysis(deque<CombinedToken> &dest, StandardLogger *logger) : 
      dest_(&dest), logger_(logger) {}

    bool Feed(CombinedCodeline &src);

    auto &GetOutput() { return dest_; }
  };

  struct ParserFrame {
    deque<Argument> args;
    deque<ASTNode> nodes;
    bool local_object;
    bool ext_object;
    bool eol;
    bool seek_last_assert;
    size_t idx;
    Token current;
    Token next;
    Token next_2;
    Token last;
    Argument domain;
    deque<Token> &tokens;

    ParserFrame(deque<Token> &tokens) :
      args(),
      nodes(),
      local_object(false),
      ext_object(false),
      eol(false),
      seek_last_assert(false),
      idx(0),
      current(INVALID_TOKEN),
      next(INVALID_TOKEN),
      next_2(INVALID_TOKEN),
      last(INVALID_TOKEN),
      domain(),
      tokens(tokens) {}

    void Eat();
  };

  class FirstStageParsing {
  private:
    ParserFrame *frame_;
    size_t index_;
    deque<Token> tokens_;
    AnnotatedAST action_base_;
    string error_string_;

    void ProduceVMCode();
    bool CleanupStack();

    void BindStmt();
    void DeliveringStmt();
    void ScopeMemberStmt();
    void UnaryExpr();
    void Calling();
    bool GetElementStmt();
    bool ArrayGeneratorStmt();
    void BinaryExpr();
    bool FunctionHeaderStmt();
    bool StructHeaderStmt(Terminator terminator);
    bool ForEachStmt();

    bool OtherExpressions();
    void LiteralValue();
    
    Message Parse();
  public:
    FirstStageParsing() : frame_(nullptr), index_(0) {}
    AnnotatedAST &GetOutput() { return action_base_; }

    Operation GetASTRoot() {
      if (action_base_.empty()) {
        return Operation::Null;
      }
      return action_base_.back().first.GetOperation();
    }

    void Clear();
    Message Make(CombinedToken &line);
  };

  struct JumpListFrame {
    Operation nest_code;
    size_t nest;
    list<size_t> jump_record;
  };

  class GrammarAndSemanticAnalysis {
  private:
    AnnotatedAST *dest_;
    string path_;
    bool inside_struct_;
    bool inside_module_;
    size_t struct_member_fn_nest;
    stack<size_t> nest_;
    stack<size_t> nest_end_;
    stack<size_t> nest_origin_;
    stack<size_t> cycle_escaper_;
    stack<Operation> nest_type_;
    stack<JumpListFrame> jump_stack_;
    list<CombinedCodeline> script_;
    deque<CombinedToken> tokens_;

  private:
    StandardLogger *logger_;
    bool is_logger_held_;

  private:
    bool ReadScript(list<CombinedCodeline> &dest);

  public:
    ~GrammarAndSemanticAnalysis() { if (is_logger_held_) delete logger_; }
    GrammarAndSemanticAnalysis() = delete;
    GrammarAndSemanticAnalysis(string path, AnnotatedAST &dest, 
      string log, bool rtlog = false) :
      dest_(&dest), path_(path), inside_struct_(false), inside_module_(false),
      struct_member_fn_nest(0),
      logger_(), is_logger_held_(true) {
      logger_ = rtlog ?
        (StandardLogger *)new StandardRTLogger(log.data(), "a") :
        (StandardLogger *)new StandardCachedLogger(log.data(), "a");
    }
    GrammarAndSemanticAnalysis(string path, AnnotatedAST &dest,
      StandardLogger *logger) :
      dest_(&dest), path_(path), inside_struct_(false), inside_module_(false),
      struct_member_fn_nest(0),
      logger_(logger), is_logger_held_(false) {}
    
    bool Start();
  };
}
