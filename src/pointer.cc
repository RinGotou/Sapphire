#include "machine.h"

namespace sapphire {
  void InitExternalPointerComponents() {
    using namespace mgmt::type;

    ObjectTraitsSetup(kTypeIdFunctionPointer, PlainDeliveryImpl<GenericFunctionPointer>);
    ObjectTraitsSetup(kTypeIdObjectPointer, PlainDeliveryImpl<GenericPointer>);

    EXPORT_CONSTANT(kTypeIdFunctionPointer);
    EXPORT_CONSTANT(kTypeIdObjectPointer);
  }
}