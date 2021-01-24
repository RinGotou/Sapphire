#pragma once
#include "annotatedAST.h"

namespace sapphire {
  using GenericFunctionPointer = void(*)();
  using ReturningTunnel = void(*)(void *, void *, int);

  extern "C" struct VMState {
    void *obj_map, *ret_slot, *vm;
    ReturningTunnel tunnel;
  };

  using Activity = Message(*)(ObjectMap &);
  using ExtensionActivity = int(*)(VMState);

  enum ParameterPattern {
    kParamAutoSize,
    kParamAutoFill,
    kParamFixed
  };

  class _FunctionImpl {
  public:
    virtual ~_FunctionImpl() {}
  };

  struct _NullFunctionType {};

  template <typename _ImplType>
  class _Function : _FunctionImpl {
  protected:
    _ImplType impl_;

  public:
    _Function(_ImplType impl) : impl_(impl) {}
    _ImplType &Get() { return impl_; }
  };

  using FunctionBase = variant<
    _Function<Activity>,
    _Function<AnnotatedAST>,
    _Function<ExtensionActivity>,
    _Function<_NullFunctionType>
  >;

  using CXXFunction = _Function<Activity>;
  using VMCodeFunction = _Function<AnnotatedAST>;
  using ExtensionFunction = _Function<ExtensionActivity>;
  using InvalidFunction = _Function<_NullFunctionType>;

  template <typename _ImplType>
  inline bool CompareFunctionBase(FunctionBase &lhs, FunctionBase &rhs) {
    auto &lhs_obj = std::get<_Function<_ImplType>>(lhs).Get();
    auto &rhs_obj = std::get<_Function<_ImplType>>(rhs).Get();
    return lhs_obj == rhs_obj;
  }

  inline bool CompareVMCodeInstance(FunctionBase &lhs, FunctionBase &rhs) {
    auto &lhs_obj = std::get<ExtensionFunction>(lhs).Get();
    auto &rhs_obj = std::get<ExtensionFunction>(rhs).Get();
    return &lhs_obj == &rhs_obj;
  }

  enum FunctionImplType { kFunctionCXX, kFunctionVMCode, kFunctionExternal, kFunctionInvalid };

  class Function {
  private:
    FunctionBase base_;
    ObjectMap record_;

  private:
    ParameterPattern mode_;
    FunctionImplType type_;
    size_t limit_;
    size_t offset_;
    string id_;
    vector<string> params_;

  public:
    Function() :
      base_(InvalidFunction(_NullFunctionType())),
      record_(), mode_(), type_(kFunctionInvalid), 
      limit_(0), offset_(0), id_(), params_() {}

    Function(Activity activity, string params, string id,
      ParameterPattern argument_mode = kParamFixed) :
      base_(_Function(activity)), record_(),
      mode_(argument_mode), type_(kFunctionCXX), limit_(0),
      offset_(0), id_(id), params_(BuildStringVector(params)) {}

    Function(size_t offset, AnnotatedAST ir, string id, vector<string> params,
      ParameterPattern argument_mode = kParamFixed) :
      base_(_Function(ir)), record_(),
      mode_(argument_mode), type_(kFunctionVMCode), limit_(0),
      offset_(offset), id_(id), params_(params) {}

    Function(ExtensionActivity activity, string id, string params_pattern,
      ParameterPattern argument_mode = kParamFixed) :
      base_(_Function(activity)), record_(),
      mode_(argument_mode), type_(kFunctionExternal), limit_(0),
      offset_(0), id_(id), params_(BuildStringVector(params_pattern)) {}

    template <typename _ImplType>
    _ImplType &Get() { return std::get<_Function<_ImplType>>(base_).Get(); }

    string GetId() const { return id_; }
    ParameterPattern GetPattern() const { return mode_; }
    vector<string> &GetParameters() { return params_; }
    FunctionImplType GetType() const { return type_; }
    size_t GetParamSize() const { return params_.size(); }
    ObjectMap &GetClosureRecord() { return record_; }
    size_t GetLimit() const { return limit_; }
    size_t GetOffset() const { return offset_; }
    bool Good() const { return (type_ != kFunctionInvalid); }

    Function &SetClosureRecord(ObjectMap record) {
      record_ = record;
      return *this;
    }

    Function &AppendClosureRecord(string_view id, Object &&obj) {
      record_.insert(NamedObject(id, obj));
      return *this;
    }

    Function &SetLimit(size_t size) {
      limit_ = size;
      return *this;
    }

    bool Compare(Function &rhs) {
      if (type_ != rhs.type_) return false;
      bool result = false;

      switch (type_) {
      case kFunctionCXX: CompareFunctionBase<Activity>(base_, rhs.base_); break;
      case kFunctionExternal: CompareFunctionBase<ExtensionActivity>(base_, rhs.base_); break;
      case kFunctionVMCode: CompareVMCodeInstance(base_, rhs.base_); break;
      default:break;
      }

      return result;
    }
  };

  using FunctionImplPointer = Function *;
}
