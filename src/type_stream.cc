#include "machine.h"

namespace sapphire {
  template <typename StreamType>
  int StreamFamilyState(State &state, ObjectMap &p) {
    StreamType &stream = p.Cast<StreamType>(kStrMe);
    state.PushValue(Object(stream.Good(), kTypeIdBool));
    return 0;
  }

  int NewInStream(State &state, ObjectMap &p) {
    string path = p.Cast<string>("path");

    shared_ptr<InStream> ifs = make_shared<InStream>(path);
    state.PushValue(Object(ifs, kTypeIdInStream));

    return 0;
  }

  int InStream_Get(State &state, ObjectMap &p) {
    InStream &ifs = p.Cast<InStream>(kStrMe);

    if (!ifs.Good()) {
      state.SetMsg("Invalid in-stream");
      return 2;
    }

    string result = ifs.GetLine();

    state.PushValue(Object(result, kTypeIdString));
    return 0;
  }

  int InStream_EOF(State &state, ObjectMap &p) {
    InStream &ifs = p.Cast<InStream>(kStrMe);
    state.PushValue(Object(ifs.eof(), kTypeIdBool));
    return 0;
  }

  int NewOutStream(State &state, ObjectMap &p) {
    string path = p.Cast<string>("path");
    bool binary = p.Cast<bool>("binary");
    bool append = p.Cast<bool>("append");

    shared_ptr<OutStream> ofs = make_shared<OutStream>(path, append, binary);
    state.PushValue(Object(ofs, kTypeIdOutStream));
    return 0;
  }

  int OutStream_Write(State &state, ObjectMap &p) {
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

    state.PushValue(Object(result, kTypeIdBool));
    return 0;
  }

  void InitStreamComponents() {
    using namespace components;
    using namespace constant;
    
    CreateStruct(kTypeIdInStream);
    StructMethodGenerator(kTypeIdInStream).Create(
      {
        Function(NewInStream, "path", kStrInitializer),
        Function(InStream_Get, "", "get"),
        Function(InStream_EOF, "", "eof"),
        Function(StreamFamilyState<InStream>, "", "good")
      }
    );

    CreateStruct(kTypeIdOutStream);
    StructMethodGenerator(kTypeIdOutStream).Create(
      {
        Function(NewOutStream, "path|binary|append", kStrInitializer),
        Function(OutStream_Write, "str", "write"),
        Function(StreamFamilyState<OutStream>, "", "good")
      }
    );

    CreateConstantObject("kOutstreamModeAppend", Object("append"));
    CreateConstantObject("kOutstreamModeTruncate", Object("truncate"));

    EXPORT_CONSTANT(kTypeIdInStream);
    EXPORT_CONSTANT(kTypeIdOutStream);
  }
}