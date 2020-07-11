#include "machine.h"

namespace sapphire {
  Message StructGetMembers(ObjectMap &p) {
    auto &struct_def = p.Cast<ObjectStruct>(kStrMe);
    auto managed_array = make_shared<ObjectArray>();

    for (auto &unit : struct_def.GetContent()) {
      managed_array->push_back(Object(unit.first));
    }

    return Message().SetObject(Object(managed_array, kTypeIdArray));
  }

  //TODO:Comparator

  void InitStructComponents() {
    using namespace components;

    CreateStruct(kTypeIdStruct);
    StructMethodGenerator(kTypeIdStruct).Create(
      {
        FunctionImpl(StructGetMembers, "", "members")
      }
    );

    CreateStruct(kTypeIdModule);
    StructMethodGenerator(kTypeIdModule).Create(
      {
        FunctionImpl(StructGetMembers, "", "members")
      }
    );

    EXPORT_CONSTANT(kTypeIdStruct);
    EXPORT_CONSTANT(kTypeIdModule);
  }
}