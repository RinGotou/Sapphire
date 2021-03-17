#include "machine.h"

#define EXPECTED_COUNT(_Count) (args.size() == _Count)

namespace sapphire {
  inline PlainType FindTypeCode(string type_id) {
    PlainType type = PlainType::Invalid;

    if (type_id == kTypeIdInt) type = PlainType::Int;
    if (type_id == kTypeIdFloat) type = PlainType::Float;
    if (type_id == kTypeIdString) type = PlainType::String;
    if (type_id == kTypeIdBool) type = PlainType::Bool;

    return type;
  }

  inline bool IsIllegalStringOperator(Operation operation) {
    return operation != Operation::Plus && operation != Operation::NotEqual
      && operation != Operation::Equals;
  }

  inline int64_t IntProducer(Object &obj) {
    int64_t result = 0;

    if (obj.GetTypeId() == kTypeIdInt) {
      result = obj.Cast<int64_t>();
    }
    else {
      switch (auto type = FindTypeCode(obj.GetTypeId()); type) {
      case PlainType::Float:result = static_cast<int64_t>(obj.Cast<double>()); break;
      case PlainType::Bool:result = obj.Cast<bool>() ? 1 : 0; break;
      default:break;
      }
    }

    return result;
  }

  inline double FloatProducer(Object &obj) {
    double result = 0;

    if (obj.GetTypeId() == kTypeIdFloat) {
      result = obj.Cast<double>();
    }
    else {
      switch (auto type = FindTypeCode(obj.GetTypeId()); type) {
      case PlainType::Int:result = static_cast<double>(obj.Cast<int64_t>()); break;
      case PlainType::Bool:result = obj.Cast<bool>() ? 1.0 : 0.0; break;
      default:break;
      }
    }

    return result;
  }

  inline string StringProducer(Object &obj) {
    string result;

    if (obj.GetTypeId() == kTypeIdString) {
      result = obj.Cast<string>();
    }
    else {
      switch (auto type = FindTypeCode(obj.GetTypeId()); type) {
      case PlainType::Float:result = to_string(obj.Cast<double>()); break;
      case PlainType::Bool:result = obj.Cast<bool>() ? kStrTrue : kStrFalse; break;
      case PlainType::Int:result = to_string(obj.Cast<int64_t>()); break;
      default:break;
      }
    }

    return result;
  }

  bool BoolProducer(Object &obj) {
    if (obj.GetTypeId() == kTypeIdBool) {
      return obj.Cast<bool>();
    }

    auto type = FindTypeCode(obj.GetTypeId());
    bool result = false;


    switch (type) {
    case PlainType::Int: result = obj.Cast<int64_t>() > 0; break;
    case PlainType::Float: result = obj.Cast<double>() > 0.0; break;
    case PlainType::Bool: result = obj.Cast<bool>(); break;
    case PlainType::String: result = !obj.Cast<string>().empty(); break;
    default:
      break;
    }

    return result;
  }
  
  string ParseRawString(const string & src) {
    string result = src;
    if (lexical::IsString(result)) result = lexical::GetRawString(result);
    return result;
  }

  void InitPlainTypesAndConstants() {
    using namespace components;
    using namespace constant;

    CreateStruct(kTypeIdInt);
    CreateStruct(kTypeIdFloat);
    CreateStruct(kTypeIdBool);
    CreateStruct(kTypeIdNull);
    CreateStruct(kTypeIdFunctionPointer);
    CreateStruct(kTypeIdObjectPointer);

    EXPORT_CONSTANT(kTypeIdInt);
    EXPORT_CONSTANT(kTypeIdFloat);
    EXPORT_CONSTANT(kTypeIdBool);
    EXPORT_CONSTANT(kTypeIdNull);
    EXPORT_CONSTANT(kPlatformType);
    EXPORT_CONSTANT(kTypeIdFunctionPointer);
    EXPORT_CONSTANT(kTypeIdObjectPointer);
    CreateConstantObject("kCoreFilename", Object(runtime::GetBinaryName()));
    CreateConstantObject("kCorePath", Object(runtime::GetBinaryPath()));
  }

  void ActivateComponents() {
    InitPlainTypesAndConstants();

    for (const auto func : kEmbeddedComponents) func();
  }

  void ReceiveExtReturningValue(void *value, void *slot, int type) {
    auto &slot_obj = *static_cast<Object *>(slot);

    if (type == kExtTypeInt) {
      auto *ret_value = static_cast<int64_t *>(value);
      slot_obj.PackContent(make_shared<int64_t>(*ret_value), kTypeIdInt);
    }
    else if (type == kExtTypeFloat) {
      auto *ret_value = static_cast<double *>(value);
      slot_obj.PackContent(make_shared<double>(*ret_value), kTypeIdFloat);
    }
    else if (type == kExtTypeBool) {
      auto *ret_value = static_cast<int *>(value);
      bool content = *ret_value == 1 ? true : false;
      slot_obj.PackContent(make_shared<bool>(content), kTypeIdBool);
    }
    else if (type == kExtTypeString) {
      const auto *ret_value = static_cast<char *>(value);
      string content(ret_value);
      slot_obj.PackContent(make_shared<string>(content), kTypeIdString);
    }
    else if (type == kExtTypeWideString) {
      const auto *ret_value = static_cast<wchar_t *>(value);
      wstring content(ret_value);
      slot_obj.PackContent(make_shared<wstring>(content), kTypeIdWideString);
    }
    else if (type == kExtTypeFunctionPointer) {
      const auto *ret_value = static_cast<GenericFunctionPointer *>(value);
      slot_obj.PackContent(make_shared<GenericFunctionPointer>(*ret_value), kTypeIdFunctionPointer);
    }
    else if (type == kExtTypeObjectPointer) {
      const auto *ret_value = static_cast<uintptr_t *>(value);
      slot_obj.PackContent(make_shared<GenericPointer>(*ret_value), kTypeIdObjectPointer);
    }
    else {
      slot_obj.PackContent(nullptr, kTypeIdNull);
    }
  }

  int PushObjectToVM(const char *id, void *ptr, const char *type_id, ExternalMemoryDisposer disposer,
    void *vm) {
    auto &machine = *static_cast<AASTMachine *>(vm);
    Object ext_obj(ptr, disposer, string(type_id));
    auto result = machine.PushObject(string(id), ext_obj);
    return result ? 1 : 0;
  }

  void ReceiveError(void *vm, const char *msg) {
    auto &machine = *static_cast<AASTMachine *>(vm);
    machine.PushError(string(msg));
  }

  void RuntimeFrame::Stepping() {
    if (!disable_step) idx += 1;
    disable_step = false;
  }

  void RuntimeFrame::Goto(size_t target_idx) {
    idx = target_idx - jump_offset;
    disable_step = true;
  }

  void RuntimeFrame::AddJumpRecord(size_t target_idx) {
    if (jump_stack.empty() || jump_stack.top() != target_idx) {
      jump_stack.push(target_idx);
    }
  }

  void RuntimeFrame::MakeError(string str) {
    error = true;
    msg_string = str;
  }

  void RuntimeFrame::MakeWarning(string str) {
    warning = true;
    msg_string = str;
  }

  void RuntimeFrame::RefreshReturnStack(Object &obj) {
    if (!void_call) {
      return_stack.push_back(new Object(obj));
      has_return_value_from_invoking = stop_point;
    }

    if (is_command) rstk_operated = true;
  }

  void RuntimeFrame::RefreshReturnStack(Object &&obj) {
    if (!void_call) {
      return_stack.push_back(new Object(std::move(obj)));
      has_return_value_from_invoking = stop_point;
    }

    if (is_command) rstk_operated = true;
  }

  void RuntimeFrame::RefreshReturnStack(const ObjectInfo &info, const shared_ptr<void> &ptr) {
    if (!void_call) {
      return_stack.push_back(new Object(info, ptr));
      has_return_value_from_invoking = stop_point;
    }

    if (is_command) rstk_operated = true;
  }
  
  void RuntimeFrame::RefreshReturnStack(bool value) {
    if (!void_call) {
      return_stack.push_back(new Object(value, kTypeIdBool));
      has_return_value_from_invoking = stop_point;
    }
    
    if (is_command) rstk_operated = true;
  }

  void RuntimeFrame::RefreshReturnStack(ObjectView &&view) {
    if (!void_call) {
      return_stack.push_back(new ObjectView(view));
      has_return_value_from_invoking = stop_point;
    }

    if (is_command) rstk_operated = true;
  }

  void AASTMachine::RecoverLastState(bool call_by_return) {
    ObjectView view(obj_stack_.GetCurrent().Find(kStrReturnValueConstrantObj));

    if (view.IsValid() && !call_by_return) { // no need to check alive state
      string_view constraint_type(view.Seek().Cast<string>());
      if (constraint_type != kTypeIdNull) {
        frame_stack_.top().MakeError("Mismatched return value type: null");
        return;
      }
    }

    frame_stack_.pop();
    code_stack_.pop_back();
    obj_stack_.Pop();
    
    if (!call_by_return) frame_stack_.top().RefreshReturnStack(Object());
  }

  void AASTMachine::FinishInitalizerCalling() {
    auto instance_obj = *obj_stack_.GetCurrent().Find(kStrMe);
    instance_obj.SetDeliveringFlag();
    RecoverLastState(false);
    frame_stack_.top().RefreshReturnStack(instance_obj);
  }

  bool AASTMachine::IsTailRecursion(size_t idx, AnnotatedAST *code) {
    if (code != code_stack_.back()) return false;

    auto &vmcode = *code;
    auto &current = vmcode[idx];
    bool result = false;

    if (idx == vmcode.size() - 1) {
      result = true;
    }
    else if (idx == vmcode.size() - 2) {
      bool needed_by_next_call =
        vmcode[idx + 1].first.GetOperation() == Operation::Return &&
        vmcode[idx + 1].second.back().GetType() == ArgumentType::RetStack &&
        vmcode[idx + 1].second.size() == 1;
      if (!current.first.annotation.void_call && needed_by_next_call) {
        result = true;
      }
    }

    return result;
  }

  bool AASTMachine::IsTailCall(size_t idx) {
    if (frame_stack_.size() <= 1) return false;
    auto &vmcode = *code_stack_.back();
    bool result = false;

    if (idx == vmcode.size() - 1) {
      result = true;
    }
    else if (idx == vmcode.size() - 2) {
      bool needed_by_next_call = 
        vmcode[idx + 1].first.GetOperation() == Operation::Return &&
        vmcode[idx + 1].second.back().GetType() == ArgumentType::RetStack &&
        vmcode[idx + 1].second.size() == 1;
      if (!vmcode[idx].first.annotation.void_call && needed_by_next_call) {
        result = true;
      }
    }

    return result;
  }

  Object *AASTMachine::FetchLiteralObject(Argument &arg) {
    using namespace constant;
    auto value = arg.GetData();

    auto *ptr = GetConstantObject(value);

    if (ptr != nullptr) return ptr;

    auto type = arg.GetStringType();
    if (type == LiteralType::Int) {
      int64_t int_value;
      from_chars(value.data(), value.data() + value.size(), int_value);
      ptr = CreateConstantObject(value, Object(int_value, kTypeIdInt));
    }
    else if (type == LiteralType::Float) {
      double float_value;
#ifndef _MSC_VER
      //dealing with issues of charconv implementation in low-version clang
      float_value = stod(value);
#else
      from_chars(value.data(), value.data() + value.size(), float_value);
#endif
      ptr = CreateConstantObject(value, Object(float_value, kTypeIdFloat));
    }
    else {
      switch (type) {
      case LiteralType::Bool:
        ptr = CreateConstantObject(value, Object(value == kStrTrue, kTypeIdBool));
        break;
      case LiteralType::String:
        ptr = CreateConstantObject(value, Object(ParseRawString(value)));
        break;
        //for binding expression
      case LiteralType::Identifier:
        ptr = CreateConstantObject(value, Object(value));
        break;
      default:
        break;
      }
    }

    return ptr;
  }

  ObjectView AASTMachine::FetchObjectView(Argument &arg) {
    using namespace constant;

#define OBJECT_DEAD_MSG {                           \
      frame.MakeError("Referenced object is dead"); \
      return ObjectView();                          \
    }

#define MEMBER_NOT_FOUND_MSG {                                                                          \
      frame.MakeError("Member '" + arg.GetData() + "' is not found inside " + arg.properties.domain.id);\
      return ObjectView();                                                                              \
    }

    auto &frame = frame_stack_.top();
    auto &return_stack = frame.return_stack;
    ObjectPointer ptr = nullptr;
    ObjectView view;

    if (arg.GetType() == ArgumentType::Literal) {
      view = FetchLiteralObject(arg);
      view.source = ObjectViewSource::Literal;
    }
    else if (arg.GetType() == ArgumentType::Pool) {
      if (!arg.properties.domain.id.empty() || arg.properties.member_access.use_last_assert) {
        if (arg.properties.member_access.use_last_assert) {
          auto &base = frame.assert_rc_copy.Cast<ObjectStruct>();
          ptr = base.Find(arg.GetData());

          if (ptr != nullptr) {
            if (!ptr->IsAlive()) OBJECT_DEAD_MSG;
            view = ObjectView(ptr);
          }
          else MEMBER_NOT_FOUND_MSG;

          if (arg.properties.member_access.is_chain_tail) frame.assert_rc_copy = Object();
        }
        else if (arg.properties.domain.type == ArgumentType::Pool) {
          ptr = obj_stack_.Find(arg.GetData(), arg.properties.domain.id, arg.properties.token_id);

          if (ptr != nullptr) {
            if (!ptr->IsAlive()) OBJECT_DEAD_MSG;
            view = ObjectView(ptr);
          }
          else MEMBER_NOT_FOUND_MSG;
        }
        else if (arg.properties.domain.type == ArgumentType::RetStack) {
          auto &sub_container = return_stack.back()->IsObjectView() ?
            dynamic_cast<ObjectView *>(return_stack.back())->Seek().Cast<ObjectStruct>() :
            dynamic_cast<ObjectPointer>(return_stack.back())->Cast<ObjectStruct>();
          ptr = sub_container.Find(arg.GetData());
          //keep object alive
          if (ptr != nullptr) {
            if (!ptr->IsAlive()) OBJECT_DEAD_MSG;
            view_delegator_.emplace_back(new Object(*ptr));
            view = ObjectView(dynamic_cast<ObjectPointer>(view_delegator_.back()));
            delete return_stack.back();
            return_stack.pop_back();
          }
          else MEMBER_NOT_FOUND_MSG;
        }
      }
      else {
        if (ptr = obj_stack_.Find(arg.GetData(), arg.properties.token_id); ptr != nullptr) {
          if (!ptr->IsAlive()) OBJECT_DEAD_MSG;
          view = ObjectView(ptr);
        }
        else if (ptr = GetConstantObject(arg.GetData()); ptr != nullptr) {
          view = ObjectView(ptr);
        }
        else {
          frame.MakeError("Object is not found: " + arg.GetData());
        }
      }
      view.source = ObjectViewSource::Ref;
    }
    else if (arg.GetType() == ArgumentType::RetStack) {
      if (!return_stack.empty()) {
        if (!return_stack.back()->IsAlive()) OBJECT_DEAD_MSG;
        view_delegator_.emplace_back(return_stack.back());
        if (view_delegator_.back()->IsObjectView()) {
          view = *dynamic_cast<ObjectView *>(view_delegator_.back());
        }
        else {
          view = ObjectView(
            dynamic_cast<ObjectPointer>(view_delegator_.back()));
          view.Seek().SeekDeliveringFlag();
        }
        
        return_stack.pop_back();
      }
      else {
        frame.MakeError("Can't get object from stack(Internal error)");
      }
      view.source = ObjectViewSource::Ref;
    }

#undef OBJECT_DEAD_MSG
#undef MEMBER_NOT_FOUND_MSG
    return view;
  }

  bool AASTMachine::CheckObjectBehavior(Object &obj, string behaviors) {
    auto sample = BuildStringVector(behaviors);
    bool result = true;

    auto do_checking = [&sample, &result](ObjectStruct &base) -> void {
      for (const auto &unit : sample) {
        auto ptr = base.Find(unit);

        if (ptr == nullptr) result = false;
        else if (ptr->GetTypeId() != kTypeIdFunction) result = false;

        if (!result) return;
      }
    };

    if (obj.IsSubContainer()) {
      //open this object directly
      auto &base = obj.Cast<ObjectStruct>();
      do_checking(base);
    }
    else {
      //find object type struct and check the methods
      auto *struct_obj_ptr = obj_stack_.Find(obj.GetTypeId(), TryAppendTokenId(obj.GetTypeId()));
      if (struct_obj_ptr != nullptr && struct_obj_ptr->IsSubContainer()) {
        auto &base = struct_obj_ptr->Cast<ObjectStruct>();
        do_checking(base);
      }
      else result = false;
    }

    return result;
  }

  bool AASTMachine::CheckObjectMethod(Object &obj, string id) {
    bool result = false;

    if (obj.IsSubContainer()) {
      auto &base = obj.Cast<ObjectStruct>();
      auto ptr = base.Find(id);
      result = (ptr != nullptr && ptr->GetTypeId() == kTypeIdFunction);
    }
    else {
      auto *struct_obj_ptr = obj_stack_.Find(obj.GetTypeId(), TryAppendTokenId(obj.GetTypeId()));
      if (struct_obj_ptr != nullptr && struct_obj_ptr->IsSubContainer()) {
        auto &base = struct_obj_ptr->Cast<ObjectStruct>();
        auto ptr = base.Find(id);
        result = (ptr != nullptr && ptr->GetTypeId() == kTypeIdFunction);
      }
    }

    return result;
  }

  void AASTMachine::GetObjectMethods(Object &obj, vector<string> &dest) {
    if (obj.IsSubContainer()) {
      auto &base = obj.Cast<ObjectStruct>().GetContent();

      for (const auto &unit : base) dest.emplace_back(unit.first);
      //apppend 'members' method
      if (obj.GetTypeId() == kTypeIdStruct) dest.emplace_back("members");
    }
    else {
      auto *struct_obj_ptr = obj_stack_.Find(obj.GetTypeId(), TryAppendTokenId(obj.GetTypeId()));
      if (struct_obj_ptr != nullptr && struct_obj_ptr->IsSubContainer()) {
        auto &base = struct_obj_ptr->Cast<ObjectStruct>().GetContent();
        for (const auto &unit : base) dest.emplace_back(unit.first);
      }
    }
  }

  bool AASTMachine::FetchFunctionImplEx(FunctionImplPointer &dest, string func_id, string type_id,
    Object *obj_ptr) {
    auto &frame = frame_stack_.top();

    //TODO:struct support is missing

#define METHOD_NOT_FOUND_MSG {                                           \
      frame.MakeError("Method of " + type_id + " is not found: " + func_id);  \
      return false;                                                      \
    }
#define TYPE_ERROR_MSG {                                                 \
      frame.MakeError(func_id + " is not a function object");                 \
      return false;                                                      \
    }

    if (type_id != kTypeIdNull) {
      auto struct_obj_ptr = obj_stack_.Find(type_id);

      if (struct_obj_ptr == nullptr) {
        frame.MakeError("Invalid struct: " + type_id);
        return false;
      }

      if (obj_ptr->IsSubContainer()) {
        auto *base_result = obj_ptr->Cast<ObjectStruct>().Find(func_id);
        auto *struct_result = struct_obj_ptr->Cast<ObjectStruct>().Find(func_id);

        auto *result = [&base_result, &struct_result]() -> auto {
          if (base_result != nullptr) return base_result;
          if (struct_result != nullptr) return struct_result;
          return ObjectPointer(nullptr);
        }();

        if (result == nullptr) METHOD_NOT_FOUND_MSG;
        if (result->GetTypeId() != kTypeIdFunction) TYPE_ERROR_MSG;

        dest = &result->Cast<Function>();
      }
      else {
        auto *result = struct_obj_ptr->Cast<ObjectStruct>().Find(func_id);
        if (result == nullptr) METHOD_NOT_FOUND_MSG;
        if (result->GetTypeId() != kTypeIdFunction) TYPE_ERROR_MSG;

        dest = &result->Cast<Function>();
      }
    }
    else {
      if (auto *ptr = obj_stack_.Find(func_id); ptr != nullptr) {
        if (ptr->GetTypeId() != kTypeIdFunction) TYPE_ERROR_MSG;
        dest = &ptr->Cast<Function>();
      }
      //Hint: behavior of initializer?
      //just making error instead for now.
      else METHOD_NOT_FOUND_MSG;
    }

#undef METHOD_NOT_FOUND_MSG
#undef TYPE_ERROR_MSG

    return true;
  }

  bool AASTMachine::FetchFunctionImpl(FunctionImplPointer &impl, CommandPointer &command, ObjectMap &obj_map) {
    auto &frame = frame_stack_.top();
    auto id = command->first.GetFunctionId();
    auto domain = command->first.GetFunctionDomain();

    auto has_domain = domain.GetType() != ArgumentType::Invalid ||
      command->first.annotation.use_last_assert;

    if (has_domain) {
      auto view = command->first.annotation.use_last_assert ?
        ObjectView(&frame.assert_rc_copy) :
        FetchObjectView(domain);

      if (frame.error) return false;
      //find method in sub-container    

      auto struct_obj_ptr = obj_stack_.Find(view.Seek().GetTypeId());

      if (struct_obj_ptr == nullptr || !struct_obj_ptr->IsSubContainer()) {
        frame.MakeError("invalid base type of object: " + view.Seek().GetTypeId());
        return false;
      }

      auto *base_result = view.Seek().IsSubContainer() ?
        view.Seek().Cast<ObjectStruct>().Find(id) : nullptr;
      auto *struct_result = struct_obj_ptr->Cast<ObjectStruct>().Find(id);
      auto *result = [&base_result, &struct_result]() -> auto {
        if (base_result != nullptr) return base_result;
        if (struct_result != nullptr) return struct_result;
        return ObjectPointer(nullptr);
      }();
      
      if (result == nullptr) {
        frame.MakeError("Method is not found: " + id); 
        return false;                                  
      }

      if (result->GetTypeId() != kTypeIdFunction) {
        frame.MakeError(id + " is not a function object");
        return false;
      }

      impl = &result->Cast<Function>();
      

      obj_map.emplace(NamedObject(kStrMe, view.Seek()));
      if (!frame.assert_rc_copy.NullPtr()) frame.assert_rc_copy = Object();
    }
    //Plain bulit-in function and user-defined function
    else {
      ObjectPointer ptr = obj_stack_.Find(id);

      if (ptr == nullptr) {
        frame.MakeError("Function is not found: " + id);
        return false;
      }

      if (!ptr->IsAlive()) {
        frame.MakeError("Referenced object is dead: " + id);
        return false;
      }

      if (ptr->GetTypeId() == kTypeIdFunction) {
        impl = &ptr->Cast<Function>();
      }
      else if (ptr->IsSubContainer() && ptr->GetTypeId() == kTypeIdStruct) {
        auto &base = ptr->Cast<ObjectStruct>();
        auto *initializer_obj = base.Find(kStrInitializer);

        if (initializer_obj == nullptr) {
          frame.MakeError("Struct doesn't have initializer");
          return false;
        }

        impl = &initializer_obj->Cast<Function>();
        if (impl->GetType() == FunctionType::Operation) {
          frame.do_initializer_calling = true;
          frame.struct_base = *ptr;
        }
      }
      else {
        frame.MakeError("Not function object: " + id);
        return false;
      }
    }
#undef METHOD_NOT_FOUND_MSG

    return true;
  }

  void AASTMachine::CheckObjectWithDomain(Function &impl, ASTNode &node, bool first_assert) {
    auto &frame = frame_stack_.top();
    auto domain = node.GetFunctionDomain();
    auto operation = node.GetOperation();
    auto need_catching = domain.GetType() == ArgumentType::Pool
      && ((operation != Operation::DomainAssertCommand)
        || (operation == Operation::DomainAssertCommand && first_assert));

    if (!need_catching) return;
    
    auto view = node.annotation.use_last_assert ?
      ObjectView(&frame.assert_rc_copy) :
      FetchObjectView(domain);

    if (frame.error) return;

    impl.AppendClosureRecord(domain.GetData(), components::DumpObject(view.Seek()));
  }

  void AASTMachine::CheckArgrumentList(Function &impl, ArgumentList &args) {
    auto &frame = frame_stack_.top();
    string_view data;
    ArgumentType type;
    ObjectView view;
    bool need_catching = false;

    for (auto &unit : args) {
      data = unit.GetData();
      type = unit.GetType();

      if (unit.properties.domain.type == ArgumentType::Pool) {
        view = ObjectView(obj_stack_.Find(unit.properties.domain.id, unit.properties.token_id));
      }
      else if (type == ArgumentType::Pool) {
        view = ObjectView(obj_stack_.Find(string(data), unit.properties.token_id));
      }
      else {
        continue;
      }

      if (!view.IsValid()) {
        frame.MakeError("Object is not found: " + unit.properties.domain.id);
        break;
      }

      if (!view.IsAlive()) {
        frame.MakeError("Object is dead: " + unit.properties.domain.id);
        break;
      }

      impl.AppendClosureRecord(data, components::DumpObject(view.Seek()));
    }
  }

  void AASTMachine::ClosureCatching(ArgumentList &args, size_t nest_end, bool closure) {
    auto &frame = frame_stack_.top();
    auto &obj_list = obj_stack_.GetBase();
    auto &origin_code = *code_stack_.back();
    size_t size = args.size();
    size_t nest = frame.idx;
    bool variable = false;
    bool not_assert_lastloop = false;
    bool first_assert = false;
    ParameterPattern argument_mode = ParameterPattern::Fixed;
    vector<string> params;
    AnnotatedAST code(&origin_code);
    string return_value_constraint;
    auto &container = obj_stack_.GetCurrent();
    auto func_id = args[0].GetData();

    for (size_t idx = nest + 1; idx < nest_end - frame.jump_offset; ++idx) {
      code.push_back(origin_code[idx]);
    }

    for (size_t idx = 1; idx < size; idx += 1) {
      auto id = args[idx].GetData();

      if (args[idx].properties.fn.constraint) {
        return_value_constraint = id;
        continue;
      }

      if (args[idx].properties.fn.variable_param) variable = true;

      params.push_back(id);
    }

    if (variable) argument_mode = ParameterPattern::Variable;

    Function impl(nest + 1, code, args[0].GetData(), params, argument_mode);

    if (closure) {
      for (auto it = code.begin(); it != code.end(); ++it) {
        //needed for CheckObjectWithDomain() for judging catching or not
        first_assert = not_assert_lastloop && it->first.GetOperation() == Operation::DomainAssertCommand;
        not_assert_lastloop = it->first.GetOperation() != Operation::DomainAssertCommand;
        //check and push target object into closure scope
        CheckObjectWithDomain(impl, it->first, first_assert);
        CheckArgrumentList(impl, it->second);
      }
    }

    if (!return_value_constraint.empty()) {
      impl.AppendClosureRecord(kStrReturnValueConstrantObj, Object(return_value_constraint));
    }

    
    if (container.Find(args[0].GetData(), false)) {
      frame.MakeError("Function definition confliction");
      return;
    }
    else {
      container.Add(func_id, Object(make_shared<Function>(impl), kTypeIdFunction),
        TryAppendTokenId(func_id));
    }

    frame.Goto(nest_end + 1);
  }

  Message AASTMachine::CallMethod(Object &obj, string id, ObjectMap &args) {
    FunctionImplPointer impl;
    auto &frame = frame_stack_.top();
    Message result;

    if (!FetchFunctionImplEx(impl, id, obj.GetTypeId(), &obj)) return result;

    ObjectMap obj_map = args;
    obj_map.emplace(NamedObject(kStrMe, obj));

    if (impl->GetType() == FunctionType::Operation) {
      result = CallVMCFunction(*impl, obj_map);
    }
    else if (impl->GetType() == FunctionType::External) {
      frame.MakeError("Unsupported feature(Invoke external function)");
    }
    else if (impl->GetType() == FunctionType::Component) {
      auto activity = impl->Get<Activity>();
      result = activity(obj_map);
      //if (!result.HasObject()) {
      //  result.SetObject(Object());
      //}
    }
    else {
      frame.MakeError("Internal Error(Unknown function type)");
    }

    return result;
  }

  Message AASTMachine::CallMethod(Object &obj, string id, const initializer_list<NamedObject> &&args) {
    ObjectMap obj_map = args;
    return CallMethod(obj, id, obj_map);
  }

  Message AASTMachine::CallVMCFunction(Function &impl, ObjectMap &obj_map) {
    auto &frame = frame_stack_.top();
    Message result;

    if (impl.GetType() != FunctionType::Operation) {
      frame.MakeError("Invalid function variant");
      return result;
    }

    frame.stop_point = true;
    code_stack_.push_back(&impl.Get<AnnotatedAST>());
    frame_stack_.push(RuntimeFrame(impl.GetId()));
    frame_stack_.top().jump_offset = impl.GetOffset();
    obj_stack_.Push();
    obj_stack_.CreateObject(kStrUserFunc, Object(impl.GetId()));
    obj_stack_.MergeMap(obj_map);
    obj_stack_.MergeMap(impl.GetClosureRecord());
    Run(true);

    if (error_) {
      frame.MakeError("Error occurred while calling user-defined function");
      return result;
    }

    if (frame.has_return_value_from_invoking) {
      frame.return_stack.back()->IsObjectView() ?
        result.SetObjectRef(dynamic_cast<ObjectView *>(frame.return_stack.back())->Seek()) :
        //not checked. SetObjectRef?
        result.SetObject(*dynamic_cast<ObjectPointer>(frame.return_stack.back()));
      delete frame.return_stack.back();
      frame.return_stack.pop_back();
    }
    frame.stop_point = false;

    return result;
  }

  void AASTMachine::CommandLoad(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    auto view = FetchObjectView(args[0]);
    if (frame.error) return;
    frame.RefreshReturnStack(std::move(view));
  }

  void AASTMachine::CommandIfOrWhile(Operation operation, ArgumentList &args, size_t nest_end) {
    auto &frame = frame_stack_.top();
    auto &code = code_stack_.front();
    bool has_jump_record = false;
    bool state = false;

    if (!EXPECTED_COUNT(1)) {
      frame.MakeError("Argument for condition is missing");
      return;
    }

    if (operation == Operation::If || operation == Operation::While) {
      frame.AddJumpRecord(nest_end);
      has_jump_record = code->FindJumpRecord(frame.idx + frame.jump_offset, frame.branch_jump_stack);
    }
    

    ObjectView view = FetchObjectView(args[0]);

    if (frame.error) return;

    if (view.Seek().GetTypeId() != kTypeIdBool) {
      frame.MakeError("Invalid state value type.");
      return;
    }

    state = view.Seek().Cast<bool>();

    if (operation == Operation::If) {
      auto create_env = [&]()->void {
        frame.scope_indicator.push(true);
        frame.condition_stack.push(state);
        obj_stack_.Push(true, true);
      };

      if (!state) {
        if (frame.branch_jump_stack.empty()) {
          frame.Goto(frame.jump_stack.top());
          frame.keep_condition = true;
        }
        else {
          create_env();
          frame.Goto(frame.branch_jump_stack.top());
          frame.branch_jump_stack.pop();
        }
      }
      else {
        create_env();
      }
    }
    else if (operation == Operation::Elif) {
      if (frame.condition_stack.empty()) {
        frame.MakeError("Unexpected Elif");
        return;
      }

      if (frame.condition_stack.top()) {
        frame.Goto(frame.jump_stack.top());
      }
      else {
        if (state) {
          frame.condition_stack.top() = true;
        }
        else {
          if (frame.branch_jump_stack.empty()) {
            frame.Goto(frame.jump_stack.top());
          }
          else {
            frame.Goto(frame.branch_jump_stack.top());
            frame.branch_jump_stack.pop();
          }
        }
      }
    }
    else if (operation == Operation::While) {
      if (!frame.jump_from_end) {
        frame.scope_indicator.push(true);
        obj_stack_.Push(true, true);
      }
      else {
        frame.jump_from_end = false;
      }

      if (!state) {
        frame.Goto(nest_end);
        frame.final_cycle = true;
      }
    }
  }

  void AASTMachine::InitForEach(ArgumentList &args, size_t nest_end) {
    auto &frame = frame_stack_.top();

    auto container_obj = FetchObjectView(args[1]).Seek();
    auto unit_id = FetchObjectView(args[0]).Seek().Cast<string>();
    auto container_type = container_obj.GetTypeId();

    if (frame.error) return;

    frame.AddJumpRecord(nest_end);

    if (container_type == kTypeIdArray) {
      if (container_obj.Cast<ObjectArray>().empty()) {
        frame.Goto(nest_end);
        frame.final_cycle = true;
        obj_stack_.Push(true);
        frame.scope_indicator.push(false);
        return;
      }

      frame.scope_indicator.push(true);
      obj_stack_.Push(true);
      obj_stack_.CreateObject(kStrContainerKeepAliveSlot, container_obj);
      obj_stack_.CreateObject(kStrIteratorObj, Object(int64_t(0), kTypeIdInt));

      auto container_unit = container_obj.Cast<ObjectArray>().at(0);
      obj_stack_.CreateObject(unit_id, container_unit, TryAppendTokenId(unit_id));
    }
    else if (container_type == kTypeIdTable) {
      if (container_obj.Cast<ObjectTable>().empty()) {
        frame.Goto(nest_end);
        frame.final_cycle = true;
        obj_stack_.Push(true);
        frame.scope_indicator.push(false);
        return;
      }

      frame.scope_indicator.push(true);
      obj_stack_.Push(true);
      obj_stack_.CreateObject(kStrContainerKeepAliveSlot, container_obj);
      
      auto it = container_obj.Cast<ObjectTable>().begin();
      Object iterator_obj(it, kTypeIdAnyStorage);
      obj_stack_.CreateObject(kStrIteratorObj, iterator_obj);

      auto key_ref = it->first;
      auto key_copy = components::DumpObject(key_ref);
      auto container_unit = make_shared<ObjectPair>(key_copy, it->second);
      obj_stack_.CreateObject(unit_id, Object(container_unit, kTypeIdPair), TryAppendTokenId(unit_id));
    }
    else {
      if (!CheckObjectBehavior(container_obj, "head|tail|empty")) {
        frame.MakeError("Container doesn't have necessary methods for enumeration");
        return;
      }

      auto empty_result = CallMethod(container_obj, kStrEmpty).GetObj();
      if (frame.error) return;
      if (empty_result.GetTypeId() != kTypeIdBool) {
        frame.MakeError("Invalid type of return value from empty()");
        return;
      }
      if (empty_result.Cast<bool>()) {
        frame.Goto(nest_end);
        frame.final_cycle = true;
        obj_stack_.Push(true); //avoid error
        frame.scope_indicator.push(false); //????
        return;
      }

      auto iterator_obj = CallMethod(container_obj, kStrHead).GetObj();
      if (frame.error) return;
      if (!CheckObjectBehavior(iterator_obj, "get|step|compare")) {
        frame.MakeError("Head iterator doesn't have necessary methods");
        return;
      }

      auto container_unit = CallMethod(iterator_obj, "get").GetObj();
      if (frame.error) return;

      frame.scope_indicator.push(true);
      obj_stack_.Push(true);
      obj_stack_.CreateObject(kStrIteratorObj, iterator_obj);
      obj_stack_.CreateObject(kStrContainerKeepAliveSlot, container_obj);
      obj_stack_.CreateObject(unit_id, container_unit, TryAppendTokenId(unit_id));

    }
  }

  void AASTMachine::CheckForEach(ArgumentList &args, size_t nest_end) {
    auto &frame = frame_stack_.top();
    auto unit_id = FetchObjectView(args[0]).Seek().Cast<string>();

    if (frame.error) return;

    auto *iterator_obj = obj_stack_.GetCurrent().Find(kStrIteratorObj);
    auto *container_obj = obj_stack_.GetCurrent().Find(kStrContainerKeepAliveSlot);
    //don't use PackObject() here. It will reset pointed object by mistake!
    //auto *container_unit = obj_stack_.GetCurrent().Find(unit_id);
    auto container_type = container_obj->GetTypeId();
    
    if (container_type == kTypeIdArray) {
      auto &index = iterator_obj->Cast<int64_t>() ;
      index += 1;
      if (index == container_obj->Cast<ObjectArray>().size()) {
        frame.Goto(nest_end);
        frame.final_cycle = true;
      }
      else {
        auto &dest = container_obj->Cast<ObjectArray>().at(index);
        obj_stack_.GetCurrent().Replace(unit_id, dest, TryAppendTokenId(unit_id));
      }
    }
    else if (container_type == kTypeIdTable) {
      auto &it = iterator_obj->Cast<ObjectTable::iterator>();
      it.operator++();
      if (it == container_obj->Cast<ObjectTable>().end()) {
        frame.Goto(nest_end);
        frame.final_cycle = true;
      }
      else {
        auto key_ref = it->first;
        auto key_copy = components::DumpObject(key_ref);
        auto unit = make_shared<ObjectPair>(key_copy, it->second);
        obj_stack_.GetCurrent().Replace(unit_id, Object(unit, kTypeIdPair), TryAppendTokenId(unit_id));
      }
    }
    else {
      CallMethod(*iterator_obj, "step");
      if (frame.error) return;

      auto tail_iterator = CallMethod(*container_obj, kStrTail).GetObj();
      if (frame.error) return;
      if (!CheckObjectBehavior(tail_iterator, "get|step|compare")) {
        frame.MakeError("Tail iterator doesn't have necessary methods");
        return;
      }

      auto comp_result = CallMethod(*iterator_obj, "compare",
        { NamedObject(kStrRightHandSide, tail_iterator) }).GetObj();
      if (frame.error) return;
      if (comp_result.GetTypeId() != kTypeIdBool) {
        frame.MakeError("Invalid type of return value from compare()");
        return;
      }

      if (comp_result.Cast<bool>()) {
        frame.Goto(nest_end);
        frame.final_cycle = true;
      }
      else {
        auto unit = CallMethod(*iterator_obj, "get").GetObj();
        if (frame.error) return;
        obj_stack_.GetCurrent().Replace(unit_id, unit, TryAppendTokenId(unit_id));
      }
    }
  }

  void AASTMachine::CommandCase(ArgumentList &args, size_t nest_end) {
    auto &frame = frame_stack_.top();
    auto &code = code_stack_.back();

    if (args.empty()) {
      frame.MakeError("Empty argument list");
      return;
    }

    frame.AddJumpRecord(nest_end);

    bool has_jump_list = 
      code->FindJumpRecord(frame.idx + frame.jump_offset, frame.branch_jump_stack);

    auto view = FetchObjectView(args[0]);
    if (frame.error) return;

    string type_id = view.Seek().GetTypeId();

    if (!lexical::IsPlainType(type_id)) {
      frame.MakeError("Non-plain object is not supported for now");
      return;
    }

    frame.scope_indicator.push(true);
    obj_stack_.Push(true);
    obj_stack_.CreateObject(kStrCaseObj, view.Seek());
    frame.condition_stack.push(false);

    if (has_jump_list) {
      frame.Goto(frame.branch_jump_stack.top());
      frame.branch_jump_stack.pop();
    }
    else {
      //although I think no one will write case block without condition branch...
      frame.Goto(frame.jump_stack.top());
    }
  }

  void AASTMachine::CommandElse() {
    auto &frame = frame_stack_.top();

    if (frame.condition_stack.empty()) {
      frame.MakeError("Unexpected 'else'");
      return;
    }

    if (frame.condition_stack.top() == true) {
      frame.Goto(frame.jump_stack.top());
    }
    else {
      frame.condition_stack.top() = true;
    }
  }

  void AASTMachine::CommandWhen(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    bool result = false;

    if (frame.condition_stack.empty()) {
      frame.MakeError("Unexpected 'when'");
      return;
    }

    if (frame.condition_stack.top()) {
      frame.Goto(frame.jump_stack.top());
      return;
    }

    if (!args.empty()) {
      ObjectPointer ptr = obj_stack_.Find(kStrCaseObj);
      string type_id = ptr->GetTypeId();
      bool found = false;

      if (ptr == nullptr) {
        frame.MakeError("Unexpected 'when'(2)");
        return;
      }

      if (!lexical::IsPlainType(type_id)) {
        frame.MakeError("Non-plain object is not supported");
        return;
      }

#define COMPARE_RESULT(_Type) (ptr->Cast<_Type>() == obj.Seek().Cast<_Type>())

      for (auto it = args.rbegin(); it != args.rend(); ++it) {
        auto obj = FetchObjectView(*it);
        if (frame.error) return;

        if (obj.Seek().GetTypeId() != type_id) continue;

        if (type_id == kTypeIdInt) {
          found = COMPARE_RESULT(int64_t);
        }
        else if (type_id == kTypeIdFloat) {
          found = COMPARE_RESULT(double);
        }
        else if (type_id == kTypeIdString) {
          found = COMPARE_RESULT(string);
        }
        else if (type_id == kTypeIdBool) {
          found = COMPARE_RESULT(bool);
        }

        if (found) break;
      }
#undef COMPARE_RESULT

      if (found) {
        frame.condition_stack.top() = true;
      }
      else {
        if (!frame.branch_jump_stack.empty()) {
          frame.Goto(frame.branch_jump_stack.top());
          frame.branch_jump_stack.pop();
        }
        else {
          frame.Goto(frame.jump_stack.top());
        }
      }
    }
  }

  void AASTMachine::CommandContinueOrBreak(Operation operation, size_t escape_depth) {
    auto &frame = frame_stack_.top();
    auto &scope_indicator = frame.scope_indicator;

    while (escape_depth != 0) {
      frame.condition_stack.pop();
      frame.jump_stack.pop();
      if (!scope_indicator.empty() && scope_indicator.top()) {
        obj_stack_.Pop();
        
      }
      scope_indicator.pop();
      escape_depth -= 1;
    }

    frame.Goto(frame.jump_stack.top());

    switch (operation) {
    case Operation::Continue:
      frame.from_continue = true; 
      break;
    case Operation::Break:
      frame.from_break = true; 
      frame.final_cycle = true;
      break;
    default:break;
    }
  }

  void AASTMachine::CommandStructBegin(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (args.size() < 1) {
      frame.MakeError("struct identifier is missing");
      return;
    }

    if (args.size() > 2) {
      frame.MakeError("Too many arguments for struct definition");
      return;
    }

    obj_stack_.Push(true);
    auto super_struct_obj = args.size() == 2 ?
      FetchObjectView(args[1]).Seek() : Object();
    auto id_obj = FetchObjectView(args[0]).Seek();

    if (frame.error) return;

    frame.struct_id = id_obj.Cast<string>();

    if (!super_struct_obj.NullPtr()) {
      frame.super_struct_id = super_struct_obj.Cast<string>();
    }

    //NOTICE: frame.struct_id = id_obj.Cast<string>();
    if (auto *ptr = obj_stack_.Find(frame.struct_id, args[0].properties.token_id); ptr != nullptr) {
      frame.MakeError("Struct is existed: " + frame.struct_id);
    }
  }

  void AASTMachine::CommandModuleBegin(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(1)) {
      frame.MakeError("module identifier is missing");
      return;
    }

    obj_stack_.Push(true);
    auto id_obj = FetchObjectView(args[0]).Seek();
    //Use struct_id slot
    frame.struct_id = id_obj.Cast<string>();
  }

  void AASTMachine::CommandConditionEnd() {
    auto &frame = frame_stack_.top();
    if (!frame.keep_condition) {
      frame.condition_stack.pop();
      frame.scope_indicator.pop();
      obj_stack_.Pop();
      
      while (!frame.branch_jump_stack.empty()) frame.branch_jump_stack.pop();
      frame.keep_condition = false;
    }

    frame.jump_stack.pop();
  }

  void AASTMachine::CommandLoopEnd(size_t nest) {
    auto &frame = frame_stack_.top();

    if (frame.final_cycle) {
      if (frame.from_continue) {
        frame.Goto(nest);
        frame.from_continue = false;
        obj_stack_.ClearCurrent();
        //obj_stack_.GetCurrent().Clear();
        frame.jump_from_end = true;
      }
      else {
        if (frame.from_break) frame.from_break = false;
        while (!frame.return_stack.empty()) {
          delete frame.return_stack.back();
          frame.return_stack.pop_back();
        }
        frame.jump_stack.pop();
        obj_stack_.Pop();
        
      }
      frame.scope_indicator.pop();
      frame.final_cycle = false;
    }
    else {
      frame.Goto(nest);
      while (!frame.return_stack.empty()) {
        delete frame.return_stack.back();
        frame.return_stack.pop_back();
      }
      obj_stack_.ClearCurrent();
      //obj_stack_.GetCurrent().Clear();
      frame.jump_from_end = true;
    }
  }

  void AASTMachine::CommandForEachEnd(size_t nest) {
    auto &frame = frame_stack_.top();

    if (frame.final_cycle) {
      if (frame.from_continue) {
        frame.Goto(nest);
        frame.from_continue = false;
        obj_stack_.GetCurrent().ClearExcept(kForEachExceptions);
        frame.jump_from_end = true;
      }
      else {
        if (frame.from_break) frame.from_break = false;
        frame.jump_stack.pop();
        obj_stack_.Pop();
        
      }
      if(!frame.scope_indicator.empty()) frame.scope_indicator.pop();
      frame.final_cycle = false;
    }
    else {
      frame.Goto(nest);
      // container unit will be disposed here, you need to regenerate a new one in header
      obj_stack_.GetCurrent().ClearExcept(kForEachExceptions);
      frame.jump_from_end = true;
    }
  }

  void AASTMachine::CommandStructEnd() {
    auto &frame = frame_stack_.top();
    auto &base = obj_stack_.GetCurrent().GetContent();
    auto managed_struct = make_shared<ObjectStruct>();
    Object *super_struct = nullptr;

    //inheritance implementation
    if (!frame.super_struct_id.empty()) {
      super_struct = obj_stack_.Find(frame.super_struct_id);
      if (super_struct == nullptr) {
        frame.MakeError("Invalid super struct");
        return;
      }
    }

    //copy members from super struct
    if (super_struct != nullptr) {
      auto &super_base = super_struct->Cast<ObjectStruct>();

      for (auto &unit : super_base.GetContent()) {
        if (compare(unit.first, kStrSuperStruct, kStrStructId)) continue;
        if (unit.second.GetTypeId() != kTypeIdFunction) {
          managed_struct->Add(unit.first, components::DumpObject(unit.second));
        }
        else {
          managed_struct->Add(unit.first, unit.second);
        }
      }

      //create reference obejct of super struct
      managed_struct->Add(kStrSuperStruct, Object().PackObject(*super_struct));
    }

    //copy module memebers
    if (auto *module_list_obj = obj_stack_.GetCurrent().Find(kStrModuleList);
      module_list_obj != nullptr) {
      auto &module_list = module_list_obj->Cast<ObjectArray>();
      for (auto it = module_list.begin(); it != module_list.end(); ++it) {
        auto &module = it->Cast<ObjectStruct>().GetContent();
        for (auto &unit : module) {
          if (unit.first == kStrStructId) continue;
          //simple patch
          if (unit.first == kStrInitializer) continue;

          if (unit.second.GetTypeId() != kTypeIdFunction) {
            managed_struct->Add(unit.first, components::DumpObject(unit.second));
          }
          else {
            managed_struct->Add(unit.first, unit.second);
          }
        }
      }
    }

    for (auto &unit : base) {
      managed_struct->Replace(unit.first, unit.second);
    }

    managed_struct->Add(kStrStructId, Object(frame.struct_id));

    obj_stack_.Pop();
    
    obj_stack_.CreateObject(
      frame.struct_id, 
      Object(managed_struct, kTypeIdStruct),
      TryAppendTokenId(frame.struct_id)
    );
    frame.struct_id.clear();
    frame.struct_id.shrink_to_fit();
  }

  void AASTMachine::CommandModuleEnd() {
    auto &frame = frame_stack_.top();
    auto &base = obj_stack_.GetCurrent().GetContent();
    auto managed_module = make_shared<ObjectStruct>();

    for (auto &unit : base) {
      managed_module->Add(unit.first, unit.second);
    }

    managed_module->Add(kStrStructId, Object(frame.struct_id));

    obj_stack_.Pop();
    
    obj_stack_.CreateObject(
      frame.struct_id, 
      Object(managed_module, kTypeIdStruct),
      TryAppendTokenId(frame.struct_id)
    );
    frame.struct_id.clear();
    frame.struct_id.shrink_to_fit();
  }

  void AASTMachine::CommandInclude(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    auto &base = obj_stack_.GetCurrent();
    auto module_view = FetchObjectView(args[0]);

    if (frame.error) return;

    auto &module_obj = FetchObjectView(args[0]).Seek();

    if (args.size() != 1) {
      frame.MakeError("Invalid including declaration");
      return;
    }

    if (auto *mod_list_obj = base.Find(kStrModuleList); mod_list_obj != nullptr) {
      auto &mod_list = mod_list_obj->Cast<ObjectArray>();
      mod_list.push_back(module_obj);
    }
    else {
      auto managed_arr = make_shared<ObjectArray>();
      base.Add(kStrModuleList, Object(managed_arr, kTypeIdArray));
      auto &mod_list = base.Find(kStrModuleList)->Cast<ObjectArray>();
      mod_list.push_back(module_obj);
    }
  }

  void AASTMachine::CommandSuper(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    auto &base = obj_stack_.GetCurrent();

    if (!frame.inside_initializer_calling) {
      frame.MakeError("Invalid super struct intializer calling");
      return;
    }

    if (auto *ptr = base.Find(kStrSuperStruct, false); ptr != nullptr) {
      if (!ptr->IsAlive()) {
        frame.MakeError("Super struct object is dead");
        return;
      }

      auto &super_struct = ptr->Cast<ObjectStruct>();
      auto *initializer = super_struct.Find(kStrInitializer);
      auto *ss_struct = super_struct.Find(kStrSuperStruct);
      auto *instance = base.Find(kStrMe, false);

      ObjectMap obj_map;

      if (initializer == nullptr) {
        frame.MakeError("Super struct doesn't have initalizer");
        return;
      }
      if (!initializer->IsAlive()) {
        frame.MakeError("Initializer object is dead");
        return;
      }

      if (initializer->GetTypeId() != kTypeIdFunction) {
        frame.MakeError("Invalid initializer function");
        return;
      }

      auto &initializer_impl = initializer->Cast<Function>();
      auto &params = initializer_impl.GetParameters();
      size_t pos = args.size() - 1;

      GenerateArgs(initializer_impl, args, obj_map);
      if (frame.error) return;

      if (ss_struct != nullptr) {
        if (!ss_struct->IsAlive()) {
          frame.MakeError("SS Struct is dead");
          return;
        }

        Object ss_struct_ref;
        ss_struct_ref.PackObject(*ss_struct);
        obj_map.emplace(NamedObject(kStrSuperStruct, ss_struct_ref));
      }

      obj_map.emplace(NamedObject(kStrMe, *instance));
      CallVMCFunction(initializer_impl, obj_map);
    }
    else {
      frame.MakeError("This struct doesn't have super struct");
      return;
    }
  }

  void AASTMachine::CommandAttribute(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    bool error = false;

    if (args.size() == 0) {
      frame.MakeError("Expect one argrument at least");
      return;
    }

    for (auto &unit : args) {
      if (unit.GetType() != ArgumentType::Pool) {
        error = true;
        break;
      }

      obj_stack_.CreateObject(unit.GetData(), Object(), unit.properties.token_id);
    }

    if (error) {
      frame.MakeError("Invalid argument for attribute identifier");
    }
  }

  void AASTMachine::CommandSwap(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (args.size() == 2) {
      if (args[0].GetType() == ArgumentType::Literal || args[1].GetType() == ArgumentType::Literal) {
        frame.MakeError("Cannot modify a literal value");
        return;
      }

      auto &right = FetchObjectView(args[1]).Seek();
      auto &left = FetchObjectView(args[0]).Seek();

      if (frame.error) return;

      left.swap(right);
    }
    else if (args.size() == 3) {
      auto &right = FetchObjectView(args[2]).Seek();
      auto &left = FetchObjectView(args[1]).Seek();
      auto &container = FetchObjectView(args[0]).Seek();

      if (frame.error) return;

      if (!compare(right.GetTypeId(), left.GetTypeId(), kTypeIdInt)) {
        frame.MakeError("Invalid index");
        return;
      }

      auto container_type = container.GetTypeId();

      if (container_type == kTypeIdArray) {
        auto &base = container.Cast<ObjectArray>();
        auto rhs_value = size_t(right.Cast<int64_t>());
        auto lhs_value = size_t(left.Cast<int64_t>());

        if (rhs_value >= base.size() || lhs_value >= base.size()) {
          frame.MakeError("Index is out of range");
          return;
        }

        base[lhs_value].swap(base[rhs_value]);
      }
      else if (container_type == kTypeIdTable) {
        auto &base = container.Cast<ObjectTable>();

        auto l_it = base.find(left);
        auto r_it = base.find(right);
        
        if (l_it != base.end() && r_it != base.end()) {
          l_it->second.swap(r_it->second);
        }
        else {
          frame.MakeError("Object is not found in this table");
        }
      }
    }
    else {
      frame.MakeError("Argument missing");
    }

  }

  void AASTMachine::CommandSwapIf(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(3)) {
      frame.MakeError("Argument missing");
      return;
    }

    if (args[0].GetType() == ArgumentType::Literal || args[1].GetType() == ArgumentType::Literal) {
      frame.MakeError("Cannot modify a literal value");
      return;
    }
    
    auto &right = FetchObjectView(args[1]).Seek();
    auto &left = FetchObjectView(args[0]).Seek();

    if (frame.error) return;

    auto &cond = FetchObjectView(args[2]).Seek();
    if (cond.GetTypeId() != kTypeIdBool) {
      frame.MakeError("Invalid condition value");
      return;
    }

    if (cond.Cast<bool>()) left.swap(right);
  }

  void AASTMachine::CommandBind(ArgumentList &args, bool local_value, bool ext_value) {
    auto &frame = frame_stack_.top();

    if (args[0].GetType() == ArgumentType::Literal &&
      lexical::GetStringType(args[0].GetData(), true) != LiteralType::Identifier) {
      frame.MakeError("Cannot modify a literal value");
      return;
    }

    //Do not change the order!
    auto rhs = FetchObjectView(args[1]);
    auto lhs = FetchObjectView(args[0]);

    if (frame.error) return;

    if (lhs.source == ObjectViewSource::Ref) {
      auto &real_lhs = lhs.Seek().Unpack();
      real_lhs = components::DumpObject(rhs.Seek());
      return;
    }
    else {
      string id = lhs.Seek().Cast<string>();

      if (lexical::GetStringType(id) != LiteralType::Identifier) {
        frame.MakeError("Invalid object id");
        return;
      }

      if (!local_value && frame.struct_id.empty()) {
        // pending modification
        ObjectPointer ptr = obj_stack_.Find(id);

        if (ptr != nullptr) {
          if (!ptr->IsAlive()) {
            frame.MakeError("Referenced object is dead: " + id);
            return;
          }

          ptr->Unpack() = components::DumpObject(rhs.Seek());
          return;
        }
      }

      if (!obj_stack_.CreateObject(id, components::DumpObject(rhs.Seek()), args[0].properties.token_id)) {
        frame.MakeError("Object binding is failed");
        return;
      }
    }
  }

  void AASTMachine::CommandDelivering(ArgumentList &args, bool local_value, bool ext_value) {
    auto &frame = frame_stack_.top();

    if (args[0].GetType() == ArgumentType::Literal &&
      lexical::GetStringType(args[0].GetData(), true) != LiteralType::Identifier) {
      frame.MakeError("Cannot modify a literal value");
      return;
    }

    if (args[1].GetType() == ArgumentType::Literal) {
      frame.MakeError("Cannot modify a literal value");
      return;
    }

    //Do not change the order!
    auto rhs = FetchObjectView(args[1]);
    auto lhs = FetchObjectView(args[0]);

    if (frame.error) return;

    if (lhs.source == ObjectViewSource::Ref) {
      auto &real_lhs = lhs.Seek().Unpack();
      real_lhs = rhs.Seek();
      rhs.Seek().Unpack() = Object();
    }
    else {
      string id = lhs.Seek().Cast<string>();

      if (lexical::GetStringType(id) != LiteralType::Identifier) {
        frame.MakeError("Invalid object id");
        return;
      }

      if (!local_value && frame.struct_id.empty()) {
        ObjectPointer ptr = obj_stack_.Find(id);

        if (ptr != nullptr) {
          if (!ptr->IsAlive()) {
            frame.MakeError("Referenced object is dead: " + id);
            return;
          }

          ptr->Unpack() = rhs.Seek();
          rhs.Seek() = Object();
          return;
        }
      }

      rhs.Seek().Unpack() = Object();

      if (!obj_stack_.CreateObject(id, rhs.Seek().Unpack(), args[0].properties.token_id)) {
        frame.MakeError("Object delivering is failed");
        return;
      }
    }
  }

  void AASTMachine::CommandTypeId(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (args.size() > 1) {
      ManagedArray base = make_shared<ObjectArray>();

      for (auto &unit : args) {
        base->emplace_back(Object(FetchObjectView(unit).Seek().GetTypeId()));
      }
      if (frame.error) return;

      Object obj(base, kTypeIdArray);
      frame.RefreshReturnStack(obj);
    }
    else if (args.size() == 1) {
      frame.RefreshReturnStack(Object(FetchObjectView(args[0]).Seek().GetTypeId()));
    }
    else {
      frame.RefreshReturnStack(Object(kTypeIdNull));
    }
  }

  void AASTMachine::CommandMethods(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(1)) {
      frame.MakeError("Argument mismatching: methods(obj)");
      return;
    }

    
    Object &obj = FetchObjectView(args[0]).Seek();
    if (frame.error) return;

    vector<string> methods;
    GetObjectMethods(obj, methods);
    ManagedArray base = make_shared<ObjectArray>();

    for (auto &unit : methods) {
      base->emplace_back(Object(unit, kTypeIdString));
    }

    if (obj.IsSubContainer()) {
      auto &container = obj.Cast<ObjectStruct>().GetContent();
      for (auto &unit : container) {
        if (unit.second.GetTypeId() == kTypeIdFunction) {
          base->emplace_back(unit.first);
        }
      }
    }

    Object ret_obj(base, kTypeIdArray);
    frame.RefreshReturnStack(ret_obj);
  }

  void AASTMachine::CommandExist(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(2)) {
      frame.MakeError("Argument mismatching: exist(obj, id)");
      return;
    }

    //Do not change the order
    auto &str_obj = FetchObjectView(args[1]).Seek();
    auto &obj = FetchObjectView(args[0]).Seek();
    if (frame.error) return;

    if (str_obj.GetTypeId() != kTypeIdString) {
      frame.MakeError("Invalid method id");
      return;
    }

    string str = str_obj.Cast<string>();
    bool first_stage = CheckObjectMethod(obj, str);
    bool second_stage = obj.IsSubContainer() ?
      [&]() -> bool {
      auto &container = obj.Cast<ObjectStruct>().GetContent();
      for (auto &unit : container) {
        if (unit.first == str) return true;
      }
      return false;
    }() : false;


    Object ret_obj(first_stage || second_stage, kTypeIdBool);

    frame.RefreshReturnStack(ret_obj);
  }

  void AASTMachine::CommandNullObj(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(1)) {
      frame.MakeError("Argument mismatching: null_obj(obj)");
      return;
    }

    Object &obj = FetchObjectView(args[0]).Seek();
    if (frame.error) return;

    frame.RefreshReturnStack(Object(obj.GetTypeId() == kTypeIdNull, kTypeIdBool));
  }

  void AASTMachine::CommandToString(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(1)) {
      frame.MakeError("Argument mismatching: convert(obj)");
      return;
    }

    Argument &arg = args[0];
    if (arg.GetType() == ArgumentType::Literal) {
      frame.RefreshReturnStack(*FetchLiteralObject(arg));
    }
    else {
      Object &obj = FetchObjectView(args[0]).Seek();
      if (frame.error) return;

      string type_id = obj.GetTypeId();
      Object ret_obj;

      if (type_id == kTypeIdString) {
        auto str = obj.Cast<string>();
        auto type = lexical::GetStringType(str, true);

        switch (type) {
        case LiteralType::Int:
          ret_obj.PackContent(make_shared<int64_t>(stol(str)), kTypeIdInt);
          break;
        case LiteralType::Float:
          ret_obj.PackContent(make_shared<double>(stod(str)), kTypeIdFloat);
          break;
        case LiteralType::Bool:
          ret_obj.PackContent(make_shared<bool>(str == kStrTrue), kTypeIdBool);
          break;
        default:
          ret_obj = obj;
          break;
        }
      }
      else {
        if (!CheckObjectMethod(obj, kStrToString)) {
          frame.MakeError("Invalid argument for convert()");
          return;
        }

        ret_obj = CallMethod(obj, kStrToString).GetObj();
        if (frame.error) return;
      }

      frame.RefreshReturnStack(ret_obj);
    }
  }

  void AASTMachine::CommandUsing(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(1)) {
      frame.MakeError("Argument mismatching: load(obj)");
      return;
    }

    auto &path_obj = FetchObjectView(args[0]).Seek();
    if (frame.error) return;

    if (path_obj.GetTypeId() != kTypeIdString) {
      frame.MakeError("Invalid path");
      return;
    }

    //Smarter directory strategy?

    string path = path_obj.Cast<string>();
    fs::path wrapped_path(path);
    string extension_name = lexical::ToLower(wrapped_path.extension().string());

    if (extension_name == ".sp" || extension_name.empty()) {
      string absolute_path = fs::absolute(fs::path(path)).string();
      if (extension_name.empty()) absolute_path.append(".sp");

      if (!fs::exists(fs::path(absolute_path))) {
        fs::path wrapped_abs_path(runtime::GetBinaryPath() + "/lib");
        absolute_path = fs::absolute(wrapped_abs_path).string();
        absolute_path.append("/" + wrapped_path.filename().string());
        if (extension_name.empty()) absolute_path.append(".sp");
      }

      AnnotatedAST &script_file = script::AppendBlankScript(absolute_path);

      //already loaded
      if (!script_file.empty()) return;

      GrammarAndSemanticAnalysis factory(absolute_path, script_file, logger_);

      if (factory.Start()) {
        AASTMachine sub_machine(script_file, logger_);
        auto &obj_base = obj_stack_.GetBase();
        sub_machine.SetDelegatedRoot(obj_base.front());
        sub_machine.Run();

        if (sub_machine.ErrorOccurred()) {
          frame.MakeError("Error is occurred in loaded script");
        }
      }
      else {
        frame.MakeError("Invalid script - " + path);
      }
    }
    else {
      frame.MakeError("Unknown file type");
    }
  }

  void AASTMachine::CommandPrint(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    auto view = FetchObjectView(args[0]);
    if (frame.error) return;

    auto type_id = view.Seek().GetTypeId();
    auto &obj = view.Seek();

    if (lexical::IsPlainType(type_id)) {
      if (type_id == kTypeIdInt) {
#ifndef _MSC_VER
        fprintf(VM_STDOUT, "%ld", obj.Cast<int64_t>());
#else
        fprintf(VM_STDOUT, "%lld", obj.Cast<int64_t>());
#endif
      }
      else if (type_id == kTypeIdFloat) {
        fprintf(VM_STDOUT, "%f", obj.Cast<double>());
      }
      else if (type_id == kTypeIdString) {
        fputs(obj.Cast<string>().data(), VM_STDOUT);
      }
      else if (type_id == kTypeIdBool) {
        fputs(obj.Cast<bool>() ? "true" : "false", VM_STDOUT);
      }
    }
    else {
      //Try to invoke 'print' method
      if (!CheckObjectMethod(obj, kStrPrintDo)) {
        string msg("<Object Type=" + obj.GetTypeId() + string(">"));
        fputs(msg.data(), VM_STDOUT);
      }
      else {
        CallMethod(obj, kStrPrintDo);
      }
    }
  }

  void AASTMachine::CommandInput(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    if (!args.empty()) {
      frame.MakeError("Invalid arguments: input()");
      return;
    }

    string buf = GetLine();
    frame.RefreshReturnStack(Object(buf, kTypeIdString));
  }

  void AASTMachine::CommandGetChar(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    if (!args.empty()) {
      frame.MakeError("Invalid arguments: getchar()");
      return;
    }

    auto value = static_cast<char>(fgetc(VM_STDIN));
    frame.RefreshReturnStack(Object(string().append(1, value), kTypeIdString));
  }

  void AASTMachine::SysCommand(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    if (args.size() != 1) {
      frame.MakeError("Invalid argument: console(command)");
      return;
    }

    auto view = FetchObjectView(args[0]);
    if (frame.error) return;

    if (view.Seek().GetTypeId() != kTypeIdString) {
      frame.MakeError("Invalid command string");
      return;
    }

    int64_t result = system(view.Seek().Cast<string>().data());
    frame.RefreshReturnStack(Object(result, kTypeIdInt));
  }

  void AASTMachine::CommandSleep(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (args.size() != 1) {
      frame.MakeError("Invalid argument: sleep(ms)");
      return;
    }

    auto view = FetchObjectView(args[0]);
    if (frame.error) return;
    if (view.Seek().GetTypeId() != kTypeIdInt) {
      frame.MakeError("Invalid sleep duration");
      return;
    }
   
    auto value = view.Seek().Cast<int64_t>();
    
#ifdef _MSC_VER
    Sleep(static_cast<DWORD>(value));
#else
    timespec spec;

    if (value >= 1000) {
      spec.tv_sec = value / 1000;
      spec.tv_nsec = (value - (static_cast<int64_t>(spec.tv_sec) * 1000))
        * 1000000;
    }
    else {
      spec.tv_sec = 0;
      spec.tv_nsec = value * 1000000;
    }

    nanosleep(&spec, nullptr);
#endif
  }

  void AASTMachine::CommandTime() {
    auto &frame = frame_stack_.top();
    time_t now = time(nullptr);
    string nowtime(ctime(&now));
    nowtime.pop_back();
    frame.RefreshReturnStack(Object(nowtime));
  }

  void AASTMachine::CommandVersion() {
    auto &frame = frame_stack_.top();
    frame.RefreshReturnStack(Object(BUILD));
  }

  void AASTMachine::CommandMachineCodeName() {
    auto &frame = frame_stack_.top();
    frame.RefreshReturnStack(Object(CODENAME));
  }

  template <Operation op_code>
  void AASTMachine::BinaryMathOperatorImpl(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(2)) {
      frame.MakeError("Argument behind operator is missing");
      return;
    }

    auto rhs = FetchObjectView(args[1]);
    auto lhs = FetchObjectView(args[0]);
    if (frame.error) return;

    auto type_rhs = FindTypeCode(rhs.Seek().GetTypeId());
    auto type_lhs = FindTypeCode(lhs.Seek().GetTypeId());

    if (frame.error) return;

    if (type_lhs == PlainType::Invalid || type_rhs  == PlainType::Invalid) {
      frame.MakeError("Try to operate with non-plain type.");
      return;
    }

    auto result_type = kResultDynamicTraits.at(ResultTraitKey(type_lhs, type_rhs));

#define RESULT_PROCESSING(_Type, _Func, _TypeId)                                     \
  _Type result = MathBox<_Type, op_code>().Do(_Func(lhs.Seek()), _Func(rhs.Seek())); \
  frame.RefreshReturnStack(Object(result, _TypeId));

    if (result_type == PlainType::String) {
      if (IsIllegalStringOperator(op_code)) {
        frame.RefreshReturnStack(Object());
        return;
      }

      RESULT_PROCESSING(string, StringProducer, kTypeIdString);
    }
    else if (result_type == PlainType::Int) {
      RESULT_PROCESSING(int64_t, IntProducer, kTypeIdInt);
    }
    else if (result_type == PlainType::Float) {
      RESULT_PROCESSING(double, FloatProducer, kTypeIdFloat);
    }
    else if (result_type == PlainType::Bool) {
      RESULT_PROCESSING(bool, BoolProducer, kTypeIdBool);
    }
#undef RESULT_PROCESSING
  }

  template <Operation op_code>
  void AASTMachine::BinaryLogicOperatorImpl(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(2)) {
      frame.MakeError("Argument behind operator is missing");
      return;
    }

    auto rhs = FetchObjectView(args[1]);
    auto lhs = FetchObjectView(args[0]);
    if (frame.error) return;

    auto type_rhs = FindTypeCode(rhs.Seek().GetTypeId());
    auto type_lhs = FindTypeCode(lhs.Seek().GetTypeId());
    bool result = false;

    if (frame.error) return;

    if (!lexical::IsPlainType(lhs.Seek().GetTypeId())) {
      if constexpr (op_code != Operation::Equals && op_code != Operation::NotEqual) {
        frame.RefreshReturnStack(Object());
      }
      else {
        if (!CheckObjectMethod(lhs.Seek(), kStrCompare)) {
          frame.MakeError("Can't operate with this operator");
          return;
        }

        //TODO:Test these code(for user-defined function)
        Object obj = CallMethod(lhs.Seek(), kStrCompare,
          { NamedObject(kStrRightHandSide, rhs.Dump()) }).GetObj();
        if (frame.error) return;

        if (obj.GetTypeId() != kTypeIdBool) {
          frame.MakeError("Invalid behavior of compare()");
          return;
        }

        bool value = obj.Cast<bool>();

        frame.RefreshReturnStack(op_code == Operation::NotEqual ? !value : value);
      }

      return;
    }

    auto result_type = kResultDynamicTraits.at(ResultTraitKey(type_lhs, type_rhs));
#define RESULT_PROCESSING(_Type, _Func) \
  result = LogicBox<_Type, op_code>().Do(_Func(lhs.Seek()), _Func(rhs.Seek()));

    if (result_type == PlainType::String) {
      if (IsIllegalStringOperator(op_code)) {
        frame.RefreshReturnStack(Object());
        return;
      }

      RESULT_PROCESSING(string, StringProducer);
    }
    else if (result_type == PlainType::Int) {
      RESULT_PROCESSING(int64_t, IntProducer);
    }
    else if (result_type == PlainType::Float) {
      RESULT_PROCESSING(double, FloatProducer);
    }
    else if (result_type == PlainType::Bool) {
      RESULT_PROCESSING(bool, BoolProducer);
    }


    frame.RefreshReturnStack(result);
#undef RESULT_PROCESSING
  }

  void AASTMachine::OperatorLogicNot(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(1)) {
      frame.MakeError("Argument behind operator is missing");
      return;
    }

    //TODO:ObjectView
    auto &rhs = FetchObjectView(args[0]).Seek();
    if (frame.error) return;

    if (rhs.GetTypeId() != kTypeIdBool) {
      frame.MakeError("Can't operate with this operator");
      return;
    }

    bool result = !rhs.Cast<bool>();


    frame.RefreshReturnStack(result);
  }

  void AASTMachine::OperatorIncreasing(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(2)) {
      frame.MakeError("Argument behind operator is missing");
      return;
    }

    auto rhs = FetchObjectView(args[1]);
    auto lhs = FetchObjectView(args[0]);

    if (frame.error) return;

    if (!compare(lhs.Seek().GetTypeId(), rhs.Seek().GetTypeId(), kTypeIdInt)) {
      frame.MakeError("Unsupported type");
      return;
    }

    auto &value = lhs.Seek().Cast<int64_t>();
    value += rhs.Seek().Cast<int64_t>();
  }

  void AASTMachine::OperatorDecreasing(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(2)) {
      frame.MakeError("Argument behind operator is missing");
      return;
    }

    auto rhs = FetchObjectView(args[1]);
    auto lhs = FetchObjectView(args[0]);

    if (frame.error) return;

    if (!compare(lhs.Seek().GetTypeId(), rhs.Seek().GetTypeId(), kTypeIdInt)) {
      frame.MakeError("Unsupported type");
      return;
    }

    auto &value = lhs.Seek().Cast<int64_t>();
    value -= rhs.Seek().Cast<int64_t>();
  }

  //TODO: Replace with multiple new commands
  void AASTMachine::ExpList(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    if (!args.empty()) {
      auto result_view = FetchObjectView(args.back());
     frame.RefreshReturnStack(result_view.Seek());
    }
  }

  void AASTMachine::InitArray(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    ManagedArray base = make_shared<ObjectArray>();

    if (!args.empty()) {
      for (auto &unit : args) {
        base->emplace_back(FetchObjectView(unit).Seek());
      }
    }
    if (frame.error) return;

    Object obj(base, kTypeIdArray);
    frame.RefreshReturnStack(obj);
  }

  void AASTMachine::CommandReturn(ArgumentList &args) {
    if (frame_stack_.size() == 1) {
      frame_stack_.top().MakeError("Unexpected return");
      return;
    }

    ObjectPointer constraint_type_obj = obj_stack_.GetCurrent().Find(kStrReturnValueConstrantObj);
    string constraint_type = constraint_type_obj == nullptr ?
      "" : constraint_type_obj->Cast<string>();

    if (args.size() == 1) {
      //keep alive
      auto ret_objview = FetchObjectView(args[0]);
      if (frame_stack_.top().error) return;
      auto ret_keepalive = ret_objview.Seek();

      if (!constraint_type.empty() && ret_objview.Seek().GetTypeId() != constraint_type) {
        frame_stack_.top().MakeError("Mismatched return value type: " + constraint_type);
        return;
      }

      if (frame_stack_.top().error) return;

      auto *container = &obj_stack_.GetCurrent();
      while (container->Find(kStrUserFunc) == nullptr) {
        obj_stack_.Pop();
        
        container = &obj_stack_.GetCurrent();
      }

      RecoverLastState(true);
      frame_stack_.top().RefreshReturnStack(ret_keepalive);
     
    }
    else if (args.size() == 0) {
      if (!constraint_type.empty() && constraint_type != kTypeIdNull) {
        frame_stack_.top().MakeError("Mismatched return value type: null");
        return;
      }

      auto *container = &obj_stack_.GetCurrent();
      while (container->Find(kStrUserFunc) == nullptr) {
        obj_stack_.Pop();
        
        container = &obj_stack_.GetCurrent();
      }

      RecoverLastState(true);
      frame_stack_.top().RefreshReturnStack(Object());
    }
    else {
      if (!constraint_type.empty() && constraint_type != kTypeIdArray) {
        frame_stack_.top().MakeError("Mismatched return value type: array");
        return;
      }

      ManagedArray obj_array = make_shared<ObjectArray>();
      for (auto it = args.begin(); it != args.end(); ++it) {
        auto obj_view = FetchObjectView(*it);
        if (frame_stack_.top().error) return;

        obj_array->emplace_back(obj_view.Seek().Unpack());
      }

      Object ret_obj(obj_array, kTypeIdArray);

      auto *container = &obj_stack_.GetCurrent();
      while (container->Find(kStrUserFunc) == nullptr) {
        obj_stack_.Pop();
        
        container = &obj_stack_.GetCurrent();
      }

      RecoverLastState(true);
      frame_stack_.top().RefreshReturnStack(ret_obj);
    }
  }

  void AASTMachine::CommandAssert(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(1)) {
      frame.MakeError("Argument mismatching: assert(bool_obj)");
      return;
    }

    auto &result_obj = FetchObjectView(args[0]).Seek();
    if (frame.error) return;

    if (result_obj.GetTypeId() != kTypeIdBool) {
      frame.MakeError("Invalid object type for assertion.");
      return;
    }

    if (!result_obj.Cast<bool>()) {
      frame.MakeError("User assertion failed.");
    }
  }

  void AASTMachine::DomainAssert(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    frame.assert_rc_copy = FetchObjectView(args[0]).Seek().Unpack();
  }

  void AASTMachine::CommandIsBaseOf(ArgumentList &args) {
    auto &frame = frame_stack_.top(); 

    if (!EXPECTED_COUNT(2)) {
      frame.MakeError("Argument mismatching: is_base_of(dest_obj, base_obj)");
      return;
    }

    auto &base_obj = FetchObjectView(args[1]).Seek();
    auto &dest_obj = FetchObjectView(args[0]).Seek();
    if (frame.error) return;

    if (!compare(kTypeIdStruct, dest_obj.GetTypeId(), base_obj.GetTypeId())) {
      frame.MakeError("Invalid argument type(Required type is struct)");
      return;
    }

    auto base_ptr = base_obj.Get();
    auto &dest_struct = dest_obj.Cast<ObjectStruct>();
    auto *super_struct_ref = dest_struct.Find(kStrSuperStruct);

    if (super_struct_ref == nullptr) {
      frame.RefreshReturnStack(Object(false, kTypeIdBool));
      return;
    }

    if (!super_struct_ref->IsAlive()) {
      frame.MakeError("Super struct object is dead");
      return;
    }

    auto dest_ptr = super_struct_ref->Get();

    frame.RefreshReturnStack(Object(dest_ptr == base_ptr, kTypeIdBool));
  }

  void AASTMachine::CommandHasBehavior(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    auto &behavior_obj = FetchObjectView(args[1]).Seek();
    auto &obj = FetchObjectView(args[0]).Seek();

    if (frame.error) return;

    auto result = CheckObjectBehavior(obj, behavior_obj.Cast<string>());

    frame.RefreshReturnStack(Object(result, kTypeIdBool));
  }

  template <ParameterPattern pattern>
  void AASTMachine::CommandCheckParameterPattern(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(1)) {
      if constexpr (pattern == ParameterPattern::Variable) {
        frame.MakeError("Argument mismatching: is_variable_param(func)");
      }

      return;
    }

    auto &func_obj = FetchObjectView(args[0]).Seek();
    if (frame.error) return;

    if (func_obj.GetTypeId() != kTypeIdFunction) {
      frame.MakeError("Expected object type is function");
      return;
    }

    auto &impl = func_obj.Cast<Function>();

    Object result(impl.GetPattern() == pattern, kTypeIdBool);
    frame.RefreshReturnStack(result);
  }

  void AASTMachine::CommandOptionalParamRange(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(1)) {
      frame.MakeError("Argument mismatching: optional_param_range(obj)");
    }

    auto &func_obj = FetchObjectView(args[0]).Seek();
    if (frame.error) return;

    if (func_obj.GetTypeId() != kTypeIdFunction) {
      frame.MakeError("Expected object type is function");
      return;
    }

    auto &impl = func_obj.Cast<Function>();
    auto size = impl.GetParamSize();
    auto limit = impl.GetLimit();
    Object result(static_cast<int64_t>(size - limit), kTypeIdInt);
    frame.RefreshReturnStack(result);
  }

  void AASTMachine::MachineCommands(Operation operation, ArgumentList &args, ASTNode &node) {
    auto &frame = frame_stack_.top();

    switch (operation) {
    case Operation::Load:
      CommandLoad(args);
      break;
    case Operation::If:
    case Operation::Elif:
    case Operation::While:
      CommandIfOrWhile(operation, args, node.annotation.nest_end);
      break;
    case Operation::Plus:
      BinaryMathOperatorImpl<Operation::Plus>(args);
      break;
    case Operation::Minus:
      BinaryMathOperatorImpl<Operation::Minus>(args);
      break;
    case Operation::Times:
      BinaryMathOperatorImpl<Operation::Times>(args);
      break;
    case Operation::Divide:
      BinaryMathOperatorImpl<Operation::Divide>(args);
      break;
    case Operation::Equals:
      BinaryLogicOperatorImpl<Operation::Equals>(args);
      break;
    case Operation::LessOrEqual:
      BinaryLogicOperatorImpl<Operation::LessOrEqual>(args);
      break;
    case Operation::GreaterOrEqual:
      BinaryLogicOperatorImpl<Operation::GreaterOrEqual>(args);
      break;
    case Operation::NotEqual:
      BinaryLogicOperatorImpl<Operation::NotEqual>(args);
      break;
    case Operation::Greater:
      BinaryLogicOperatorImpl<Operation::Greater>(args);
      break;
    case Operation::Less:
      BinaryLogicOperatorImpl<Operation::Less>(args);
      break;
    case Operation::And:
      BinaryLogicOperatorImpl<Operation::And>(args);
      break;
    case Operation::Or:
      BinaryLogicOperatorImpl<Operation::Or>(args);
      break;
    case Operation::Increase:
      OperatorIncreasing(args);
      break;
    case Operation::Decrease:
      OperatorDecreasing(args);
      break;
    case Operation::Not:
      OperatorLogicNot(args);
      break;
    case Operation::For:
      if (frame.jump_from_end) {
        CheckForEach(args, node.annotation.nest_end);
        frame.jump_from_end = false;
      }
      else {
        InitForEach(args, node.annotation.nest_end);
      }
      //CommandForEach(args, node.option.nest_end);
      break;
    case Operation::NullObj:
      CommandNullObj(args);
      break;
    case Operation::ToString:
      CommandToString(args);
      break;
    case Operation::Time:
      CommandTime();
      break;
    case Operation::Version:
      CommandVersion();
      break;
    case Operation::CodeName:
      CommandMachineCodeName();
      break;
    case Operation::Swap:
      CommandSwap(args);
      break;
    case Operation::SwapIf:
      CommandSwapIf(args);
      break;
    case Operation::Bind:
      CommandBind(args, node.annotation.local_object, node.annotation.ext_object);
      break;
    case Operation::Delivering:
      CommandDelivering(args, node.annotation.local_object, node.annotation.ext_object);
      break;
    case Operation::ExpList:
      ExpList(args);
      break;
    case Operation::InitialArray:
      InitArray(args);
      break;
    case Operation::Return:
      CommandReturn(args);
      break;
    case Operation::Assert:
      CommandAssert(args);
      break;
    case Operation::TypeId:
      CommandTypeId(args);
      break;
    case Operation::Methods:
      CommandMethods(args);
      break;
    case Operation::Exist:
      CommandExist(args);
      break;
    case Operation::Fn:
      ClosureCatching(args, node.annotation.nest_end, frame_stack_.size() > 1);
      break;
    case Operation::Case:
      CommandCase(args, node.annotation.nest_end);
      break;
    case Operation::When:
      CommandWhen(args);
      break;
    case Operation::End:
      switch (node.annotation.nest_root) {
      case Operation::While:
        CommandLoopEnd(node.annotation.nest);
        break;
      case Operation::For:
        CommandForEachEnd(node.annotation.nest);
        break;
      case Operation::If:
      case Operation::Case:
        CommandConditionEnd();
        break;
      case Operation::Struct:
        CommandStructEnd();
        break;
      case Operation::Module:
        CommandModuleEnd();
        break;
      default:break;
      }
      break;
    case Operation::Continue:
    case Operation::Break:
      CommandContinueOrBreak(operation, node.annotation.escape_depth);
      break;
    case Operation::Else:
      CommandElse();
      break;
    case Operation::Using:
      CommandUsing(args);
      break;
    case Operation::Struct:
      CommandStructBegin(args);
      break;
    case Operation::Module:
      CommandModuleBegin(args);
      break;
    case Operation::DomainAssertCommand:
      DomainAssert(args);
      break;
    case Operation::Include:
      CommandInclude(args);
      break;
    case Operation::Super:
      CommandSuper(args);
      break;
    case Operation::Attribute:
      CommandAttribute(args);
      break;
    case Operation::IsBaseOf:
      CommandIsBaseOf(args);
      break;
    case Operation::HasBehavior:
      CommandHasBehavior(args);
      break;
    case Operation::IsVariableParam:
      CommandCheckParameterPattern<ParameterPattern::Variable>(args);
      break;
    case Operation::OptionalParamRange:
      CommandOptionalParamRange(args);
      break;
    case Operation::Print:
      CommandPrint(args);
      break;
    case Operation::PrintLine:
      CommandPrint(args);
      fputs("\n", VM_STDOUT);
      break;
    case Operation::Input:
      CommandInput(args);
      break;
    case Operation::Conole:
      SysCommand(args);
      break;
    case Operation::GetChar:
      CommandGetChar(args);
      break;
    default:
      break;
    }
  }

  //deprecated
  void AASTMachine::GenerateArgs(Function &impl, ArgumentList &args, ObjectMap &obj_map) {
    switch (impl.GetPattern()) {
    case ParameterPattern::Fixed:
      Generate_Fixed(impl, args, obj_map);
      break;
    case ParameterPattern::Variable:
      Generate_Variable(impl, args, obj_map);
      break;
    default:
      break;
    }
  }

  void AASTMachine::Generate_Fixed(Function &impl, ArgumentList &args, ObjectMap &obj_map) {
    auto &frame = frame_stack_.top();
    auto &params = impl.GetParameters();
    size_t pos = args.size() - 1;
    ObjectView view;

    if (args.size() > params.size()) {
      frame.MakeError("Too many arguments");
      return;
    }

    if (args.size() < params.size()) {
      frame.MakeError("Minimum argument amount is " + to_string(params.size()));
      return;
    }


    for (auto it = params.rbegin(); it != params.rend(); ++it) {
      view = FetchObjectView(args[pos]);
      view.Seek().RemoveDeliveringFlag();
      obj_map.emplace(NamedObject(*it, view.Seek()));
      pos -= 1;
    }
  }

  void AASTMachine::Generate_Variable(Function &impl, ArgumentList &args, ObjectMap &obj_map) {
    auto &frame = frame_stack_.top();
    vector<string> &params = impl.GetParameters();
    list<Object> temp_list;
    ManagedArray va_base = make_shared<ObjectArray>();
    size_t pos = args.size(), diff = args.size() - params.size() + 1;

    if (args.size() < params.size()) {
      frame.MakeError("Minimum argument amount is " + to_string(params.size()));
      return;
    }

    while (diff != 0) {
      temp_list.emplace_front(FetchObjectView(args[pos - 1]).Dump().RemoveDeliveringFlag());
      pos -= 1;
      diff -= 1;
    }

    if (!temp_list.empty()) {
      for (auto it = temp_list.begin(); it != temp_list.end(); ++it) {
        va_base->emplace_back(*it);
      }

      temp_list.clear();
    }

    obj_map.insert(NamedObject(params.back(), Object(va_base, kTypeIdArray)));


    while (pos > 0) {
      obj_map.emplace(params[pos - 1], FetchObjectView(args[pos - 1]).Dump().RemoveDeliveringFlag());
      pos -= 1;
    }
  }

  void AASTMachine::CallExtensionFunction(ObjectMap &p, Function &impl) {
    auto &frame = frame_stack_.top();
    Object returning_slot;
    auto ext_activity = impl.Get<ExtensionActivity>();
    ExternalState vm_state{ &p, &returning_slot, this, ReceiveExtReturningValue };
    auto result_code = ext_activity(vm_state);
    if (result_code < 1) {
      frame.MakeError("Extension reports error while invoking external activity.");
      return;
    }
    frame.RefreshReturnStack(returning_slot);
  }

  void AASTMachine::GenerateStructInstance(ObjectMap &p) {
    auto &frame = frame_stack_.top();

    auto &base = frame.struct_base.Cast<ObjectStruct>().GetContent();
    auto managed_instance = make_shared<ObjectStruct>();
    auto struct_id = base.at(kStrStructId).Cast<string>();
    auto super_struct = [&]() -> Object {
      auto it = base.find(kStrSuperStruct);
      if (it == base.end()) return Object();
      return it->second;
    }();

    for (auto &unit : base) {
      if (compare(unit.first, kStrInitializer, kStrStructId, kStrSuperStruct)) {
        continue;
      }

      managed_instance->Add(unit.first, components::DumpObject(unit.second));
    }

    if (!super_struct.NullPtr()) {
      p.insert(NamedObject(kStrSuperStruct, super_struct));
    }

    Object instance_obj(managed_instance, struct_id);
    instance_obj.SetContainerFlag();
    p.insert(NamedObject(kStrMe, instance_obj));
    frame.struct_base = Object();
  }

  void AASTMachine::GenerateErrorMessages(size_t stop_index) {
    //Under consideration
    if (frame_stack_.top().error) {
      //TODO:reporting function calling chain
      AppendMessage(frame_stack_.top().msg_string, StateLevel::Error,
        logger_, stop_index);
    }

    frame_stack_.pop();

    while (!frame_stack_.empty()) {
      if (frame_stack_.top().stop_point) break;
      frame_stack_.pop();
    }
  }

  //for extension callback facilities
  bool AASTMachine::PushObject(string id, Object object) {
    auto &frame = frame_stack_.top();
    auto result = obj_stack_.CreateObject(id, object, TryAppendTokenId(id));
    if (!result) {
      frame.MakeError("Cannot create object");
      return false;
    }
    return true;
  }

  void AASTMachine::PushError(string msg) {
    auto &frame = frame_stack_.top();
    frame.MakeError(msg);
  }

  void AASTMachine::CopyComponents() {
    auto &base = obj_stack_.GetCurrent();
    auto &comp_base = components::GetBuiltinComponentsObjBase();
    
    for (auto &unit : comp_base) {
      base.Add(unit.first, unit.second, TryAppendTokenId(unit.first));
    }
  }

  void AASTMachine::Run(bool invoke) {
    if (code_stack_.empty()) return;

    bool next_tick;
    size_t script_idx = 0;
    Message msg;
    AnnotatedAST *code = code_stack_.back();
    Sentense *sentense = nullptr;
    ObjectMap obj_map;
    FunctionImplPointer impl;

    if (!invoke) {
      frame_stack_.push(RuntimeFrame());
      obj_stack_.Push();
      if (!delegated_base_scope_) CopyComponents();
    }

    RuntimeFrame *frame = &frame_stack_.top();
    size_t size = code->size();

    obj_map.reserve(10);

    //Refreshing loop tick state to make it work correctly.
    auto refresh_tick = [&]() -> void {
      code = code_stack_.back();
      size = code->size();
      frame = &frame_stack_.top();
    };
    //Protect current runtime environment and load another function
    auto update_stack_frame = [&](Function &func) -> void {
      bool inside_initializer_calling = frame->do_initializer_calling;
      frame->do_initializer_calling = false;
      code_stack_.push_back(&func.Get<AnnotatedAST>());
      frame_stack_.push(RuntimeFrame(func.GetId()));
      obj_stack_.Push();
      obj_stack_.CreateObject(kStrUserFunc, Object(func.GetId()));
      obj_stack_.MergeMap(obj_map);
      obj_stack_.MergeMap(impl->GetClosureRecord());
      refresh_tick();
      frame->jump_offset = func.GetOffset();
      frame->inside_initializer_calling = inside_initializer_calling;
    };
    //Convert current environment to next self-calling 
    auto tail_recursion = [&]() -> void {
      string function_scope = frame_stack_.top().function_scope;
      size_t jump_offset = frame_stack_.top().jump_offset;
      obj_map.Naturalize(obj_stack_.GetCurrent());
      frame_stack_.top() = RuntimeFrame(function_scope);
      obj_stack_.ClearCurrent();
      obj_stack_.CreateObject(kStrUserFunc, Object(function_scope));
      obj_stack_.MergeMap(obj_map);
      obj_stack_.MergeMap(impl->GetClosureRecord());
      refresh_tick();
      frame->jump_offset = jump_offset;
    };
    //Convert current environment to next calling
    auto tail_call = [&](Function &func) -> void {
      code_stack_.pop_back();
      code_stack_.push_back(&func.Get<AnnotatedAST>());
      obj_map.Naturalize(obj_stack_.GetCurrent());
      //auto inside_initiailizer_calling = frame_stack_.top().do_initializer_calling;
      frame_stack_.top() = RuntimeFrame(func.GetId());
      //frame_stack_.top().inside_initializer_calling = inside_initiailizer_calling;
      obj_stack_.ClearCurrent();
      obj_stack_.CreateObject(kStrUserFunc, Object(func.GetId()));
      obj_stack_.MergeMap(obj_map);
      obj_stack_.MergeMap(impl->GetClosureRecord());
      refresh_tick();
      frame->jump_offset = func.GetOffset();
    };

    auto load_function_impl = [&](bool invoking_request) -> bool {
      bool switch_to_next_tick = false;
      switch (impl->GetType()) {
      case FunctionType::Operation:
        //start new processing in next tick.
        if (invoking_request) goto direct_load_vmcode;
        if (IsTailRecursion(frame->idx, &impl->Get<AnnotatedAST>())) tail_recursion();
        else if (IsTailCall(frame->idx) && !frame->do_initializer_calling) tail_call(*impl);
        else {
        direct_load_vmcode:
          update_stack_frame(*impl);
        }
        switch_to_next_tick = true;
        break;
      case FunctionType::External:
        if (invoking_request) {
          frame->MakeError("Unsupported feature");
          break;
        }
        CallExtensionFunction(obj_map, *impl);
        if (!frame->error) {
          frame->Stepping();
          switch_to_next_tick = true;
        }
        break;
      case FunctionType::Component:
        msg = impl->Get<Activity>()(obj_map);
        if (msg.GetLevel() == StateLevel::Error) {
          frame->MakeError(msg.GetDetail());
        }
        else if (msg.GetLevel() == StateLevel::Warning) {
          frame->MakeWarning(msg.GetDetail());
        }
        switch_to_next_tick = invoking_request;
        break;
      default:
        break;
      }

      return switch_to_next_tick;
    };

    auto cleanup_cache = [&]() -> void {
      for (auto &unit : view_delegator_) delete unit;
      view_delegator_.clear();
    };

    // Main loop of virtual machine.
    while (frame->idx < size || frame_stack_.size() > 1) {
      cleanup_cache();

      //break at stop point.
      if (frame->stop_point) break;

      if (frame->warning) {
        AppendMessage(frame->msg_string, StateLevel::Warning, logger_);
        frame->warning = false;
      }

      //switch to last stack frame when indicator reaches end of the block.
      if (frame->idx == size && frame_stack_.size() > 1) {
        //Bring saved environment back
        if (frame->inside_initializer_calling) FinishInitalizerCalling();
        else RecoverLastState(false);

        if (frame->error) break;
        //Update register data
        refresh_tick();
        if (!frame->stop_point) {
          frame->Stepping();
        }
        continue;
      }

      sentense = &(*code)[frame->idx];
      script_idx = sentense->first.idx;
      // indicator for disposing returning value or not
      frame->void_call = sentense->first.annotation.void_call; 
      frame->current_code = code;
      frame->is_command = sentense->first.type == NodeType::Operation;

      if (sentense->first.type == NodeType::Operation) {
        MachineCommands(sentense->first.GetOperation(), sentense->second, sentense->first);
        
        auto is_return = sentense->first.GetOperation() == Operation::Return;

        if (is_return) refresh_tick();
        if (frame->error) break;
        if (!frame->stop_point) frame->Stepping();
        if (!frame->rstk_operated && !is_return) {
          frame->RefreshReturnStack(Object());
        }
        frame->rstk_operated = false;
        continue;
      }
      else {
        obj_map.clear();

        if (sentense->first.type == NodeType::Function) {
          if (!FetchFunctionImpl(impl, sentense, obj_map)) {
            break;
          }
        }

        GenerateArgs(*impl, sentense->second, obj_map);
        if (frame->do_initializer_calling) GenerateStructInstance(obj_map);
        if (frame->error) break;


        next_tick = load_function_impl(false);
        if (frame->error) break;
        if (next_tick) continue;

        if (msg.HasObject()) frame->RefreshReturnStack(msg.GetObjectInfo(), msg.GetPtr());
        else frame->RefreshReturnStack(Object());
      }

      frame->Stepping();
    }

    if (frame->error) {
      AppendMessage(frame->msg_string, StateLevel::Error, logger_, script_idx);
    }

    error_ = frame->error;
  }
}
