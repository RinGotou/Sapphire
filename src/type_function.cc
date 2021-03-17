#include "machine.h"

namespace sapphire {
  int Function_GetId(State &state, ObjectMap &p) {
    auto &impl = p.Cast<Function>(kStrMe);
    state.PushValue(Object(impl.GetId(), kTypeIdString));
    return 0;
  }

  int Function_GetParameters(State &state, ObjectMap &p) {
    auto &impl = p.Cast<Function>(kStrMe);
    shared_ptr<ObjectArray> dest_base = make_shared<ObjectArray>();
    auto origin_vector = impl.AccessParameters();

    for (auto it = origin_vector.begin(); it != origin_vector.end(); ++it) {
      dest_base->emplace_back(Object(*it, kTypeIdString));
    }

    state.PushValue(Object(dest_base, kTypeIdArray));
    return 0;
  }

  //int Function_Compare(State &state, ObjectMap &p) {
  //  auto &rhs = p[kStrRightHandSide];
  //  auto &lhs = p[kStrMe].Cast<Function>();

  //  string type_id = rhs.GetTypeId();
  //  bool result = false;

  //  if (type_id == kTypeIdFunction) {
  //    auto &rhs_impl = rhs.Cast<Function>();

  //    result = lhs.Compare(rhs_impl);
  //  }

  //  state.PushValue(Object(result, kTypeIdBool));
  //  return 0;
  //}

  void InitFunctionType() {
    using namespace components;

    CreateStruct(kTypeIdFunction);
    StructMethodGenerator(kTypeIdFunction).Create(
      {
        Function(Function_GetId, "", "id"),
        Function(Function_GetParameters, "", "params"),
        //Function(Function_Compare, kStrRightHandSide, kStrCompare)
      }
    );

    EXPORT_CONSTANT(kTypeIdFunction);
  }
}