#include "machine.h"

namespace sapphire {
  //We didn't provide regular constructor for string
  //Don't use old type checking function. Write something to replace that
  Message String_GetElement(ObjectMap &p) {
    auto index = p.Cast<int64_t>("index");
    auto &me = p.Cast<string>(kStrMe);

    if (index < 0 || index >= static_cast<int64_t>(me.size())) {
      return Message("Invalid index for this string", StateLevel::Error);
    }

    auto elem = make_shared<string>();
    elem->append(1, me[index]);
    return Message().SetObject(Object(elem, kTypeIdString));
  }

  Message String_SubStr(ObjectMap &p) {
    auto begin = p.Cast<int64_t>("begin");
    auto size = p.Cast<int64_t>("size");
    auto &me = p.Cast<string>(kStrMe);

    if (begin < 0 || begin >= static_cast<int64_t>(me.size())) {
      return Message("Invalid begin index for this string", StateLevel::Error);
    }

    if (size > (static_cast<int64_t>(me.size()) - begin) || size < 0) {
      return Message("Invalid size for this string", StateLevel::Error);
    }

    auto elem = me.substr(begin, size);
    return Message().SetObject(Object(elem, kTypeIdString));
  }

  Message String_GetSize(ObjectMap &p) {
    auto &me = p.Cast<string>(kStrMe);
    auto size = static_cast<int64_t>(me.size());
    return Message().SetObject(Object(size, kTypeIdInt));
  }

  Message String_ToWide(ObjectMap &p) {
    auto wstr = s2ws(p.Cast<string>(kStrMe));
    return Message().SetObject(Object(wstr, kTypeIdWideString));
  }

  Message String_Compare(ObjectMap &p) {
    auto &rhs_obj = p[kStrRightHandSide];
    auto &me = p.Cast<string>(kStrMe);

    if (rhs_obj.GetTypeId() != kTypeIdString) {
      return Message().SetObject(false);
    }

    return Message().SetObject(me == rhs_obj.Cast<string>());
  }
  
  Message NewWideString(ObjectMap &p) {
    auto &src = p.Cast<string>("src");
    auto me = make_shared<wstring>(s2ws(src));
    return Message().SetObject(Object(me, kTypeIdWideString));
  }

  Message WString_GetElement(ObjectMap &p) {
    auto index = p.Cast<int64_t>("index");
    auto &me = p.Cast<wstring>(kStrMe);

    if (index < 0 || index >= static_cast<int64_t>(me.size())) {
      return Message("Invalid index for this string", StateLevel::Error);
    }

    auto elem = make_shared<wstring>();
    elem->append(1, me[index]);
    return Message().SetObject(Object(elem, kTypeIdWideString));
  }

  Message WString_SubStr(ObjectMap &p) {
    auto begin = p.Cast<int64_t>("begin");
    auto size = p.Cast<int64_t>("size");
    auto &me = p.Cast<wstring>(kStrMe);

    if (begin < 0 || begin >= static_cast<int64_t>(me.size())) {
      return Message("Invalid begin index for this string", StateLevel::Error);
    }

    if (size > (static_cast<int64_t>(me.size()) - begin) || size < 0) {
      return Message("Invalid size for this string", StateLevel::Error);
    }

    auto elem = me.substr(begin, size);
    return Message().SetObject(Object(elem, kTypeIdWideString));
  }

  Message WString_GetSize(ObjectMap &p) {
    auto &me = p.Cast<wstring>(kStrMe);
    auto size = static_cast<int64_t>(me.size());
    return Message().SetObject(Object(size, kTypeIdInt));
  }

  Message WString_ToBytes(ObjectMap &p) {
    auto str = ws2s(p.Cast<wstring>(kStrMe));
    return Message().SetObject(Object(str, kTypeIdString));
  }

  Message WString_Compare(ObjectMap &p) {
    auto &rhs_obj = p[kStrRightHandSide];
    auto &me = p.Cast<wstring>(kStrMe);

    if (rhs_obj.GetTypeId() != kTypeIdWideString) {
      return Message().SetObject(false);
    }

    return Message().SetObject(me == rhs_obj.Cast<wstring>());
  }

  void InitStringTypes() {
    using namespace components;
    using components::CreateFunctionObject;

    CreateStruct(kTypeIdString);
    StructMethodGenerator(kTypeIdString).Create(
      {
        Function(String_GetElement, "index", "at"),
        Function(String_SubStr, "begin|size", "substr"),
        Function(String_GetSize, "", "size"),
        Function(String_ToWide, "", "to_wide"),
        Function(String_Compare, kStrRightHandSide, kStrCompare)
      }
    );

    CreateStruct(kTypeIdWideString);
    StructMethodGenerator(kTypeIdWideString).Create(
      {
        Function(NewWideString, "src", kStrInitializer),
        Function(WString_GetElement, "index", "at"),
        Function(WString_SubStr, "begin|size", "substr"),
        Function(WString_GetSize, "", "size"),
        Function(WString_ToBytes, "", "to_bytes"),
        Function(WString_Compare, kStrRightHandSide, kStrCompare)
      }
    );
  }

  template <int base>
  Message DecimalConvert(ObjectMap &p) {
    string str = ParseRawString(p["str"].Cast<string>());

    int64_t dest = stol(str, nullptr, base);
    return Message().SetObject(Object(
      make_shared<int64_t>(dest), kTypeIdInt
    ));
  }

  Message ConvertStringToInt(ObjectMap &p) {
    auto value = static_cast<char>(p.Cast<int64_t>("value"));
    return Message().SetObject(string().append(1, value));
  }

  Message ConvertIntToString(ObjectMap &p) {
    auto &value = p.Cast<string>("value");

    if (value.size() != 1) {
      return Message("Invalid char", StateLevel::Error);
    }

    return Message().SetObject(static_cast<int64_t>(value[0]));
  }

  void InitStringComponents() {
    using namespace components;
    using components::CreateFunctionObject;

    CreateFunctionObject(Function(DecimalConvert<2>, "str", "bin"));
    CreateFunctionObject(Function(DecimalConvert<8>, "str", "octa"));
    CreateFunctionObject(Function(DecimalConvert<16>, "str", "hex"));
    CreateFunctionObject(Function(ConvertStringToInt, "value", "s2int"));
    CreateFunctionObject(Function(ConvertIntToString, "value", "i2str"));
  }
}