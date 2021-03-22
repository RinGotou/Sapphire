#include "machine.h"

namespace sapphire {
  int NullObj(State &state, ObjectMap &p) {
    auto &obj = p["obj"];
    state.PushValue(Object(obj.GetTypeId() == kTypeIdNull, kTypeIdBool));
    return 0;
  }

  int Input(State &state, ObjectMap &p) {
    string buf = GetLine();
    state.PushValue(Object(buf, kTypeIdString));
    return 0;
  }

  int SystemConsole(State &state, ObjectMap &p) {
    auto cmd = p.Cast<string>("cmd");
    int64_t result = system(cmd.data());
    state.PushValue(Object(result, kTypeIdInt));
    return 0;
  }

  int TimeString(State &state, ObjectMap &p) {
    time_t now = time(nullptr);
    string nowtime(ctime(&now));
    nowtime.pop_back();
    state.PushValue(Object(nowtime, kTypeIdString));
    return 0;
  }

  int IsBaseOf(State &state, ObjectMap &p) {
    auto &base = p["base"];
    auto &target = p["target"];

    if (!target.IsSubContainer()) {
      state.PushValue(Object(false, kTypeIdBool));
      return 0;
    }

    if (!compare(kTypeIdStruct, base.GetTypeId(), target.GetTypeId())) {
      state.SetMsg("Invalid struct");
      return 2;
    }

    auto *super_struct_ref = target.Cast<ObjectStruct>().Find(kStrSuperStruct);

    if (super_struct_ref == nullptr) {
      state.PushValue(Object(false, kTypeIdBool));
      return 0;
    }

    if (!super_struct_ref->IsAlive()) {
      state.SetMsg("Super struct object is dead");
      return 2;
    }

    auto target_base = super_struct_ref->Get();
    state.PushValue(Object(target_base == base.Get(), kTypeIdBool));
    return 0;
  }

  bool CheckObjectBehavior(State &state, Object &obj, string &behaviors) {
    auto sample = BuildStringVector(behaviors);
    bool result = true;
    auto &obj_stack = state.AccessScope();

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
      auto *struct_obj_ptr = obj_stack.Find(obj.GetTypeId(), TryAppendTokenId(obj.GetTypeId()));
      if (struct_obj_ptr != nullptr && struct_obj_ptr->IsSubContainer()) {
        auto &base = struct_obj_ptr->Cast<ObjectStruct>();
        do_checking(base);
      }
      else result = false;
    }

    return result;
  }

  int HasBehavior(State &state, ObjectMap &p) {
    auto &behaviors = p.Cast<string>("behaviors");
    auto &obj = p["obj"];

    auto result = CheckObjectBehavior(state, obj, behaviors);
    state.PushValue(Object(result, kTypeIdBool));
    return 0;
  }

  bool CheckObjectMethod(State &state, Object &obj, string id) {
    auto &obj_stack = state.AccessScope();
    bool result = false;

    if (obj.IsSubContainer()) {
      auto &base = obj.Cast<ObjectStruct>();
      auto ptr = base.Find(id);
      result = (ptr != nullptr && ptr->GetTypeId() == kTypeIdFunction);
    }
    else {
      auto *struct_obj_ptr = obj_stack.Find(obj.GetTypeId(), TryAppendTokenId(obj.GetTypeId()));
      if (struct_obj_ptr != nullptr && struct_obj_ptr->IsSubContainer()) {
        auto &base = struct_obj_ptr->Cast<ObjectStruct>();
        auto ptr = base.Find(id);
        result = (ptr != nullptr && ptr->GetTypeId() == kTypeIdFunction);
      }
    }

    return result;
  }

  int ExistMethod(State &state, ObjectMap &p) {
    auto &obj = p["obj"];
    auto &str = p.Cast<string>("str");

    bool first_stage = CheckObjectMethod(state, obj, str);
    bool second_stage = obj.IsSubContainer() ?
      [&]() -> bool {
      auto &container = obj.Cast<ObjectStruct>().GetContent();
      for (auto &unit : container) {
        if (unit.first == str) {
          return true;
        }
      }
      return false;
    }() : false;

    state.PushValue(Object(first_stage || second_stage, kTypeIdBool));
    return 0;
  }

  void GetObjectMethods(State &state, Object &obj, vector<string> &dest) {
    auto &obj_stack = state.AccessScope();

    if (obj.IsSubContainer()) {
      auto &base = obj.Cast<ObjectStruct>().GetContent();

      for (const auto &unit : base) dest.emplace_back(unit.first);
      //apppend 'members' method
      if (obj.GetTypeId() == kTypeIdStruct) dest.emplace_back("members");
    }
    else {
      auto *struct_obj_ptr = obj_stack.Find(obj.GetTypeId(), TryAppendTokenId(obj.GetTypeId()));
      if (struct_obj_ptr != nullptr && struct_obj_ptr->IsSubContainer()) {
        auto &base = struct_obj_ptr->Cast<ObjectStruct>().GetContent();
        for (const auto &unit : base) dest.emplace_back(unit.first);
      }
    }
  }

  int GetMethodList(State &state, ObjectMap &p) {
    auto &obj = p["obj"];
    vector<string> temp;
    auto managed_array = make_shared<ObjectArray>();

    GetObjectMethods(state, obj, temp);

    for (auto &unit : temp) {
      managed_array->emplace_back(Object(unit, kTypeIdString));
    }

    if (obj.IsSubContainer()) {
      auto &container = obj.Cast<ObjectStruct>().GetContent();
      for (auto &unit : container) {
        if (unit.second.GetTypeId() == kTypeIdFunction) {
          managed_array->emplace_back(unit.first);
        }
      }
    }

    state.PushValue(Object(managed_array, kTypeIdArray));
    return 0;
  }

  int Version(State &state, ObjectMap &p) {
    state.PushValue(Object(string(BUILD), kTypeIdString));
    return 0;
  }

  int Codename(State &state, ObjectMap &p) {
    state.PushValue(Object(string(CODENAME), kTypeIdString));
    return 0;
  }

  void InitBaseUtils() {
    using namespace components;

    CreateFunctionObject(Function(NullObj, "obj", "nullobj"));
    CreateFunctionObject(Function(Input, "", "input"));
    CreateFunctionObject(Function(SystemConsole, "cmd", "console"));
    CreateFunctionObject(Function(TimeString, "", "timestring"));
    CreateFunctionObject(Function(IsBaseOf, "base|target", "is_base_of"));
    CreateFunctionObject(Function(Version, "", "core_version"));
    CreateFunctionObject(Function(Codename, "", "core_codename"));
    CreateFunctionObject(Function(HasBehavior, "obj|behaviors", "has_behavior"));
    CreateFunctionObject(Function(GetMethodList, "obj", "get_methods"));
    CreateFunctionObject(Function(ExistMethod, "obj|str", "exist"));
  }
}