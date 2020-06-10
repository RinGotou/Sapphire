#include "machine.h"

namespace sapphire {
  void InitExternalPointerComponents() {
    using namespace mgmt::type;

    ObjectTraitsSetup(kTypeIdFunctionPointer, 
      PlainDeliveryImpl<GenericFunctionPointer>, 
      PlainHasher<GenericFunctionPointer>)
      .InitComparator(PlainComparator<GenericFunctionPointer>);

    ObjectTraitsSetup(kTypeIdObjectPointer, 
      PlainDeliveryImpl<GenericPointer>, 
      PlainHasher<GenericPointer>)
      .InitComparator(PlainComparator<GenericPointer>);

    EXPORT_CONSTANT(kTypeIdFunctionPointer);
    EXPORT_CONSTANT(kTypeIdObjectPointer);
  }
}