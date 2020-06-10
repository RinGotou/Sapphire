#include "function.h"

namespace sapphire {
  Message MakeInvokePoint(string id, string type_id, Object obj) {
    vector<string> arg = { id,type_id };
    return Message(CombineStringVector(arg)).SetInvokingSign(obj);
  }
}