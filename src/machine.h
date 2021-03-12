#pragma once
#include "frontend.h"
#include "management.h"
#include "components.h"

#define CHECK_PRINT_OPT(_Map)                          \
  if (_Map.find(kStrSwitchLine) != p.end()) {          \
    putc('\n', VM_STDOUT);                             \
  }

namespace sapphire {
  enum class PlainType {
    Int = 1,
    Float = 2,
    String = 3,
    Bool = 4,
    Invalid = -1
  };

  using Expect = pair<string, string>;
  using ExpectationList = initializer_list<Expect>;
  using NullableList = initializer_list<string>;

//  CommentedResult TypeChecking(ExpectationList &&lst,
//    ObjectMap &obj_map,
//    NullableList &&nullable = {});
//
//#define TC_ERROR(_Obj) Message(std::get<string>(_Obj), StateLevel::Error)
//#define TC_FAIL(_Obj) !std::get<bool>(_Obj)

  string ParseRawString(const string &src);
  void InitPlainTypesAndConstants();
  void ActivateComponents();
  void ReceiveError(void* vm, const char* msg);

  using ResultTraitKey = pair<PlainType, PlainType>;
  using TraitUnit = pair<ResultTraitKey, PlainType>;
  const map<ResultTraitKey, PlainType> kResultDynamicTraits = {
    TraitUnit(ResultTraitKey(PlainType::Int, PlainType::Int), PlainType::Int),
    TraitUnit(ResultTraitKey(PlainType::Int, PlainType::Float), PlainType::Float),
    TraitUnit(ResultTraitKey(PlainType::Int, PlainType::String), PlainType::String),
    TraitUnit(ResultTraitKey(PlainType::Int, PlainType::Bool), PlainType::Int),
    TraitUnit(ResultTraitKey(PlainType::Float, PlainType::Float), PlainType::Float),
    TraitUnit(ResultTraitKey(PlainType::Float, PlainType::Int), PlainType::Float),
    TraitUnit(ResultTraitKey(PlainType::Float, PlainType::String), PlainType::String),
    TraitUnit(ResultTraitKey(PlainType::Float, PlainType::Bool), PlainType::Float),
    TraitUnit(ResultTraitKey(PlainType::String, PlainType::String), PlainType::String),
    TraitUnit(ResultTraitKey(PlainType::String, PlainType::Int), PlainType::String),
    TraitUnit(ResultTraitKey(PlainType::String, PlainType::Float), PlainType::String),
    TraitUnit(ResultTraitKey(PlainType::String, PlainType::Bool), PlainType::String),
    TraitUnit(ResultTraitKey(PlainType::Bool, PlainType::Bool), PlainType::Bool),
    TraitUnit(ResultTraitKey(PlainType::Bool, PlainType::Int), PlainType::Int),
    TraitUnit(ResultTraitKey(PlainType::Bool, PlainType::Float), PlainType::Float),
    TraitUnit(ResultTraitKey(PlainType::Bool, PlainType::String), PlainType::String)
  };

  template <typename ResultType, class Tx, class Ty, Operation op>
  struct BinaryOpBox {
    ResultType Do(Tx A, Ty B) {
      return Tx();
    }
  };

  template <typename ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, Operation::Plus> {
    ResultType Do(Tx A, Ty B) {
      return A + B;
    }
  };

  template <typename ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, Operation::Minus> {
    ResultType Do(Tx A, Ty B) {
      return A - B;
    }
  };

  template <typename ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, Operation::Times> {
    ResultType Do(Tx A, Ty B) {
      return A * B;
    }
  };

  template <typename ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, Operation::Divide> {
    ResultType Do(Tx A, Ty B) {
      return A / B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, Operation::Equals> {
    bool Do(Tx A, Ty B) {
      return A == B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, Operation::LessOrEqual> {
    bool Do(Tx A, Ty B) {
      return A <= B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, Operation::GreaterOrEqual> {
    bool Do(Tx A, Ty B) {
      return A >= B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, Operation::NotEqual> {
    bool Do(Tx A, Ty B) {
      return A != B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, Operation::Greater> {
    bool Do(Tx A, Ty B) {
      return A > B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, Operation::Less> {
    bool Do(Tx A, Ty B) {
      return A < B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, Operation::And> {
    bool Do(Tx A, Ty B) {
      return A && B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, Operation::Or> {
    bool Do(Tx A, Ty B) {
      return A || B;
    }
  };

  //Dispose divide operation for bool type
  template <>
  struct BinaryOpBox<bool, bool, bool, Operation::Divide> {
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

  DISPOSE_STRING_MATH_OP(Operation::Minus);
  DISPOSE_STRING_MATH_OP(Operation::Times);
  DISPOSE_STRING_MATH_OP(Operation::Divide);
  DISPOSE_STRING_LOGIC_OP(Operation::LessOrEqual);
  DISPOSE_STRING_LOGIC_OP(Operation::GreaterOrEqual);
  DISPOSE_STRING_LOGIC_OP(Operation::Greater);
  DISPOSE_STRING_LOGIC_OP(Operation::Less);
  DISPOSE_STRING_LOGIC_OP(Operation::And);
  DISPOSE_STRING_LOGIC_OP(Operation::Or);

#undef DISPOSE_STRING_MATH_OP
#undef DISPOSE_STRING_LOGIC_OP

  template <typename ResultType, Operation op>
  using MathBox = BinaryOpBox<ResultType, ResultType, ResultType, op>;

  template <typename Tx, Operation op>
  using LogicBox = BinaryOpBox<bool, Tx, Tx, op>;

  const string kIteratorBehavior = "obj|step_forward|compare";
  const string kContainerBehavior = "head|tail|empty";
  const string kForEachExceptions = "!iterator|!container_keepalive";

  using CommandPointer = Sentense * ;

  struct RuntimeInfo {
    //TODO: put some boolean values into heres
  };

  class RuntimeFrame {
  public:
    bool error;
    bool warning;
    bool from_continue;
    bool from_break;
    bool void_call;
    bool disable_step;
    bool final_cycle;
    bool jump_from_end;
    bool do_initializer_calling;
    bool inside_initializer_calling;
    bool stop_point;
    bool has_return_value_from_invoking;
    bool keep_condition;
    bool is_command;
    bool rstk_operated;
    AnnotatedAST *current_code;
    Object struct_base;
    Object assert_rc_copy;
    size_t jump_offset;
    size_t idx;
    string msg_string;
    string function_scope;
    string struct_id;
    string super_struct_id;
    stack<bool> condition_stack; //preserved
    stack<bool> scope_indicator;
    stack<size_t> jump_stack;
    stack<size_t> branch_jump_stack;
    vector<ObjectCommonSlot> return_stack;

    RuntimeFrame(string scope = kStrRootScope) :
      error(false),
      warning(false),
      from_continue(false),
      from_break(false),
      void_call(false),
      disable_step(false),
      final_cycle(false),
      jump_from_end(false),
      do_initializer_calling(false),
      inside_initializer_calling(false),
      stop_point(false),
      has_return_value_from_invoking(false),
      keep_condition(false),
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

  using FrameStack = stack<RuntimeFrame, deque<RuntimeFrame>>;

  class Machine {
  private:
    StandardLogger *logger_;
    bool is_logger_host_;

  private:
    void RecoverLastState(bool call_by_return);
    void FinishInitalizerCalling();
    bool IsTailRecursion(size_t idx, AnnotatedAST *code);
    bool IsTailCall(size_t idx);

    Object *FetchLiteralObject(Argument &arg);
    ObjectView FetchObjectView(Argument &arg);
    bool CheckObjectBehavior(Object &obj, string behaviors);
    bool CheckObjectMethod(Object &obj, string id);
    void GetObjectMethods(Object &obj, vector<string> &dest);

    bool FetchFunctionImplEx(FunctionImplPointer &dest, string func_id, string type_id = kTypeIdNull, 
      Object *obj_ptr = nullptr);

    bool FetchFunctionImpl(FunctionImplPointer &impl, CommandPointer &command,
      ObjectMap &obj_map);

    void CheckDomainObject(Function &impl, ASTNode &req, bool first_assert);
    void CheckArgrumentList(Function &impl, ArgumentList &args);
    void ClosureCatching(ArgumentList &args, size_t nest_end, bool closure);

    Message CallMethod(Object &obj, string id, ObjectMap &args);
    Message CallMethod(Object &obj, string id,
      const initializer_list<NamedObject> &&args = {});
    Message CallVMCFunction(Function &impl, ObjectMap &obj_map);

    void CommandLoad(ArgumentList &args);
    void CommandIfOrWhile(Operation token, ArgumentList &args, size_t nest_end);
    void InitForEach(ArgumentList &args, size_t nest_end);
    void CheckForEach(ArgumentList &args, size_t nest_end);
    
    //void CommandForEach(ArgumentList &args, size_t nest_end);
    //void ForEachChecking(ArgumentList &args, size_t nest_end);
    void CommandCase(ArgumentList &args, size_t nest_end);
    void CommandElse();
    void CommandWhen(ArgumentList &args);
    void CommandContinueOrBreak(Operation token, size_t escape_depth);
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

    void CommandTime();
    void CommandVersion();
    void CommandMachineCodeName();

    template <Operation op_code>
    void BinaryMathOperatorImpl(ArgumentList &args);

    template <Operation op_code>
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

    void MachineCommands(Operation token, ArgumentList &args, ASTNode &request);

    void GenerateArgs(Function &impl, ArgumentList &args, ObjectMap &obj_map);
    void Generate_Fixed(Function &impl, ArgumentList &args, ObjectMap &obj_map);
    void Generate_AutoSize(Function &impl, ArgumentList &args, ObjectMap &obj_map);
    void Generate_AutoFill(Function &impl, ArgumentList &args, ObjectMap &obj_map);
    void CallExtensionFunction(ObjectMap &p, Function &impl);
    void GenerateStructInstance(ObjectMap &p);
    void GenerateErrorMessages(size_t stop_index);
  private:
    deque<AASTPointer> code_stack_;
    FrameStack frame_stack_;
    ObjectStack obj_stack_;
    vector<ObjectCommonSlot> view_delegator_;
    bool delegated_base_scope_;
    bool error_;
    

  public:
    ~Machine() { if (is_logger_host_) delete logger_; }
    Machine() = delete;
    Machine(const Machine &rhs) = delete;
    Machine(const Machine &&rhs) = delete;
    void operator=(const Machine &) = delete;
    void operator=(const Machine &&) = delete;

    Machine(AnnotatedAST &ir, string log_path, bool rtlog = false) :
      logger_(nullptr),
      is_logger_host_(true),
      code_stack_(),
      frame_stack_(),
      obj_stack_(),
      delegated_base_scope_(false),
      error_(false) { 

      code_stack_.push_back(&ir); 
      logger_ = rtlog ?
        (StandardLogger *)new StandardRTLogger(log_path, "a") :
        (StandardLogger *)new StandardCachedLogger(log_path, "a");
    }

    Machine(AnnotatedAST &ir, StandardLogger *logger) :
      logger_(logger),
      is_logger_host_(false),
      code_stack_(),
      frame_stack_(),
      obj_stack_(),
      delegated_base_scope_(false),
      error_(false) {

      code_stack_.push_back(&ir);
    }

    bool PushObject(string id, Object object);
    void PushError(string msg);
    void CopyComponents();
    void Run(bool invoke = false);

    void SetPreviousStack(ObjectStack &prev) {
      obj_stack_.SetPreviousStack(prev);
    }

    void SetDelegatedRoot(ObjectContainer &root) {
      obj_stack_.SetDelegatedRoot(root);
      delegated_base_scope_ = true;
    }

    bool ErrorOccurred() const {
      return error_;
    }
  };
}
