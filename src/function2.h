#pragma once
#include "annotatedAST.h"

namespace sapphire {
  using GenericFunctionPointer = void(*)();
  
  class State;

  using Activity = int(*)(State &);

  enum class ParameterPattern { Variable, Fixed };

  class CommonFunctionBase {
  public:
    virtual ~CommonFunctionBase() {}
  };

  struct FunctionPlacebo {};

  template <typename _T>
  struct FunctionBase : public CommonFunctionBase {
    _T impl;
    size_t offset;

    FunctionBase() : impl(), offset(0) {}

    FunctionBase(_T &impl, size_t offset) :
      impl(impl), offset(offset) {}
  };

  using ComponentFunction = FunctionBase<Activity>;
  using UserDefinedFunction = FunctionBase<AnnotatedAST>;
  using InvalidFunction = FunctionBase <FunctionPlacebo>;

  using FunctionSlot = variant<
    FunctionBase<Activity>,
    FunctionBase<AnnotatedAST>,
    //FuntionBase<ExtensionActivity>,
    FunctionBase<FunctionPlacebo>
  >;

  template <typename _T>
  inline bool CompareFunctionSlot(FunctionSlot &lhs, FunctionSlot &rhs) {
    auto &lhs_obj = std::get<FunctionBase<_T>>(lhs).impl;
    auto &rhs_obj = std::get<FunctionBase<_T>>(rhs).impl;
    //nested type should implement operator==() for this function
    return lhs_obj == rhs_obj;
  }

  //ineline bool CompareExtensionFunction

  enum class FunctionType { Component, UserDef, External, Invalid };

  class Function {
  protected:
    FunctionSlot slot_;

  public:
    ObjectMap scope;

  protected:
    bool variable_param_;
    FunctionType type_;
    vector<string> params_;
    string id_; //preserved for compatibility,  remove it in the future

  public:
    Function() :
      slot_(InvalidFunction()), scope(),
      variable_param_(false),
      type_(FunctionType::Invalid),
      params_(), id_() {}

    Function(Activity activity, string params, string id, bool variable = false) :
      slot_(ComponentFunction(activity, 0)), scope(),
      variable_param_(variable),
      type_(FunctionType::Component),
      params_(BuildStringVector(params)), id_(id) {}

    Function(size_t offset, AnnotatedAST ir, string id, vector<string> params, bool variable = false) :
      slot_(UserDefinedFunction(ir, offset)), scope(),
      variable_param_(variable),
      type_(FunctionType::UserDef),
      params_(params), id_(id) {}

    template <typename _T>
    _T &Get() { return std::get<FunctionBase<_T>>(slot_); }

    //Remove in the future, deprecated
    auto GetId() const { return id_; }

    auto IsVariableParam() const { return variable_param_; }
    auto &AccessParameters() { return params_; }
    auto GetType() const { return type_; }
    auto GetParamSize() const { return params_.size(); }
    //auto &AccessClosureScope() { return scope_; }
    auto GetOffset() const { return std::get<UserDefinedFunction>(slot_).offset; }
    auto Good() const { return (type_ != FunctionType::Invalid); }

    bool Compare(Function &rhs) {
      if (type_ != rhs.type_) return false;
      bool result = false;

      switch (type_) {
      case FunctionType::Component: CompareFunctionSlot<Activity>(slot_, rhs.slot_); break;
      case FunctionType::UserDef: CompareFunctionSlot<AnnotatedAST>(slot_, rhs.slot_); break;
      case FunctionType::External: /*TODO: Reconstruct external component support */ break;
      default: break;
      }
    }
  };

  using FunctionPointer = Function *;
}
