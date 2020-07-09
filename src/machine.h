#pragma once
#include "frontend.h"
#include "management.h"
#include "components.h"

#define CHECK_PRINT_OPT(_Map)                          \
  if (_Map.find(kStrSwitchLine) != p.end()) {          \
    putc('\n', VM_STDOUT);                             \
  }

namespace sapphire {
  using Expect = pair<string, string>;
  using ExpectationList = initializer_list<Expect>;
  using NullableList = initializer_list<string>;

  CommentedResult TypeChecking(ExpectationList &&lst,
    ObjectMap &obj_map,
    NullableList &&nullable = {});

#define TC_ERROR(_Obj) Message(std::get<string>(_Obj), kStateError)
#define TC_FAIL(_Obj) !std::get<bool>(_Obj)

  using management::type::PlainComparator;

  string ParseRawString(const string &src);
  void InitPlainTypesAndConstants();
  void ActivateComponents();
  void ReceiveError(void* vm, const char* msg);

  using ResultTraitKey = pair<PlainType, PlainType>;
  using TraitUnit = pair<ResultTraitKey, PlainType>;
  const map<ResultTraitKey, PlainType> kResultDynamicTraits = {
    TraitUnit(ResultTraitKey(kPlainInt, kPlainInt), kPlainInt),
    TraitUnit(ResultTraitKey(kPlainInt, kPlainFloat), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainInt, kPlainString), kPlainString),
    TraitUnit(ResultTraitKey(kPlainInt, kPlainBool), kPlainInt),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainFloat), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainInt), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainString), kPlainString),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainBool), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainString, kPlainString), kPlainString),
    TraitUnit(ResultTraitKey(kPlainString, kPlainInt), kPlainString),
    TraitUnit(ResultTraitKey(kPlainString, kPlainFloat), kPlainString),
    TraitUnit(ResultTraitKey(kPlainString, kPlainBool), kPlainString),
    TraitUnit(ResultTraitKey(kPlainBool, kPlainBool), kPlainBool),
    TraitUnit(ResultTraitKey(kPlainBool, kPlainInt), kPlainInt),
    TraitUnit(ResultTraitKey(kPlainBool, kPlainFloat), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainBool, kPlainString), kPlainString)
  };

  template <typename ResultType, class Tx, class Ty, Keyword op>
  struct BinaryOpBox {
    ResultType Do(Tx A, Ty B) {
      return Tx();
    }
  };

  template <typename ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kKeywordPlus> {
    ResultType Do(Tx A, Ty B) {
      return A + B;
    }
  };

  template <typename ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kKeywordMinus> {
    ResultType Do(Tx A, Ty B) {
      return A - B;
    }
  };

  template <typename ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kKeywordTimes> {
    ResultType Do(Tx A, Ty B) {
      return A * B;
    }
  };

  template <typename ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kKeywordDivide> {
    ResultType Do(Tx A, Ty B) {
      return A / B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordEquals> {
    bool Do(Tx A, Ty B) {
      return A == B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordLessOrEqual> {
    bool Do(Tx A, Ty B) {
      return A <= B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordGreaterOrEqual> {
    bool Do(Tx A, Ty B) {
      return A >= B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordNotEqual> {
    bool Do(Tx A, Ty B) {
      return A != B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordGreater> {
    bool Do(Tx A, Ty B) {
      return A > B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordLess> {
    bool Do(Tx A, Ty B) {
      return A < B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordAnd> {
    bool Do(Tx A, Ty B) {
      return A && B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordOr> {
    bool Do(Tx A, Ty B) {
      return A || B;
    }
  };

  //Dispose divide operation for bool type
  template <>
  struct BinaryOpBox<bool, bool, bool, kKeywordDivide> {
    bool Do(bool A, bool B) {
      return true;
    }
  };

#define DISPOSE_STRING_MATH_OP(_OP)                 \
  template <>                                       \
  struct BinaryOpBox<string, string, string, _OP> { \
    string Do(string A, string B) {                 \
      return string();                              \
    }                                               \
  }                                                 \

#define DISPOSE_STRING_LOGIC_OP(_OP)                \
  template <>                                       \
  struct BinaryOpBox<bool, string, string, _OP> {   \
    bool Do(string A, string B) {                   \
      return false;                                 \
    }                                               \
  }                                                 \

  DISPOSE_STRING_MATH_OP(kKeywordMinus);
  DISPOSE_STRING_MATH_OP(kKeywordTimes);
  DISPOSE_STRING_MATH_OP(kKeywordDivide);
  DISPOSE_STRING_LOGIC_OP(kKeywordLessOrEqual);
  DISPOSE_STRING_LOGIC_OP(kKeywordGreaterOrEqual);
  DISPOSE_STRING_LOGIC_OP(kKeywordGreater);
  DISPOSE_STRING_LOGIC_OP(kKeywordLess);
  DISPOSE_STRING_LOGIC_OP(kKeywordAnd);
  DISPOSE_STRING_LOGIC_OP(kKeywordOr);

#undef DISPOSE_STRING_MATH_OP
#undef DISPOSE_STRING_LOGIC_OP

  template <typename ResultType, Keyword op>
  using MathBox = BinaryOpBox<ResultType, ResultType, ResultType, op>;

  template <typename Tx, Keyword op>
  using LogicBox = BinaryOpBox<bool, Tx, Tx, op>;

  const string kIteratorBehavior = "obj|step_forward|__compare";
  const string kContainerBehavior = "head|tail|empty";
  const string kForEachExceptions = "!iterator|!containter_keepalive";

  using CommandPointer = Command * ;

  class RuntimeFrame {
  public:
    bool error;
    bool warning;
    bool activated_continue;
    bool activated_break;
    bool void_call;
    bool disable_step;
    bool final_cycle;
    bool jump_from_end;
    bool initializer_calling;
    bool inside_initializer_calling;
    bool stop_point;
    bool has_return_value_from_invoking;
    bool cancel_cleanup;
    bool required_by_next_cond;
    bool is_there_a_cond;
    bool reserved_cond;
    bool direct_delivering;
    bool is_command;
    bool rstk_operated;
    VMCode *current_code;
    Object struct_base;
    Object assert_rc_copy;
    size_t jump_offset;
    size_t idx;
    string msg_string;
    string function_scope;
    string struct_id;
    string super_struct_id;
    stack<bool> condition_stack; //preserved
    stack<bool> scope_stack;
    stack<size_t> jump_stack;
    stack<size_t> branch_jump_stack;
    vector<ObjectCommonSlot> return_stack;

    RuntimeFrame(string scope = kStrRootScope) :
      error(false),
      warning(false),
      activated_continue(false),
      activated_break(false),
      void_call(false),
      disable_step(false),
      final_cycle(false),
      jump_from_end(false),
      initializer_calling(false),
      inside_initializer_calling(false),
      stop_point(false),
      has_return_value_from_invoking(false),
      cancel_cleanup(false),
      required_by_next_cond(false),
      is_there_a_cond(false),
      reserved_cond(false),
      direct_delivering(false),
      is_command(false),
      rstk_operated(false),
      current_code(nullptr),
      assert_rc_copy(),
      jump_offset(0),
      idx(0),
      msg_string(),
      function_scope(),
      struct_id(),
      super_struct_id(),
      condition_stack(),
      jump_stack(),
      branch_jump_stack(),
      return_stack() {}

    void Stepping();
    void Goto(size_t taget_idx);

    void AddJumpRecord(size_t target_idx);
    void MakeError(string str);
    void MakeWarning(string str);
    void RefreshReturnStack(Object &obj);
    void RefreshReturnStack(Object &&obj);
    void RefreshReturnStack(const ObjectInfo &info, const shared_ptr<void> &ptr);
    void RefreshReturnStack(bool value);
    void RefreshReturnStack(ObjectView &&view);

    template <class T>
    void RefreshReturnStack(T &value, string &type_id) {
      if (!void_call) {
        return_stack.push_back(new Object(value, type_id));
      }
      if (stop_point) {
        return_stack.push_back(new Object(value, type_id));
        has_return_value_from_invoking = true;
      }
    }

    template <class T>
    void RefreshReturnStack(T &&value, string &type_id) {
      if (!void_call) {
        return_stack.push_back(new Object(std::forward<T>(value), type_id));
      }
      if (stop_point) {
        return_stack.push_back(new Object(std::forward<T>(value), type_id));
        has_return_value_from_invoking = true;
      }
    }
  };

  using FrameStack = stack<RuntimeFrame, vector<RuntimeFrame>>;

  struct _IgnoredException : std::exception {};
  struct _CustomError : std::exception {
  public:
    //TODO:Memory Management
    _CustomError(const char *msg) : 
      std::exception(std::runtime_error(msg)) {}
  };

  //TODO: new argument generator and storage?
  class Machine {
  private:
    StandardLogger *logger_;
    bool is_logger_host_;

  private:
    void RecoverLastState();
    void FinishInitalizerCalling();
    bool IsTailRecursion(size_t idx, VMCode *code);
    bool IsTailCall(size_t idx);

    Object *FetchLiteralObject(Argument &arg);
    Object FetchFunctionObject(string id);
    ObjectView FetchObjectView(Argument &arg);

    bool FetchFunctionImplEx(FunctionImplPointer &dest, string id, string type_id = kTypeIdNull, 
      Object *obj_ptr = nullptr);

    bool FetchFunctionImpl(FunctionImplPointer &impl, CommandPointer &command,
      ObjectMap &obj_map);

    void CheckDomainObject(FunctionImpl &impl, Request &req, bool first_assert);
    void CheckArgrumentList(FunctionImpl &impl, ArgumentList &args);
    void ClosureCatching(ArgumentList &args, size_t nest_end, bool closure);

    Message CallMethod(Object &obj, string id, ObjectMap &args);
    Message CallMethod(Object &obj, string id,
      const initializer_list<NamedObject> &&args = {});
    Message CallVMCFunction(FunctionImpl &impl, ObjectMap &obj_map);

    void CommandIfOrWhile(Keyword token, ArgumentList &args, size_t nest_end);
    void CommandForEach(ArgumentList &args, size_t nest_end);
    void ForEachChecking(ArgumentList &args, size_t nest_end);
    void CommandCase(ArgumentList &args, size_t nest_end);
    void CommandElse();
    void CommandWhen(ArgumentList &args);
    void CommandContinueOrBreak(Keyword token, size_t escape_depth);
    void CommandStructBegin(ArgumentList &args);
    void CommandModuleBegin(ArgumentList &args);
    void CommandConditionEnd();
    void CommandLoopEnd(size_t nest);
    void CommandForEachEnd(size_t nest);
    void CommandStructEnd();
    void CommandModuleEnd();
    void CommandInclude(ArgumentList &args);
    void CommandSuper(ArgumentList &args);
    void CommandAttribute(ArgumentList &args);

    void CommandSwap(ArgumentList &args);
    void CommandSwapIf(ArgumentList &args);
    void CommandBind(ArgumentList &args, bool local_value, bool ext_value);
    void CommandDelivering(ArgumentList &args, bool local_value, bool ext_value);
    void CommandTypeId(ArgumentList &args);
    void CommandMethods(ArgumentList &args);
    void CommandExist(ArgumentList &args);
    void CommandNullObj(ArgumentList &args);
    void CommandToString(ArgumentList &args);
    void CommandUsing(ArgumentList &args);
    void CommandPrint(ArgumentList &args);
    void CommandInput(ArgumentList &args);
    void CommandGetChar(ArgumentList &args);
    void SysCommand(ArgumentList &args);
    void CommandSleep(ArgumentList &args);

    //TODO: Smaller command implementations
    //void CommandJumpIf();
    //void CommandJumpIfNot();

    void CommandTime();
    void CommandVersion();
    void CommandMachineCodeName();

    template <Keyword op_code>
    void BinaryMathOperatorImpl(ArgumentList &args);

    template <Keyword op_code>
    void BinaryLogicOperatorImpl(ArgumentList &args);

    void OperatorIncreasing(ArgumentList &args);
    void OperatorDecreasing(ArgumentList &args);

    void OperatorLogicNot(ArgumentList &args);

    void ExpList(ArgumentList &args);
    void InitArray(ArgumentList &args);
    void CommandReturn(ArgumentList &args);
    void CommandAssert(ArgumentList &args);
    void DomainAssert(ArgumentList &args);
    void CommandIsBaseOf(ArgumentList &args);
    void CommandHasBehavior(ArgumentList &args);
    template <ParameterPattern pattern>
    void CommandCheckParameterPattern(ArgumentList &args);
    void CommandOptionalParamRange(ArgumentList &args);

    void MachineCommands(Keyword token, ArgumentList &args, Request &request);

    void GenerateArgs(FunctionImpl &impl, ArgumentList &args, ObjectMap &obj_map);
    void Generate_Fixed(FunctionImpl &impl, ArgumentList &args, ObjectMap &obj_map);
    void Generate_AutoSize(FunctionImpl &impl, ArgumentList &args, ObjectMap &obj_map);
    void Generate_AutoFill(FunctionImpl &impl, ArgumentList &args, ObjectMap &obj_map);
    void CallExtensionFunction(ObjectMap &p, FunctionImpl &impl);
    void GenerateStructInstance(ObjectMap &p);
    void GenerateErrorMessages(size_t stop_index);
  private:
    deque<VMCodePointer> code_stack_;
    FrameStack frame_stack_;
    ObjectStack obj_stack_;
    unordered_map<size_t, FunctionImplPointer> impl_cache_;
    vector<ObjectCommonSlot> view_delegator_;
    bool error_;

  public:
    ~Machine() { if (is_logger_host_) delete logger_; }
    Machine() = delete;
    Machine(const Machine &rhs) = delete;
    Machine(const Machine &&rhs) = delete;
    void operator=(const Machine &) = delete;
    void operator=(const Machine &&) = delete;

    Machine(VMCode &ir, string log_path, bool rtlog = false) :
      logger_(nullptr),
      is_logger_host_(true),
      code_stack_(),
      frame_stack_(),
      obj_stack_(),
      error_(false) { 

      code_stack_.push_back(&ir); 
      logger_ = rtlog ?
        (StandardLogger *)new StandardRTLogger(log_path, "a") :
        (StandardLogger *)new StandardCachedLogger(log_path, "a");
    }

    Machine(VMCode &ir, StandardLogger *logger) :
      logger_(logger),
      is_logger_host_(false),
      code_stack_(),
      frame_stack_(),
      obj_stack_(),
      error_(false) {

      code_stack_.push_back(&ir);
    }

    bool PushObject(string id, Object object);
    void PushError(string msg);

    void SetPreviousStack(ObjectStack &prev) {
      obj_stack_.SetPreviousStack(prev);
    }

    void SetDelegatedRoot(ObjectContainer &root) {
      obj_stack_.SetDelegatedRoot(root);
    }

    void Run(bool invoke = false);

    bool ErrorOccurred() const {
      return error_;
    }
  };
}
