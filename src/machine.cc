#include "machine.h"

#define EXPECTED_COUNT(_Count) (args.size() == _Count)

namespace sapphire {
  CommentedResult TypeChecking(ExpectationList &&lst, ObjectMap &obj_map,
    NullableList &&nullable) {
    bool result = true;
    bool found = false;
    string msg, type_id;

#define ERROR_REPORT   {                                              \
      result = false;                                                 \
      msg = "Unexpected object type: " + obj_iter->second.GetTypeId() \
        + "(Parameter: " + unit.first + ")";                          \
      break;                                                          \
    }

    for (auto &unit : lst) {
      auto obj_iter = obj_map.find(unit.first);
      bool is_nullable = find_in_list(unit.first, nullable);

      if (obj_iter != obj_map.end()) {
        type_id = obj_iter->second.GetTypeId();

        if (type_id == kTypeIdNull) {
          if (!is_nullable) ERROR_REPORT;
          continue;
        }
        else {
          if (type_id != unit.second) ERROR_REPORT;
        }
      }
      else {
        ERROR_REPORT;
      }
    }

    return { result, msg };
  }


  inline PlainType FindTypeCode(string type_id) {
    PlainType type = kNotPlainType;

    if (type_id == kTypeIdInt) type = kPlainInt;
    if (type_id == kTypeIdFloat) type = kPlainFloat;
    if (type_id == kTypeIdString) type = kPlainString;
    if (type_id == kTypeIdBool) type = kPlainBool;

    return type;
  }

  inline bool IsIllegalStringOperator(Keyword keyword) {
    return keyword != kKeywordPlus && keyword != kKeywordNotEqual
      && keyword != kKeywordEquals;
  }

  inline int64_t IntProducer(Object &obj) {
    int64_t result = 0;

    if (obj.GetTypeId() == kTypeIdInt) {
      result = obj.Cast<int64_t>();
    }
    else {
      switch (auto type = FindTypeCode(obj.GetTypeId()); type) {
      case kPlainFloat:result = static_cast<int64_t>(obj.Cast<double>()); break;
      case kPlainBool:result = obj.Cast<bool>() ? 1 : 0; break;
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
      case kPlainInt:result = static_cast<double>(obj.Cast<int64_t>()); break;
      case kPlainBool:result = obj.Cast<bool>() ? 1.0 : 0.0; break;
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
      case kPlainFloat:result = to_string(obj.Cast<double>()); break;
      case kPlainBool:result = obj.Cast<bool>() ? kStrTrue : kStrFalse; break;
      case kPlainInt:result = to_string(obj.Cast<int64_t>()); break;
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
    case kPlainInt: result = obj.Cast<int64_t>() > 0; break;
    case kPlainFloat: result = obj.Cast<double>() > 0.0; break;
    case kPlainBool: result = obj.Cast<bool>(); break;
    case kPlainString: result = !obj.Cast<string>().empty(); break;
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
    auto &machine = *static_cast<Machine *>(vm);
    Object ext_obj(ptr, disposer, string(type_id));
    auto result = machine.PushObject(string(id), ext_obj);
    return result ? 1 : 0;
  }

  void ReceiveError(void *vm, const char *msg) {
    auto &machine = *static_cast<Machine *>(vm);
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
    if (required_by_next_cond) {
      is_there_a_cond = true;
      reserved_cond = value;
    }
    else {
      if (!void_call) {
        return_stack.push_back(new Object(value, kTypeIdBool));
        has_return_value_from_invoking = stop_point;
      }
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

  void Machine::RecoverLastState(bool call_by_return) {
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

  void Machine::FinishInitalizerCalling() {
    auto instance_obj = *obj_stack_.GetCurrent().Find(kStrMe);
    instance_obj.SetDeliveringFlag();
    RecoverLastState(false);
    frame_stack_.top().RefreshReturnStack(instance_obj);
  }

  bool Machine::IsTailRecursion(size_t idx, VMCode *code) {
    if (code != code_stack_.back()) return false;

    auto &vmcode = *code;
    auto &current = vmcode[idx];
    bool result = false;

    if (idx == vmcode.size() - 1) {
      result = true;
    }
    else if (idx == vmcode.size() - 2) {
      bool needed_by_next_call =
        vmcode[idx + 1].first.GetKeywordValue() == kKeywordReturn &&
        vmcode[idx + 1].second.back().GetType() == kArgumentReturnStack &&
        vmcode[idx + 1].second.size() == 1;
      if (!current.first.option.void_call && needed_by_next_call) {
        result = true;
      }
    }

    return result;
  }

  bool Machine::IsTailCall(size_t idx) {
    if (frame_stack_.size() <= 1) return false;
    auto &vmcode = *code_stack_.back();
    bool result = false;

    if (idx == vmcode.size() - 1) {
      result = true;
    }
    else if (idx == vmcode.size() - 2) {
      bool needed_by_next_call = 
        vmcode[idx + 1].first.GetKeywordValue() == kKeywordReturn &&
        vmcode[idx + 1].second.back().GetType() == kArgumentReturnStack &&
        vmcode[idx + 1].second.size() == 1;
      if (!vmcode[idx].first.option.void_call && needed_by_next_call) {
        result = true;
      }
    }

    return result;
  }

  Object *Machine::FetchLiteralObject(Argument &arg) {
    using namespace constant;
    auto value = arg.GetData();

    auto *ptr = GetConstantObject(value);

    if (ptr != nullptr) return ptr;

    auto type = arg.GetStringType();
    if (type == kStringTypeInt) {
      int64_t int_value;
      from_chars(value.data(), value.data() + value.size(), int_value);
      ptr = CreateConstantObject(value, Object(int_value, kTypeIdInt));
    }
    else if (type == kStringTypeFloat) {
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
      case kStringTypeBool:
        ptr = CreateConstantObject(value, Object(value == kStrTrue, kTypeIdBool));
        break;
      case kStringTypeLiteralStr:
        ptr = CreateConstantObject(value, Object(ParseRawString(value)));
        break;
        //for binding expression
      case kStringTypeIdentifier:
        ptr = CreateConstantObject(value, Object(value));
        break;
      default:
        break;
      }
    }

    return ptr;
  }

  ObjectView Machine::FetchObjectView(Argument &arg) {
    using namespace constant;

#define OBJECT_DEAD_MSG {                           \
      frame.MakeError("Referenced object is dead"); \
      return ObjectView();                          \
    }

#define MEMBER_NOT_FOUND_MSG {                                                                   \
      frame.MakeError("Member '" + arg.GetData() + "' is not found inside " + arg.option.domain);\
      return ObjectView();                                                                       \
    }

    auto &frame = frame_stack_.top();
    auto &return_stack = frame.return_stack;
    ObjectPointer ptr = nullptr;
    ObjectView view;

    if (arg.GetType() == kArgumentLiteral) {
      view = FetchLiteralObject(arg);
      view.source = ObjectViewSource::kSourceLiteral;
    }
    else if (arg.GetType() == kArgumentObjectStack) {
      if (!arg.option.domain.empty() || arg.option.use_last_assert) {
        if (arg.option.use_last_assert) {
          auto &base = frame.assert_rc_copy.Cast<ObjectStruct>();
          ptr = base.Find(arg.GetData());

          if (ptr != nullptr) {
            if (!ptr->IsAlive()) OBJECT_DEAD_MSG;
            view = ObjectView(ptr);
          }
          else MEMBER_NOT_FOUND_MSG;

          if (arg.option.assert_chain_tail) frame.assert_rc_copy = Object();
        }
        else if (arg.option.domain_type == kArgumentObjectStack) {
          ptr = obj_stack_.Find(arg.GetData(), arg.option.domain, arg.option.token_id);

          if (ptr != nullptr) {
            if (!ptr->IsAlive()) OBJECT_DEAD_MSG;
            view = ObjectView(ptr);
          }
          else MEMBER_NOT_FOUND_MSG;
        }
        else if (arg.option.domain_type == kArgumentReturnStack) {
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
        if (ptr = obj_stack_.Find(arg.GetData(), arg.option.token_id); ptr != nullptr) {
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
      view.source = ObjectViewSource::kSourceReference;
    }
    else if (arg.GetType() == kArgumentReturnStack) {
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
      view.source = ObjectViewSource::kSourceReference;
    }

#undef OBJECT_DEAD_MSG
#undef MEMBER_NOT_FOUND_MSG
    return view;
  }

  bool Machine::CheckObjectBehavior(Object &obj, string behaviors) {
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

  bool Machine::CheckObjectMethod(Object &obj, string id) {
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

  void Machine::GetObjectMethods(Object &obj, vector<string> &dest) {
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

  bool Machine::FetchFunctionImplEx(FunctionImplPointer &dest, string func_id, string type_id,
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

  bool Machine::FetchFunctionImpl(FunctionImplPointer &impl, CommandPointer &command, ObjectMap &obj_map) {
    auto &frame = frame_stack_.top();
    auto id = command->first.GetInterfaceId();
    auto domain = command->first.GetInterfaceDomain();

    auto has_domain = domain.GetType() != kArgumentNull ||
      command->first.option.use_last_assert;

    if (has_domain) {
      auto view = command->first.option.use_last_assert ?
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
      if (!frame.assert_rc_copy.Null()) frame.assert_rc_copy = Object();
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
        if (impl->GetType() == kFunctionVMCode) {
          frame.initializer_calling = true;
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

  void Machine::CheckDomainObject(Function &impl, Request &req, bool first_assert) {
    auto &frame = frame_stack_.top();
    auto domain = req.GetInterfaceDomain();
    auto keyword = req.GetKeywordValue();
    auto need_catching = domain.GetType() == kArgumentObjectStack
      && ((keyword != kKeywordDomainAssertCommand)
        || (keyword == kKeywordDomainAssertCommand && first_assert));

    if (!need_catching) return;
    
    auto view = req.option.use_last_assert ?
      ObjectView(&frame.assert_rc_copy) :
      FetchObjectView(domain);

    if (frame.error) return;

    impl.AppendClosureRecord(domain.GetData(), components::DumpObject(view.Seek()));
  }

  void Machine::CheckArgrumentList(Function &impl, ArgumentList &args) {
    auto &frame = frame_stack_.top();
    string_view data;
    ArgumentType type;
    ObjectView view;
    bool need_catching = false;

    for (auto &unit : args) {
      data = unit.GetData();
      type = unit.GetType();

      if (unit.option.domain_type == kArgumentObjectStack) {
        view = ObjectView(obj_stack_.Find(unit.option.domain, unit.option.token_id));
      }
      else if (type == kArgumentObjectStack) {
        view = ObjectView(obj_stack_.Find(string(data), unit.option.token_id));
      }
      else {
        continue;
      }

      if (!view.IsValid()) {
        frame.MakeError("Object is not found: " + unit.option.domain);
        break;
      }

      if (!view.IsAlive()) {
        frame.MakeError("Object is dead: " + unit.option.domain);
        break;
      }

      impl.AppendClosureRecord(data, components::DumpObject(view.Seek()));
    }
  }

  void Machine::ClosureCatching(ArgumentList &args, size_t nest_end, bool closure) {
    auto &frame = frame_stack_.top();
    auto &obj_list = obj_stack_.GetBase();
    auto &origin_code = *code_stack_.back();
    size_t counter = 0;
    size_t size = args.size();
    size_t nest = frame.idx;
    bool optional = false;
    bool variable = false;
    bool not_assert_before = false;
    bool first_assert = false;
    ParameterPattern argument_mode = kParamFixed;
    vector<string> params;
    VMCode code(&origin_code);
    string return_value_constraint;
    auto &container = obj_stack_.GetCurrent();
    auto func_id = args[0].GetData();

    for (size_t idx = nest + 1; idx < nest_end - frame.jump_offset; ++idx) {
      code.push_back(origin_code[idx]);
    }

    for (size_t idx = 1; idx < size; idx += 1) {
      auto id = args[idx].GetData();

      if (args[idx].option.is_constraint) {
        return_value_constraint = id;
        continue;
      }

      if (args[idx].option.optional_param) {
        optional = true;
        counter += 1;
      }

      if (args[idx].option.variable_param) variable = true;

      params.push_back(id);
    }

    if (optional) argument_mode = kParamAutoFill;
    if (variable) argument_mode = kParamAutoSize;

    Function impl(nest + 1, code, args[0].GetData(), params, argument_mode);

    if (optional) {
      impl.SetLimit(params.size() - counter);
    }

    if (closure) {
      for (auto it = code.begin(); it != code.end(); ++it) {
        first_assert = not_assert_before && it->first.GetKeywordValue() == kKeywordDomainAssertCommand;
        not_assert_before = it->first.GetKeywordValue() != kKeywordDomainAssertCommand;
        CheckDomainObject(impl, it->first, first_assert);
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

  Message Machine::CallMethod(Object &obj, string id, ObjectMap &args) {
    FunctionImplPointer impl;
    auto &frame = frame_stack_.top();
    Message result;

    if (!FetchFunctionImplEx(impl, id, obj.GetTypeId(), &obj)) return result;

    ObjectMap obj_map = args;
    obj_map.emplace(NamedObject(kStrMe, obj));

    if (impl->GetType() == kFunctionVMCode) {
      result = CallVMCFunction(*impl, obj_map);
    }
    else if (impl->GetType() == kFunctionExternal) {
      frame.MakeError("External function isn't supported for now");
    }
    else if (impl->GetType() == kFunctionCXX) {
      auto activity = impl->Get<Activity>();
      result = activity(obj_map);
    }
    else {
      frame.MakeError("Unknown function implementation (Internal error)");
    }

    return result;
  }

  Message Machine::CallMethod(Object &obj, string id, const initializer_list<NamedObject> &&args) {
    ObjectMap obj_map = args;
    return CallMethod(obj, id, obj_map);
  }

  Message Machine::CallVMCFunction(Function &impl, ObjectMap &obj_map) {
    auto &frame = frame_stack_.top();
    Message result;

    if (impl.GetType() != kFunctionVMCode) {
      frame.MakeError("Invalid function variant");
      return result;
    }

    frame.stop_point = true;
    code_stack_.push_back(&impl.Get<VMCode>());
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

  void Machine::CommandLoad(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    auto view = FetchObjectView(args[0]);
    if (frame.error) return;
    frame.RefreshReturnStack(std::move(view));
  }

  void Machine::CommandIfOrWhile(Keyword token, ArgumentList &args, size_t nest_end) {
    auto &frame = frame_stack_.top();
    auto &code = code_stack_.front();
    bool has_jump_record = false;
    bool state = false;

    if (!EXPECTED_COUNT(1)) {
      frame.MakeError("Argument for condition is missing");
      return;
    }

    if (token == kKeywordIf || token == kKeywordWhile) {
      frame.AddJumpRecord(nest_end);
      has_jump_record = code->FindJumpRecord(frame.idx + frame.jump_offset, frame.branch_jump_stack);
    }
    
    if (frame.is_there_a_cond) {
      state = frame.reserved_cond;
      frame.is_there_a_cond = false;
    }
    else {
      ObjectView view = FetchObjectView(args[0]);

      if (frame.error) return;

      if (view.Seek().GetTypeId() != kTypeIdBool) {
        frame.MakeError("Invalid state value type.");
        return;
      }

      state = view.Seek().Cast<bool>();
    }

    if (token == kKeywordIf) {
      auto create_env = [&]()->void {
        frame.scope_stack.push(true);
        frame.condition_stack.push(state);
        obj_stack_.Push(true, true);
      };

      if (!state) {
        if (frame.branch_jump_stack.empty()) {
          frame.Goto(frame.jump_stack.top());
          frame.cancel_cleanup = true;
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
    else if (token == kKeywordElif) {
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
    else if (token == kKeywordWhile) {
      if (!frame.jump_from_end) {
        frame.scope_stack.push(true);
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

  void Machine::CommandForEach(ArgumentList &args, size_t nest_end) {
    // TODO: Remove iterator type and using duck type system for other types.
    // TODO: Remove redundant container fetching
    auto &frame = frame_stack_.top();
    ObjectMap obj_map;

    frame.AddJumpRecord(nest_end);

    if (frame.jump_from_end) {
      ForEachChecking(args, nest_end);
      frame.jump_from_end = false;
      return;
    }

    auto unit_id = FetchObjectView(args[0]).Seek().Cast<string>();
    //keep alive
    auto container_obj = FetchObjectView(args[1]).Seek();

    if (frame.error) return;

    if (!CheckObjectBehavior(container_obj, kContainerBehavior)) {
      frame.MakeError("Invalid container object");
      return;
    }

    auto msg = CallMethod(container_obj, kStrHead);
    if (frame.error) return;
    if (!msg.HasObject()) {
      frame.MakeError("Invalid returning value from iterator");
      return;
    }

    auto empty = CallMethod(container_obj, "empty");
    if (frame.error) return;
    if (!empty.HasObject() || empty.GetObj().GetTypeId() != kTypeIdBool) {
      frame.MakeError("Invalid empty() implementation");
      return;
    }
    else if (empty.GetObj().Cast<bool>()) {
      frame.Goto(nest_end);
      frame.final_cycle = true;
      obj_stack_.Push(true); //avoid error
      frame.scope_stack.push(false);
      return;
    }

    auto iterator_obj = msg.GetObj();
    if (!CheckObjectBehavior(iterator_obj, kIteratorBehavior)) {
      frame.MakeError("Invalid iterator object");
      return;
    }

    auto unit = CallMethod(iterator_obj, "obj").GetObj();
    if (frame.error) return;

    frame.scope_stack.push(true);
    obj_stack_.Push(true);
    obj_stack_.CreateObject(kStrIteratorObj, iterator_obj);
    obj_stack_.CreateObject(kStrContainerKeepAliveSlot, container_obj);
    obj_stack_.CreateObject(unit_id, unit, TryAppendTokenId(unit_id));
  }

  void Machine::ForEachChecking(ArgumentList &args, size_t nest_end) {
    auto &frame = frame_stack_.top();
    auto unit_id = FetchObjectView(args[0]).Seek().Cast<string>();
    if (frame.error) return;

    auto *iterator = obj_stack_.GetCurrent().Find(kStrIteratorObj);
    auto *container = obj_stack_.GetCurrent().Find(kStrContainerKeepAliveSlot);
    ObjectMap obj_map;

    auto tail = CallMethod(*container, kStrTail).GetObj();
    if (frame.error) return;
    if (!CheckObjectBehavior(tail, kIteratorBehavior)) {
      frame.MakeError("Invalid container object");
      return;
    }

    CallMethod(*iterator, "step_forward");
    if (frame.error) return;

    auto result = CallMethod(*iterator, kStrCompare,
      { NamedObject(kStrRightHandSide,tail) }).GetObj();
    if (frame.error) return;

    if (result.GetTypeId() != kTypeIdBool) {
      frame.MakeError("Invalid iterator object");
      return;
    }

    if (result.Cast<bool>()) {
      frame.Goto(nest_end);
      frame.final_cycle = true;
    }
    else {
      auto unit = CallMethod(*iterator, "obj").GetObj();
      if (frame.error) return;
      obj_stack_.CreateObject(unit_id, unit, TryAppendTokenId(unit_id));
    }
  }

  void Machine::CommandCase(ArgumentList &args, size_t nest_end) {
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

    frame.scope_stack.push(true);
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

  void Machine::CommandElse() {
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

  void Machine::CommandWhen(ArgumentList &args) {
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

  void Machine::CommandContinueOrBreak(Keyword token, size_t escape_depth) {
    auto &frame = frame_stack_.top();
    auto &scope_stack = frame.scope_stack;

    while (escape_depth != 0) {
      frame.condition_stack.pop();
      frame.jump_stack.pop();
      if (!scope_stack.empty() && scope_stack.top()) {
        obj_stack_.Pop();
        
      }
      scope_stack.pop();
      escape_depth -= 1;
    }

    frame.Goto(frame.jump_stack.top());

    switch (token) {
    case kKeywordContinue:
      frame.activated_continue = true; 
      break;
    case kKeywordBreak:
      frame.activated_break = true; 
      frame.final_cycle = true;
      break;
    default:break;
    }
  }

  void Machine::CommandStructBegin(ArgumentList &args) {
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

    if (!super_struct_obj.Null()) {
      frame.super_struct_id = super_struct_obj.Cast<string>();
    }

    //NOTICE: frame.struct_id = id_obj.Cast<string>();
    if (auto *ptr = obj_stack_.Find(frame.struct_id, args[0].option.token_id); ptr != nullptr) {
      frame.MakeError("Struct is existed: " + frame.struct_id);
    }
  }

  void Machine::CommandModuleBegin(ArgumentList &args) {
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

  void Machine::CommandConditionEnd() {
    auto &frame = frame_stack_.top();
    if (!frame.cancel_cleanup) {
      frame.condition_stack.pop();
      frame.scope_stack.pop();
      obj_stack_.Pop();
      
      while (!frame.branch_jump_stack.empty()) frame.branch_jump_stack.pop();
      frame.cancel_cleanup = false;
    }

    frame.jump_stack.pop();
  }

  void Machine::CommandLoopEnd(size_t nest) {
    auto &frame = frame_stack_.top();

    if (frame.final_cycle) {
      if (frame.activated_continue) {
        frame.Goto(nest);
        frame.activated_continue = false;
        obj_stack_.ClearCurrent();
        //obj_stack_.GetCurrent().Clear();
        frame.jump_from_end = true;
      }
      else {
        if (frame.activated_break) frame.activated_break = false;
        while (!frame.return_stack.empty()) {
          delete frame.return_stack.back();
          frame.return_stack.pop_back();
        }
        frame.jump_stack.pop();
        obj_stack_.Pop();
        
      }
      frame.scope_stack.pop();
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

  void Machine::CommandForEachEnd(size_t nest) {
    auto &frame = frame_stack_.top();

    if (frame.final_cycle) {
      if (frame.activated_continue) {
        frame.Goto(nest);
        frame.activated_continue = false;
        obj_stack_.GetCurrent().ClearExcept(kForEachExceptions);
        frame.jump_from_end = true;
      }
      else {
        if (frame.activated_break) frame.activated_break = false;
        frame.jump_stack.pop();
        obj_stack_.Pop();
        
      }
      if(!frame.scope_stack.empty()) frame.scope_stack.pop();
      frame.final_cycle = false;
    }
    else {
      frame.Goto(nest);
      obj_stack_.GetCurrent().ClearExcept(kForEachExceptions);
      frame.jump_from_end = true;
    }
  }

  void Machine::CommandStructEnd() {
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

  void Machine::CommandModuleEnd() {
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

  void Machine::CommandInclude(ArgumentList &args) {
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

  void Machine::CommandSuper(ArgumentList &args) {
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

  void Machine::CommandAttribute(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    bool error = false;

    if (args.size() == 0) {
      frame.MakeError("Expect one argrument at least");
      return;
    }

    for (auto &unit : args) {
      if (unit.GetType() != kArgumentObjectStack) {
        error = true;
        break;
      }

      obj_stack_.CreateObject(unit.GetData(), Object(), unit.option.token_id);
    }

    if (error) {
      frame.MakeError("Invalid argument for attribute identifier");
    }
  }

  void Machine::CommandSwap(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (args.size() == 2) {
      if (args[0].GetType() == kArgumentLiteral || args[1].GetType() == kArgumentLiteral) {
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

  void Machine::CommandSwapIf(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(3)) {
      frame.MakeError("Argument missing");
      return;
    }

    if (args[0].GetType() == kArgumentLiteral || args[1].GetType() == kArgumentLiteral) {
      frame.MakeError("Cannot modify a literal value");
      return;
    }
    
    auto &right = FetchObjectView(args[1]).Seek();
    auto &left = FetchObjectView(args[0]).Seek();

    if (frame.error) return;

    if (frame.is_there_a_cond) {
      if (frame.reserved_cond) {
        left.swap(right);
      }
      frame.is_there_a_cond = false;
    }
    else {
      auto &cond = FetchObjectView(args[2]).Seek();
      if (cond.GetTypeId() != kTypeIdBool) {
        frame.MakeError("Invalid condition value");
        return;
      }

      if (cond.Cast<bool>()) left.swap(right);
    }
  }

  void Machine::CommandBind(ArgumentList &args, bool local_value, bool ext_value) {
    auto &frame = frame_stack_.top();

    if (args[0].GetType() == kArgumentLiteral &&
      lexical::GetStringType(args[0].GetData(), true) != kStringTypeIdentifier) {
      frame.MakeError("Cannot modify a literal value");
      return;
    }

    //Do not change the order!
    auto rhs = FetchObjectView(args[1]);
    auto lhs = FetchObjectView(args[0]);

    if (frame.error) return;

    if (lhs.source == ObjectViewSource::kSourceReference) {
      auto &real_lhs = lhs.Seek().Unpack();
      real_lhs = components::DumpObject(rhs.Seek());
      return;
    }
    else {
      string id = lhs.Seek().Cast<string>();

      if (lexical::GetStringType(id) != kStringTypeIdentifier) {
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

      if (!obj_stack_.CreateObject(id, components::DumpObject(rhs.Seek()), 
        args[0].option.token_id)) {
        frame.MakeError("Object binding is failed");
        return;
      }
    }
  }

  void Machine::CommandDelivering(ArgumentList &args, bool local_value, bool ext_value) {
    auto &frame = frame_stack_.top();

    if (args[0].GetType() == kArgumentLiteral &&
      lexical::GetStringType(args[0].GetData(), true) != kStringTypeIdentifier) {
      frame.MakeError("Cannot modify a literal value");
      return;
    }

    if (args[1].GetType() == kArgumentLiteral) {
      frame.MakeError("Cannot modify a literal value");
      return;
    }

    //Do not change the order!
    auto rhs = FetchObjectView(args[1]);
    auto lhs = FetchObjectView(args[0]);

    if (frame.error) return;

    if (lhs.source == ObjectViewSource::kSourceReference) {
      auto &real_lhs = lhs.Seek().Unpack();
      real_lhs = rhs.Seek();
      rhs.Seek().Unpack() = Object();
    }
    else {
      string id = lhs.Seek().Cast<string>();

      if (lexical::GetStringType(id) != kStringTypeIdentifier) {
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

      if (!obj_stack_.CreateObject(id, rhs.Seek().Unpack(), args[0].option.token_id)) {
        frame.MakeError("Object delivering is failed");
        return;
      }
    }
  }

  void Machine::CommandTypeId(ArgumentList &args) {
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

  void Machine::CommandMethods(ArgumentList &args) {
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

  void Machine::CommandExist(ArgumentList &args) {
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

  void Machine::CommandNullObj(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(1)) {
      frame.MakeError("Argument mismatching: null_obj(obj)");
      return;
    }

    Object &obj = FetchObjectView(args[0]).Seek();
    if (frame.error) return;

    frame.RefreshReturnStack(Object(obj.GetTypeId() == kTypeIdNull, kTypeIdBool));
  }

  void Machine::CommandToString(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(1)) {
      frame.MakeError("Argument mismatching: convert(obj)");
      return;
    }

    Argument &arg = args[0];
    if (arg.GetType() == kArgumentLiteral) {
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
        case kStringTypeInt:
          ret_obj.PackContent(make_shared<int64_t>(stol(str)), kTypeIdInt);
          break;
        case kStringTypeFloat:
          ret_obj.PackContent(make_shared<double>(stod(str)), kTypeIdFloat);
          break;
        case kStringTypeBool:
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

  void Machine::CommandUsing(ArgumentList &args) {
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

      VMCode &script_file = script::AppendBlankScript(absolute_path);

      //already loaded
      if (!script_file.empty()) return;

      VMCodeFactory factory(absolute_path, script_file, logger_);

      if (factory.Start()) {
        Machine sub_machine(script_file, logger_);
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

  void Machine::CommandPrint(ArgumentList &args) {
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

  void Machine::CommandInput(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    if (!args.empty()) {
      frame.MakeError("Invalid arguments: input()");
      return;
    }

    string buf = GetLine();
    frame.RefreshReturnStack(Object(buf, kTypeIdString));
  }

  void Machine::CommandGetChar(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    if (!args.empty()) {
      frame.MakeError("Invalid arguments: getchar()");
      return;
    }

    auto value = static_cast<char>(fgetc(VM_STDIN));
    frame.RefreshReturnStack(Object(string().append(1, value), kTypeIdString));
  }

  void Machine::SysCommand(ArgumentList &args) {
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

  void Machine::CommandSleep(ArgumentList &args) {
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

  void Machine::CommandTime() {
    auto &frame = frame_stack_.top();
    time_t now = time(nullptr);
    string nowtime(ctime(&now));
    nowtime.pop_back();
    frame.RefreshReturnStack(Object(nowtime));
  }

  void Machine::CommandVersion() {
    auto &frame = frame_stack_.top();
    frame.RefreshReturnStack(Object(PRODUCT_VER));
  }

  void Machine::CommandMachineCodeName() {
    auto &frame = frame_stack_.top();
    frame.RefreshReturnStack(Object(CODENAME));
  }

  template <Keyword op_code>
  void Machine::BinaryMathOperatorImpl(ArgumentList &args) {
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

    if (type_lhs == kNotPlainType || type_rhs  == kNotPlainType) {
      frame.MakeError("Try to operate with non-plain type.");
      return;
    }

    auto result_type = kResultDynamicTraits.at(ResultTraitKey(type_lhs, type_rhs));

#define RESULT_PROCESSING(_Type, _Func, _TypeId)                                     \
  _Type result = MathBox<_Type, op_code>().Do(_Func(lhs.Seek()), _Func(rhs.Seek())); \
  frame.RefreshReturnStack(Object(result, _TypeId));

    if (result_type == kPlainString) {
      if (IsIllegalStringOperator(op_code)) {
        frame.RefreshReturnStack(Object());
        return;
      }

      RESULT_PROCESSING(string, StringProducer, kTypeIdString);
    }
    else if (result_type == kPlainInt) {
      RESULT_PROCESSING(int64_t, IntProducer, kTypeIdInt);
    }
    else if (result_type == kPlainFloat) {
      RESULT_PROCESSING(double, FloatProducer, kTypeIdFloat);
    }
    else if (result_type == kPlainBool) {
      RESULT_PROCESSING(bool, BoolProducer, kTypeIdBool);
    }
#undef RESULT_PROCESSING
  }

  template <Keyword op_code>
  void Machine::BinaryLogicOperatorImpl(ArgumentList &args) {
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
      if constexpr (op_code != kKeywordEquals && op_code != kKeywordNotEqual) {
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

        frame.RefreshReturnStack(op_code == kKeywordNotEqual ? !value : value);
      }

      return;
    }

    auto result_type = kResultDynamicTraits.at(ResultTraitKey(type_lhs, type_rhs));
#define RESULT_PROCESSING(_Type, _Func) \
  result = LogicBox<_Type, op_code>().Do(_Func(lhs.Seek()), _Func(rhs.Seek()));

    if (result_type == kPlainString) {
      if (IsIllegalStringOperator(op_code)) {
        frame.RefreshReturnStack(Object());
        return;
      }

      RESULT_PROCESSING(string, StringProducer);
    }
    else if (result_type == kPlainInt) {
      RESULT_PROCESSING(int64_t, IntProducer);
    }
    else if (result_type == kPlainFloat) {
      RESULT_PROCESSING(double, FloatProducer);
    }
    else if (result_type == kPlainBool) {
      RESULT_PROCESSING(bool, BoolProducer);
    }


    frame.RefreshReturnStack(result);
#undef RESULT_PROCESSING
  }

  void Machine::OperatorLogicNot(ArgumentList &args) {
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

    if (frame.required_by_next_cond) {
      frame.reserved_cond = result;
      frame.is_there_a_cond = true;
    }
    else {
      frame.RefreshReturnStack(result);
    }
  }

  void Machine::OperatorIncreasing(ArgumentList &args) {
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

  void Machine::OperatorDecreasing(ArgumentList &args) {
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
  void Machine::ExpList(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    if (frame.is_there_a_cond) {
      //DO NOTHING
    }
    else if (!args.empty()) {
      auto result_view = FetchObjectView(args.back());

      if (result_view.Seek().GetTypeId() == kTypeIdBool) {
        frame.is_there_a_cond = true;
        frame.reserved_cond = result_view.Seek().Cast<bool>();
      }
      else {
        frame.RefreshReturnStack(result_view.Seek());
      }
    }
  }

  void Machine::InitArray(ArgumentList &args) {
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

  void Machine::CommandReturn(ArgumentList &args) {
    if (frame_stack_.size() == 1) {
      frame_stack_.top().MakeError("Unexpected return");
      return;
    }

    ObjectPointer constraint_type_obj = obj_stack_.GetCurrent().Find(kStrReturnValueConstrantObj);
    string constraint_type = constraint_type_obj == nullptr ?
      "" : constraint_type_obj->Cast<string>();

    if (args.size() == 1) {
      //keep alive
      Object ret_obj = FetchObjectView(args[0]).Seek().Unpack();

      if (!constraint_type.empty() && ret_obj.GetTypeId() != constraint_type) {
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
      frame_stack_.top().RefreshReturnStack(ret_obj);
     
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

  void Machine::CommandAssert(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(1)) {
      frame.MakeError("Argument mismatching: assert(bool_obj)");
      return;
    }

    if (frame.is_there_a_cond) {
      if (frame.reserved_cond) frame.MakeError("User assertion failed.");
      frame.is_there_a_cond = false;
    }
    else {
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
  }

  void Machine::DomainAssert(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    frame.assert_rc_copy = FetchObjectView(args[0]).Seek().Unpack();
  }

  void Machine::CommandIsBaseOf(ArgumentList &args) {
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

  void Machine::CommandHasBehavior(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    auto &behavior_obj = FetchObjectView(args[1]).Seek();
    auto &obj = FetchObjectView(args[0]).Seek();

    if (frame.error) return;

    auto result = CheckObjectBehavior(obj, behavior_obj.Cast<string>());

    frame.RefreshReturnStack(Object(result, kTypeIdBool));
  }

  template <ParameterPattern pattern>
  void Machine::CommandCheckParameterPattern(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (!EXPECTED_COUNT(1)) {
      if constexpr (pattern == kParamAutoSize) {
        frame.MakeError("Argument mismatching: is_variable_param(func)");
      }
      else if constexpr (pattern == kParamAutoFill) {
        frame.MakeError("Argument mismatching: is_optional_param(func");
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

  void Machine::CommandOptionalParamRange(ArgumentList &args) {
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

  void Machine::MachineCommands(Keyword token, ArgumentList &args, Request &request) {
    auto &frame = frame_stack_.top();

    switch (token) {
    case kKeywordLoad:
      CommandLoad(args);
      break;
    case kKeywordIf:
    case kKeywordElif:
    case kKeywordWhile:
      CommandIfOrWhile(token, args, request.option.nest_end);
      break;
    case kKeywordPlus:
      BinaryMathOperatorImpl<kKeywordPlus>(args);
      break;
    case kKeywordMinus:
      BinaryMathOperatorImpl<kKeywordMinus>(args);
      break;
    case kKeywordTimes:
      BinaryMathOperatorImpl<kKeywordTimes>(args);
      break;
    case kKeywordDivide:
      BinaryMathOperatorImpl<kKeywordDivide>(args);
      break;
    case kKeywordEquals:
      BinaryLogicOperatorImpl<kKeywordEquals>(args);
      break;
    case kKeywordLessOrEqual:
      BinaryLogicOperatorImpl<kKeywordLessOrEqual>(args);
      break;
    case kKeywordGreaterOrEqual:
      BinaryLogicOperatorImpl<kKeywordGreaterOrEqual>(args);
      break;
    case kKeywordNotEqual:
      BinaryLogicOperatorImpl<kKeywordNotEqual>(args);
      break;
    case kKeywordGreater:
      BinaryLogicOperatorImpl<kKeywordGreater>(args);
      break;
    case kKeywordLess:
      BinaryLogicOperatorImpl<kKeywordLess>(args);
      break;
    case kKeywordAnd:
      BinaryLogicOperatorImpl<kKeywordAnd>(args);
      break;
    case kKeywordOr:
      BinaryLogicOperatorImpl<kKeywordOr>(args);
      break;
    case kKeywordIncrease:
      OperatorIncreasing(args);
      break;
    case kKeywordDecrease:
      OperatorDecreasing(args);
      break;
    case kKeywordNot:
      OperatorLogicNot(args);
      break;
    case kKeywordFor:
      CommandForEach(args, request.option.nest_end);
      break;
    case kKeywordNullObj:
      CommandNullObj(args);
      break;
    case kKeywordToString:
      CommandToString(args);
      break;
    case kKeywordTime:
      CommandTime();
      break;
    case kKeywordVersion:
      CommandVersion();
      break;
    case kKeywordCodeName:
      CommandMachineCodeName();
      break;
    case kKeywordSwap:
      CommandSwap(args);
      break;
    case kKeywordSwapIf:
      CommandSwapIf(args);
      break;
    case kKeywordBind:
      CommandBind(args, request.option.local_object,
        request.option.ext_object);
      break;
    case kKeywordDelivering:
      CommandDelivering(args, request.option.local_object,
        request.option.ext_object);
      break;
    case kKeywordExpList:
      ExpList(args);
      break;
    case kKeywordInitialArray:
      InitArray(args);
      break;
    case kKeywordReturn:
      CommandReturn(args);
      break;
    case kKeywordAssert:
      CommandAssert(args);
      break;
    case kKeywordTypeId:
      CommandTypeId(args);
      break;
    case kKeywordMethods:
      CommandMethods(args);
      break;
    case kKeywordExist:
      CommandExist(args);
      break;
    case kKeywordFn:
      ClosureCatching(args, request.option.nest_end, frame_stack_.size() > 1);
      break;
    case kKeywordCase:
      CommandCase(args, request.option.nest_end);
      break;
    case kKeywordWhen:
      CommandWhen(args);
      break;
    case kKeywordEnd:
      switch (request.option.nest_root) {
      case kKeywordWhile:
        CommandLoopEnd(request.option.nest);
        break;
      case kKeywordFor:
        CommandForEachEnd(request.option.nest);
        break;
      case kKeywordIf:
      case kKeywordCase:
        CommandConditionEnd();
        break;
      case kKeywordStruct:
        CommandStructEnd();
        break;
      case kKeywordModule:
        CommandModuleEnd();
        break;
      default:break;
      }
      break;
    case kKeywordContinue:
    case kKeywordBreak:
      CommandContinueOrBreak(token, request.option.escape_depth);
      break;
    case kKeywordElse:
      CommandElse();
      break;
    case kKeywordUsing:
      CommandUsing(args);
      break;
    case kKeywordStruct:
      CommandStructBegin(args);
      break;
    case kKeywordModule:
      CommandModuleBegin(args);
      break;
    case kKeywordDomainAssertCommand:
      DomainAssert(args);
      break;
    case kKeywordInclude:
      CommandInclude(args);
      break;
    case kKeywordSuper:
      CommandSuper(args);
      break;
    case kKeywordAttribute:
      CommandAttribute(args);
      break;
    case kKeywordIsBaseOf:
      CommandIsBaseOf(args);
      break;
    case kKeywordHasBehavior:
      CommandHasBehavior(args);
      break;
    case kKeywordIsVariableParam:
      CommandCheckParameterPattern<kParamAutoSize>(args);
      break;
    case kKeywordIsOptionalParam:
      CommandCheckParameterPattern<kParamAutoFill>(args);
      break;
    case kKeywordOptionalParamRange:
      CommandOptionalParamRange(args);
      break;
    case kKeywordPrint:
      CommandPrint(args);
      break;
    case kKeywordPrintLine:
      CommandPrint(args);
      fputs("\n", VM_STDOUT);
      break;
    case kKeywordInput:
      CommandInput(args);
      break;
    case kKeywordConole:
      SysCommand(args);
      break;
    case kKeywordGetChar:
      CommandGetChar(args);
    default:
      break;
    }
  }

  void Machine::GenerateArgs(Function &impl, ArgumentList &args, ObjectMap &obj_map) {
    switch (impl.GetPattern()) {
    case kParamFixed:
      Generate_Fixed(impl, args, obj_map);
      break;
    case kParamAutoSize:
      Generate_AutoSize(impl, args, obj_map);
      break;
    case kParamAutoFill:
      Generate_AutoFill(impl, args, obj_map);
      break;
    default:
      break;
    }
  }

  void Machine::Generate_Fixed(Function &impl, ArgumentList &args, ObjectMap &obj_map) {
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

  void Machine::Generate_AutoSize(Function &impl, ArgumentList &args, ObjectMap &obj_map) {
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

  void Machine::Generate_AutoFill(Function &impl, ArgumentList &args, ObjectMap &obj_map) {
    auto &frame = frame_stack_.top();
    auto &params = impl.GetParameters();
    size_t limit = impl.GetLimit();
    size_t pos = args.size() - 1, param_pos = params.size() - 1;

    if (args.size() > params.size()) {
      frame.MakeError("Too many arguments");
      return;
    }

    if (args.size() < limit) {
      frame.MakeError("Minimum argument amount is " + to_string(limit));
      return;
    }

    for (auto it = params.crbegin(); it != params.crend(); ++it) {
      if (param_pos != pos) {
        obj_map.emplace(NamedObject(*it, Object()));
      }
      else {
        obj_map.emplace(NamedObject(*it, FetchObjectView(args[pos]).Dump().RemoveDeliveringFlag()));
        pos -= 1;
      }
      param_pos -= 1;
    }
  }

  void Machine::CallExtensionFunction(ObjectMap &p, Function &impl) {
    auto &frame = frame_stack_.top();
    Object returning_slot;
    auto ext_activity = impl.Get<ExtensionActivity>();
    VMState vm_state{ &p, &returning_slot, this, ReceiveExtReturningValue };
    auto result_code = ext_activity(vm_state);
    if (result_code < 1) {
      frame.MakeError("Extension reports error while invoking external activity.");
      return;
    }
    frame.RefreshReturnStack(returning_slot);
  }

  void Machine::GenerateStructInstance(ObjectMap &p) {
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

    if (!super_struct.Null()) {
      p.insert(NamedObject(kStrSuperStruct, super_struct));
    }

    Object instance_obj(managed_instance, struct_id);
    instance_obj.SetContainerFlag();
    p.insert(NamedObject(kStrMe, instance_obj));
    frame.struct_base = Object();
  }

  void Machine::GenerateErrorMessages(size_t stop_index) {
    //Under consideration
    if (frame_stack_.top().error) {
      //TODO:reporting function calling chain
      AppendMessage(frame_stack_.top().msg_string, kStateError,
        logger_, stop_index);
    }

    frame_stack_.pop();

    while (!frame_stack_.empty()) {
      if (frame_stack_.top().stop_point) break;
      frame_stack_.pop();
    }
  }

  //for extension callback facilities
  bool Machine::PushObject(string id, Object object) {
    auto &frame = frame_stack_.top();
    auto result = obj_stack_.CreateObject(id, object, TryAppendTokenId(id));
    if (!result) {
      frame.MakeError("Cannot create object");
      return false;
    }
    return true;
  }

  void Machine::PushError(string msg) {
    auto &frame = frame_stack_.top();
    frame.MakeError(msg);
  }

  void Machine::CopyComponents() {
    auto &base = obj_stack_.GetCurrent();
    auto &comp_base = components::GetBuiltinComponentsObjBase();
    
    for (auto &unit : comp_base) {
      base.Add(unit.first, unit.second, TryAppendTokenId(unit.first));
    }
  }

  void Machine::Run(bool invoke) {
    if (code_stack_.empty()) return;

    bool                next_tick;
    size_t              script_idx = 0;
    Message             msg;
    VMCode              *code = code_stack_.back();
    Command             *command = nullptr;
    ObjectMap           obj_map;
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
      //block other event trigger while processing current event function
      bool inside_initializer_calling = frame->initializer_calling;
      frame->initializer_calling = false;
      code_stack_.push_back(&func.Get<VMCode>());
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
      code_stack_.push_back(&func.Get<VMCode>());
      obj_map.Naturalize(obj_stack_.GetCurrent());
      frame_stack_.top() = RuntimeFrame(func.GetId());
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
      case kFunctionVMCode:
        //start new processing in next tick.
        if (invoking_request) goto direct_load_vmcode;
        if (IsTailRecursion(frame->idx, &impl->Get<VMCode>())) tail_recursion();
        else if (IsTailCall(frame->idx)) tail_call(*impl);
        else {
        direct_load_vmcode:
          update_stack_frame(*impl);
        }
        switch_to_next_tick = true;
        break;
      case kFunctionExternal:
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
      case kFunctionCXX:
        msg = impl->Get<Activity>()(obj_map);
        if (msg.GetLevel() == kStateError) {
          frame->MakeError(msg.GetDetail());
        }
        else if (msg.GetLevel() == kStateWarning) {
          frame->MakeWarning(msg.GetDetail());
        }
        switch_to_next_tick = invoking_request;
        break;
      default:
        break;
      }

      return switch_to_next_tick;
    };

    auto unpack_invoking_request = [&]() -> bool {
      bool failed = false;

      auto invoking_req = BuildStringVector(msg.GetDetail());
      auto obj = msg.GetObj();
      failed = FetchFunctionImplEx(impl, invoking_req[0], invoking_req[1], &obj);

      return failed;
    };

    auto is_required_by_cond = [&]() -> bool {
      bool main_trigger = lexical::IsOperator(command->first.GetKeywordValue());
      if (frame->idx >= size - 1) return false;
      auto &next_cmd = (*code)[frame->idx + 1];
      auto keyword_value = next_cmd.first.GetKeywordValue();
      auto arg_type = next_cmd.second.size() > 0 ?
        next_cmd.second.back().GetType() :
        kArgumentNull;
      bool explist_optimization = false;

      if (frame->idx < size - 2) {
        auto &next2_cmd = (*code)[frame->idx + 2];
        auto keyword_value2 = next2_cmd.first.GetKeywordValue();
        auto arg_type2 = next2_cmd.second.size() > 0 ?
          next2_cmd.second.back().GetType() :
          kArgumentNull;
        explist_optimization = (keyword_value == kKeywordExpList)
          && (keyword_value2 == kKeywordWhile
            || keyword_value2 == kKeywordIf);
      }

      return ((keyword_value == kKeywordIf 
        || keyword_value == kKeywordWhile
        || keyword_value == kKeywordCSwapIf
        || keyword_value == kKeywordSwapIf
        || keyword_value == kKeywordAssert) 
         || explist_optimization)
        && arg_type == kArgumentReturnStack
        && main_trigger;
    };

    auto cleanup_cache = [&]() -> void {
      for (auto &unit : view_delegator_) delete unit;
      view_delegator_.clear();
    };

    // Main loop of virtual machine.
    // TODO:dispose return value in event function
    while (frame->idx < size || frame_stack_.size() > 1) {
      cleanup_cache();

      //break at stop point.
      if (frame->stop_point) break;

      if (frame->warning) {
        AppendMessage(frame->msg_string, kStateWarning, logger_);
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
        frame->Stepping();
        continue;
      }

      //load current command and refreshing indicators
      command          = &(*code)[frame->idx];
      script_idx       = command->first.idx;
      frame->void_call = command->first.option.void_call; // dispose returning value
      frame->required_by_next_cond = is_required_by_cond();
      frame->current_code = code;
      frame->is_command = command->first.type == kRequestCommand;

      //Built-in machine commands.
      if (command->first.type == kRequestCommand) {
        MachineCommands(command->first.GetKeywordValue(), 
          command->second, command->first);
        
        auto is_return = command->first.GetKeywordValue() == kKeywordReturn;

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
        // TODO: method predictions
        //cleaning object map for user-defined function and C++ function
        obj_map.clear();

        //Query function(Interpreter built-in or user-defined)
        //error string will be generated in FetchFunctionImpl.
        if (command->first.type == kRequestFunction) {
          if (!FetchFunctionImpl(impl, command, obj_map)) {
            break;
          }
        }

        //Build object map for function call expressed by command
        GenerateArgs(*impl, command->second, obj_map);
        if (frame->initializer_calling) GenerateStructInstance(obj_map);
        if (frame->error) break;


        next_tick = load_function_impl(false);
        if (frame->error) break;
        if (next_tick) continue;

        //Pushing returning value to returning stack.
        if (msg.HasObject()) frame->RefreshReturnStack(msg.GetObjectInfo(), msg.GetPtr());
        else frame->RefreshReturnStack(Object());
      }
      //indicator + 1
      frame->Stepping();
    }

    if (frame->error) {
      //TODO:reporting function calling chain
      AppendMessage(frame->msg_string, kStateError,
        logger_, script_idx);
    }

    error_ = frame->error;
  }
}
