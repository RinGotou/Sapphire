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
    using namespace mgmt;
    using namespace mgmt::type;

    ObjectTraitsSetup(kTypeIdStruct, ShallowDelivery)
      .InitMethods({
        FunctionImpl(StructGetMembers, "", "members")
      });
    ObjectTraitsSetup(kTypeIdModule, ShallowDelivery)
      .InitMethods({
      FunctionImpl(StructGetMembers, "", "members")
      });


    EXPORT_CONSTANT(kTypeIdStruct);
    EXPORT_CONSTANT(kTypeIdModule);
  }
}