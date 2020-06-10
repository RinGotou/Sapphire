#include "machine.h"

namespace sapphire {
  template <typename StreamType>
  Message StreamFamilyState(ObjectMap &p) {
    StreamType &stream = p.Cast<StreamType>(kStrMe);
    return Message().SetObject(stream.Good());
  }

  ///////////////////////////////////////////////////////////////
  // InStream implementations
  Message NewInStream(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("path", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    string path = p.Cast<string>("path");

    shared_ptr<InStream> ifs = make_shared<InStream>(path);
    ifs->eof();

    return Message().SetObject(Object(ifs, kTypeIdInStream));
  }

  Message InStreamGet(ObjectMap &p) {
    InStream &ifs = p.Cast<InStream>(kStrMe);

    if (!ifs.Good()) {
      return Message("Invalid instream.", kStateError);
    }

    string result = ifs.GetLine();

    return Message().SetObject(result);
  }

  Message InStreamEOF(ObjectMap &p) {
    InStream &ifs = p.Cast<InStream>(kStrMe);
    return Message().SetObject(ifs.eof());
  }
  ///////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////
  // OutStream implementations
  Message NewOutStream(ObjectMap &p) {
    auto tc = TypeChecking(
      { 
        Expect("path", kTypeIdString),
        Expect("binary", kTypeIdBool),
        Expect("append", kTypeIdBool)
      }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    string path = p.Cast<string>("path");
    bool binary = p.Cast<bool>("binary");
    bool append = p.Cast<bool>("append");

    shared_ptr<OutStream> ofs = make_shared<OutStream>(path, append, binary);
    return Message().SetObject(Object(ofs, kTypeIdOutStream));
  }

  Message OutStreamWrite(ObjectMap &p) {
    OutStream &ofs = p.Cast<OutStream>(kStrMe);
    auto &obj = p["str"];
    bool result = true;

    if (obj.GetTypeId() == kTypeIdString) {
      string str = obj.Cast<string>();
      result = ofs.Write(str);
    }
    else {
      result = false;
    }

    return Message().SetObject(result);
  }
  ///////////////////////////////////////////////////////////////

  void InitStreamComponents() {
    using namespace management::type;

    ObjectTraitsSetup(kTypeIdInStream, ShallowDelivery, PointerHasher)
      .InitConstructor(
        FunctionImpl(NewInStream, "path", "instream")
      )
      .InitMethods(
        {
          FunctionImpl(InStreamGet, "", "get"),
          FunctionImpl(InStreamEOF, "", "eof"),
          FunctionImpl(StreamFamilyState<InStream>, "", "good"),
        }
    );

    ObjectTraitsSetup(kTypeIdOutStream, ShallowDelivery, PointerHasher)
      .InitConstructor(
        FunctionImpl(NewOutStream, "path|binary|append", "outstream")
      )
      .InitMethods(
        {
          FunctionImpl(OutStreamWrite, "str", "write"),
          FunctionImpl(StreamFamilyState<OutStream>, "", "good"),
        }
    );

    management::CreateConstantObject("kOutstreamModeAppend", Object("append"));
    management::CreateConstantObject("kOutstreamModeTruncate", Object("truncate"));

    EXPORT_CONSTANT(kTypeIdInStream);
    EXPORT_CONSTANT(kTypeIdOutStream);
  }
}