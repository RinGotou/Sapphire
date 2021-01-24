#include "machine.h"

namespace sapphire {
  Message Struct_GetMembers(ObjectMap &p) {
    auto &struct_def = p.Cast<ObjectStruct>(kStrMe);
    auto managed_array = make_shared<ObjectArray>();

    for (auto &unit : struct_def.GetContent()) {
      managed_array->push_back(Object(unit.first));
    }

    return Message().SetObject(Object(managed_array, kTypeIdArray));
  }

  void InitStructComponents() {
    using namespace components;

    CreateStruct(kTypeIdStruct);
    StructMethodGenerator(kTypeIdStruct).Create(
      {
        Function(Struct_GetMembers, "", "members")
      }
    );

    CreateStruct(kTypeIdModule);
    StructMethodGenerator(kTypeIdModule).Create(
      {
        Function(Struct_GetMembers, "", "members")
      }
    );

    EXPORT_CONSTANT(kTypeIdStruct);
    EXPORT_CONSTANT(kTypeIdModule);
  }
}