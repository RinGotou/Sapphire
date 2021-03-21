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
  }
}