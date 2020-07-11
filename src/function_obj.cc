#include "machine.h"

namespace sapphire {
  Message FunctionGetId(ObjectMap &p) {
    auto &impl = p.Cast<Function>(kStrMe);
    return Message(impl.GetId());
  }

  Message FunctionGetParameters(ObjectMap &p) {
    auto &impl = p.Cast<Function>(kStrMe);
    shared_ptr<ObjectArray> dest_base = make_shared<ObjectArray>();
    auto origin_vector = impl.GetParameters();

    for (auto it = origin_vector.begin(); it != origin_vector.end(); ++it) {
      dest_base->emplace_back(Object(*it, kTypeIdString));
    }

    return Message().SetObject(Object(dest_base, kTypeIdArray));
  }

  Message FunctionCompare(ObjectMap &p) {
    auto &rhs = p[kStrRightHandSide];
    auto &lhs = p[kStrMe].Cast<Function>();

    string type_id = rhs.GetTypeId();
    bool result = false;

    if (type_id == kTypeIdFunction) {
      auto &rhs_interface = rhs.Cast<Function>();

      result = (lhs == rhs_interface);
    }

    return Message().SetObject(result);
  }

  void InitFunctionType() {
    using namespace components;

    CreateStruct(kTypeIdFunction);
    StructMethodGenerator(kTypeIdFunction).Create(
      {
        Function(FunctionGetId, "", "id"),
        Function(FunctionGetParameters, "", "params"),
        Function(FunctionCompare, kStrRightHandSide, kStrCompare)
      }
    );

    EXPORT_CONSTANT(kTypeIdFunction);
  }
}