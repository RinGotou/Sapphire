#pragma once
#include "message.h"
#include "object.h"

namespace sapphire {
  enum ArgumentType {
    kArgumentLiteralValue, 
    kArgumentObjectStack, 
    kArgumentReturnValue, 
    kArgumentInvalid

  };

  enum NodeType {
    kNodeMachineCommand, 
    kNodeFunction, 
    kNodeInvalid
  };

  struct ArgumentProperties {
    struct {
      bool optional_param, variable_param, constraint;
    } fn;
    struct {
      bool use_last_assert, is_chain_tail;
    } member_access;
    struct {
      string id;
      ArgumentType type;
    } domain;
    size_t token_id;

    ArgumentProperties() : 
      fn{ false, false, false },
      member_access{ false, false },
      domain{ "", kArgumentInvalid },
      token_id(0) {}
  };

  struct Annotation {
    bool void_call;
    bool local_object;
    bool ext_object;
    bool use_last_assert;
    size_t nest;
    size_t nest_end;
    size_t escape_depth;

    Keyword nest_root;

    Annotation() : 
      void_call(false), 
      local_object(false), 
      ext_object(false),
      use_last_assert(false),
      nest(0),
      nest_end(0),
      escape_depth(0),
      nest_root(kKeywordNull) {}
  };

  class Argument {
  private:
    string data_;
    ArgumentType type_;
    LiteralType token_type_;

  public:
    ArgumentProperties properties;

  public:
    Argument() :
      data_(), type_(kArgumentInvalid), token_type_(kLiteralTypeInvalid), properties() {}
    Argument(string data, ArgumentType type, LiteralType token_type) :
      data_(data), type_(type), token_type_(token_type), properties() {}

    void SetDomain(string id, ArgumentType type) {
      properties.domain.id = id;
      properties.domain.type = type;
    }

    bool HasDomain() {
      return properties.domain.type != kArgumentInvalid;
    }

    auto &GetData() { return data_; }
    auto &GetType() { return type_; }
    LiteralType GetStringType() { return token_type_; }
    bool IsPlaceholder() const { return type_ == kArgumentInvalid; }
  };

  struct FunctionInfo { string id; Argument domain; };

  class ASTNode {
  private:
    variant<Keyword, FunctionInfo> data_;

  public:
    size_t idx;
    NodeType type;
    Annotation annotation;

    ASTNode() :
      data_(), idx(0), type(kNodeInvalid), annotation() {}
    ASTNode(Keyword token) :
      data_(token), idx(0), type(kNodeMachineCommand), annotation() {} 
    ASTNode(string token, Argument domain = Argument()) :
      data_(FunctionInfo{ token, domain }), idx(0), type(kNodeFunction), annotation() {}

    string GetFunctionId() {
      if (type == kNodeFunction) return std::get<FunctionInfo>(data_).id;
      return string();
    }

    Argument GetFunctionDomain() {
      if (type == kNodeFunction) return std::get<FunctionInfo>(data_).domain;
      return Argument();
    }

    bool HasDomain() {
      if (type != kNodeFunction) return false;
      auto &func_info = std::get<FunctionInfo>(data_);
      return func_info.domain.GetType() != kArgumentInvalid;
    }

    void SetDomainTokenId(size_t token_id) {
      if (type != kNodeFunction) return;
      auto &func_info = std::get<FunctionInfo>(data_);
      func_info.domain.properties.token_id = token_id;
    }

    Keyword GetKeywordValue() {
      if (type == kNodeMachineCommand) return std::get<Keyword>(data_);
      return kKeywordNull;
    }

    bool IsPlaceholder() const { return type == kNodeInvalid; }
   };

  using ArgumentList = deque<Argument>;
  using Sentense = pair<ASTNode, ArgumentList>;

  class AnnotatedAST : public deque<Sentense> {
  protected:
    AnnotatedAST *source_;
    unordered_map<size_t, list<size_t>> jump_record_;

  public:
    AnnotatedAST() : deque<Sentense>(), source_(nullptr) {}
    AnnotatedAST(AnnotatedAST *source) : deque<Sentense>(), source_(source) {}
    AnnotatedAST(const AnnotatedAST &rhs) : deque<Sentense>(rhs), source_(rhs.source_),
      jump_record_(rhs.jump_record_) {}
    AnnotatedAST(const AnnotatedAST &&rhs) : AnnotatedAST(rhs) {}

    void AddJumpRecord(size_t index, list<size_t> record) {
      jump_record_.emplace(make_pair(index, record));
    }

    bool FindJumpRecord(size_t index, stack<size_t> &dest);
  };

  using AASTPointer = AnnotatedAST * ;
}