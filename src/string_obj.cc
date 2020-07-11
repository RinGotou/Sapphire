#include "string_obj.h"

namespace sapphire {
  inline bool IsStringFamily(Object &obj) {
    return compare(obj.GetTypeId(), kTypeIdString, kTypeIdWideString);
  }

  Message CreateStringFromArray(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("src", kTypeIdArray) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &base = p.Cast<ObjectArray>("src");
    shared_ptr<string> dest(make_shared<string>());
    
    for (auto it = base.begin(); it != base.end(); ++it) {
      if (it->GetTypeId() == kTypeIdInt) {
        dest->append(1, static_cast<char>(it->Cast<int64_t>()));
        continue;
      }

      if (it->GetTypeId() != kTypeIdString) {
        continue;
      }

      dest->append(it->Cast<string>());
    }

    return Message().SetObject(Object(dest, kTypeIdString));
  }

  Message CharFromInt(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("value", kTypeIdInt) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto value = static_cast<char>(p.Cast<int64_t>("value"));
    return Message().SetObject(string().append(1, value));
  }

  Message IntFromChar(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("value", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &value = p.Cast<string>("value");

    if (value.size() != 1) {
      return Message("Invalid char", kStateError);
    }

    return Message().SetObject(static_cast<int64_t>(value[0]));
  }

  //String
  Message NewString(ObjectMap &p) {
    Object &obj = p["raw_string"];
    Object base;

    if (!IsStringFamily(obj)) {
      return Message("String constructor cannot accept non-string obejct.", kStateError);
    }

    if (obj.GetTypeId() == kTypeIdWideString) {
      wstring wstr = obj.Cast<wstring>();
      string output = ws2s(wstr);

      base.PackContent(make_shared<string>(output), kTypeIdString);
    }
    else if (obj.GetTypeId() == kTypeIdString) {
      string copy = obj.Cast<string>();
      base.PackContent(make_shared<string>(copy), kTypeIdString);
    }
    else {
      string output = obj.Cast<string>();

      base.PackContent(make_shared<string>(output), kTypeIdString);
    }

    return Message().SetObject(base);
  }

  Message StringCompare(ObjectMap &p) {
    auto &rhs = p[kStrRightHandSide];
    string lhs = p[kStrMe].Cast<string>();

    string type_id = rhs.GetTypeId();
    bool result = false;

    if (type_id == kTypeIdString) {
      string rhs_str = rhs.Cast<string>();
      result = (lhs == rhs_str);
    }

    return Message().SetObject(result);
  }

  Message StringToArray(ObjectMap &p) {
    auto &str = p.Cast<string>(kStrMe);
    shared_ptr<ObjectArray> base(make_shared<ObjectArray>());

    for (auto &unit : str) {
      base->emplace_back(string().append(1, unit));
    }

    return Message().SetObject(Object(base, kTypeIdArray));
  }

  //wstring
  Message NewWideString(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("raw_string", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    Object obj = p["raw_string"];

    string output = obj.Cast<string>();
    wstring wstr = s2ws(output);

    return Message()
      .SetObject(Object(make_shared<wstring>(wstr), kTypeIdWideString));
  }

  Message WideStringCompare(ObjectMap &p) {
    auto &rhs = p[kStrRightHandSide];
    wstring lhs = p[kStrMe].Cast<wstring>();
    bool result = false;
    if (rhs.GetTypeId() == kTypeIdWideString) {
      wstring rhs_wstr = rhs.Cast<wstring>();

      result = (lhs == rhs_wstr);
    }

    return Message().SetObject(result);
  }

  Message WideStringPrint(ObjectMap &p) {
    wstring &str = p.Cast<wstring>(kStrMe);
    OutStreamW(VM_STDOUT).Write(str);
    CHECK_PRINT_OPT(p);
    return Message();
  }

  void InitBaseTypes() {
    using namespace components;
    using components::CreateFunctionObject;

    CreateStruct(kTypeIdString);
    StructMethodGenerator(kTypeIdString).Create(
      {
        Function(NewString, "raw_string", kStrInitializer),
        Function(StringFamilyGetElement<string>, "index", "at"),
        Function(StringFamilySubStr<string>, "start|size", "substr"),
        Function(GetStringFamilySize<string>, "", "size"),
        Function(StringFamilyConverting<wstring, string>, "", "to_wide"),
        Function(StringCompare, kStrRightHandSide, kStrCompare),
        Function(StringToArray, "","to_array")
      }
    );

    CreateStruct(kTypeIdWideString);
    StructMethodGenerator(kTypeIdWideString).Create(
      {
        Function(NewWideString, "raw_string", kStrInitializer),
        Function(GetStringFamilySize<wstring>,  "", "size"),
        Function(StringFamilyGetElement<wstring>, "index", kStrAt),
        Function(WideStringPrint, "", "print"),
        Function(StringFamilySubStr<wstring>, "start|size", "substr"),
        Function(StringFamilyConverting<string, wstring>, "", "to_byte"),
        Function(WideStringCompare, kStrRightHandSide, kStrCompare)
      }
    );

    CreateFunctionObject(Function(DecimalConvert<2>, "str", "bin"));
    CreateFunctionObject(Function(DecimalConvert<8>, "str", "octa"));
    CreateFunctionObject(Function(DecimalConvert<16>, "str", "hex"));
    CreateFunctionObject(Function(CreateStringFromArray, "src", "ar2string"));
    CreateFunctionObject(Function(CharFromInt, "value", "int2str"));
    CreateFunctionObject(Function(IntFromChar, "value", "str2int"));

    EXPORT_CONSTANT(kTypeIdString);
    EXPORT_CONSTANT(kTypeIdWideString);
  }
}