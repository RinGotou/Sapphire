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
    //auto tc = TypeChecking({ Expect("path", kTypeIdString) }, p);
    //if (TC_FAIL(tc)) return TC_ERROR(tc);

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
    //auto tc = TypeChecking(
    //  { 
    //    Expect("path", kTypeIdString),
    //    Expect("binary", kTypeIdBool),
    //    Expect("append", kTypeIdBool)
    //  }, p);
    //if (TC_FAIL(tc)) return TC_ERROR(tc);

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
    using namespace components;
    using namespace constant;
    
    CreateStruct(kTypeIdInStream);
    StructMethodGenerator(kTypeIdInStream).Create(
      {
        Function(NewInStream, "path", kStrInitializer),
        Function(InStreamGet, "", "get"),
        Function(InStreamEOF, "", "eof"),
        Function(StreamFamilyState<InStream>, "", "good")
      }
    );

    CreateStruct(kTypeIdOutStream);
    StructMethodGenerator(kTypeIdOutStream).Create(
      {
        Function(NewOutStream, "path|binary|append", kStrInitializer),
        Function(OutStreamWrite, "str", "write"),
        Function(StreamFamilyState<OutStream>, "", "good")
      }
    );

    CreateConstantObject("kOutstreamModeAppend", Object("append"));
    CreateConstantObject("kOutstreamModeTruncate", Object("truncate"));

    EXPORT_CONSTANT(kTypeIdInStream);
    EXPORT_CONSTANT(kTypeIdOutStream);
  }
}