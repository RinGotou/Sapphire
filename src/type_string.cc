#include "machine.h"

namespace sapphire {
  //We didn't provide regular constructor for string
  //Don't use old type checking function. Write something to replace that
  int String_GetElement(State &state, ObjectMap &p) {
    auto index = p.Cast<int64_t>("index");
    auto &me = p.Cast<string>(kStrMe);

    if (index < 0 || index >= static_cast<int64_t>(me.size())) {
      state.SetMsg("Index is out of range");
      return 2;
    }

    auto elem = make_shared<string>();
    elem->append(1, me[index]);
    state.PushValue(Object(elem, kTypeIdString));
    return 0;
  }

  int String_SubStr(State &state, ObjectMap &p) {
    auto begin = p.Cast<int64_t>("begin");
    auto size = p.Cast<int64_t>("size");
    auto &me = p.Cast<string>(kStrMe);

    if (begin < 0 || begin >= static_cast<int64_t>(me.size())) {
      state.SetMsg("Invalid begin index");
      return 2;
    }

    if (size > (static_cast<int64_t>(me.size()) - begin) || size < 0) {
      state.SetMsg("Invalid size");
      return 2;
    }

    auto elem = me.substr(begin, size);
    state.PushValue(Object(elem, kTypeIdString));
    return 0;
  }

  int String_GetSize(State &state, ObjectMap &p) {
    auto &me = p.Cast<string>(kStrMe);
    auto size = static_cast<int64_t>(me.size());
    state.PushValue(Object(size, kTypeIdInt));
    return 0;
  }

  int String_ToWide(State &state, ObjectMap &p) {
    auto wstr = s2ws(p.Cast<string>(kStrMe));
    state.PushValue(Object(wstr, kTypeIdWideString));
    return 0;
  }

  int String_Compare(State &state, ObjectMap &p) {
    auto &rhs_obj = p[kStrRightHandSide];
    auto &me = p.Cast<string>(kStrMe);

    if (rhs_obj.GetTypeId() != kTypeIdString) {
      state.PushValue(Object(false, kTypeIdBool));
      return 0;
    }

    state.PushValue(Object(me == rhs_obj.Cast<string>(), kTypeIdBool));
    return 0;
  }
  
  int NewWideString(State &state, ObjectMap &p) {
    auto &src = p.Cast<string>("src");
    auto me = make_shared<wstring>(s2ws(src));
    state.PushValue(Object(me, kTypeIdWideString));
    return 0;
  }

  int WString_GetElement(State &state, ObjectMap &p) {
    auto index = p.Cast<int64_t>("index");
    auto &me = p.Cast<wstring>(kStrMe);

    if (index < 0 || index >= static_cast<int64_t>(me.size())) {
      state.SetMsg("Invalid syntax");
      return 2;
    }

    auto elem = make_shared<wstring>();
    elem->append(1, me[index]);
    state.PushValue(Object(elem, kTypeIdWideString));
    return 0;
  }

  int WString_SubStr(State &state, ObjectMap &p) {
    auto begin = p.Cast<int64_t>("begin");
    auto size = p.Cast<int64_t>("size");
    auto &me = p.Cast<wstring>(kStrMe);

    if (begin < 0 || begin >= static_cast<int64_t>(me.size())) {
      state.SetMsg("Invalid begin index");
      return 2;
    }

    if (size > (static_cast<int64_t>(me.size()) - begin) || size < 0) {
      state.SetMsg("Invalid size");
      return 2;
    }

    auto elem = me.substr(begin, size);
    state.PushValue(Object(elem, kTypeIdWideString));
    return 0;
  }

  int WString_GetSize(State &state, ObjectMap &p) {
    auto &me = p.Cast<wstring>(kStrMe);
    auto size = static_cast<int64_t>(me.size());
    state.PushValue(Object(size, kTypeIdInt));
    return 0;
  }

  int WString_ToBytes(State &state, ObjectMap &p) {
    auto str = ws2s(p.Cast<wstring>(kStrMe));
    state.PushValue(Object(str, kTypeIdString));
    return 0;
  }

  int WString_Compare(State &state, ObjectMap &p) {
    auto &rhs_obj = p[kStrRightHandSide];
    auto &me = p.Cast<wstring>(kStrMe);

    if (rhs_obj.GetTypeId() != kTypeIdWideString) {
      state.PushValue(Object(false, kTypeIdBool));
      return 0;
    }

    state.PushValue(Object(me == rhs_obj.Cast<wstring>(), kTypeIdBool));
    return 0;
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
  int DecimalConvert(State &state, ObjectMap &p) {
    string str = ParseRawString(p["str"].Cast<string>());

    int64_t dest = stol(str, nullptr, base);
    state.PushValue(Object(dest, kTypeIdInt));
    return 0;
  }

  int ConvertCharToInt(State &state, ObjectMap &p) {
    auto &value = p.Cast<string>("value");

    if (value.size() != 1) {
      state.SetMsg("Function can only convert single-char string");
      return 2;
    }

    state.PushValue(Object(static_cast<int64_t>(value[0]), kTypeIdString));
    return 0;
  }

  int ConvertIntToChar(State &state, ObjectMap &p) {
    auto value = static_cast<char>(p.Cast<int64_t>("value"));
    state.PushValue(Object(string().append(1, value), kTypeIdString));
    return 0;

  }

  void InitStringComponents() {
    using namespace components;
    using components::CreateFunctionObject;

    CreateFunctionObject(Function(DecimalConvert<2>, "str", "bin"));
    CreateFunctionObject(Function(DecimalConvert<8>, "str", "octa"));
    CreateFunctionObject(Function(DecimalConvert<16>, "str", "hex"));
    CreateFunctionObject(Function(ConvertCharToInt, "value", "c2int"));
    CreateFunctionObject(Function(ConvertIntToChar, "value", "i2char"));
  }
}