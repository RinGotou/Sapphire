#include "machine.h"

namespace sapphire {
  Message NewArray(ObjectMap &p) {
    ManagedArray base = make_shared<ObjectArray>();

    if (!p["size"].NullPtr()) {
      auto size = p.Cast<int64_t>("size");
      if (size < 0) return Message("Invalid array size.", kStateError);
      auto size_value = static_cast<size_t>(size);


      Object obj = p["init_value"];

      auto type_id = obj.GetTypeId();

      for (size_t count = 0; count < size_value; ++count) {
        base->emplace_back(components::DumpObject(obj));
      }
    }

    return Message().SetObject(Object(base, kTypeIdArray));
  }

  Message Array_GetElement(ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrMe);
    auto &idx = p.Cast<int64_t>("index");
    size_t size = base.size();

    if (size_t(idx) >= size) return Message("Index is out of range", kStateError);
    if (size_t(idx) < 0) return Message("Index is out of range");

    return Message().SetObjectRef(base[size_t(idx)]);
  }

  Message Array_GetSize(ObjectMap &p) {
    auto &obj = p[kStrMe];
    int64_t size = static_cast<int64_t>(obj.Cast<ObjectArray>().size());
    return Message().SetObject(Object(make_shared<int64_t>(size), kTypeIdInt));
  }

  Message Array_Empty(ObjectMap &p) {
    return Message().SetObject(p[kStrMe].Cast<ObjectArray>().empty());
  }

  Message Array_Push(ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrMe);
    Object obj = components::DumpObject(p["object"]);
    base.emplace_back(obj);

    return Message();
  }

  Message Array_Pop(ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrMe);
    if (!base.empty()) base.pop_back();

    return Message().SetObject(base.empty());
  }

  Message Array_Clear(ObjectMap &p) {
    auto &base = p.Cast<ObjectArray>(kStrMe);
    base.clear();
    base.shrink_to_fit();
    return Message();
  }

  Message NewPair(ObjectMap &p) {
    auto &left = p["left"];
    auto &right = p["right"];
    ManagedPair pair = make_shared<ObjectPair>(
      components::DumpObject(left),
      components::DumpObject(right));
    return Message().SetObject(Object(pair, kTypeIdPair));
  }

  Message Pair_Left(ObjectMap &p) {
    auto &base = p.Cast<ObjectPair>(kStrMe);
    return Message().SetObject(Object().PackObject(base.first));
  }

  Message Pair_Right(ObjectMap &p) {
    auto &base = p.Cast<ObjectPair>(kStrMe);
    return Message().SetObject(Object().PackObject(base.second));
  }

  Message NewTable(ObjectMap &p) {
    ManagedTable table = make_shared<ObjectTable>();
    return Message().SetObject(Object(table, kTypeIdTable));
  }

  Message Table_Insert(ObjectMap &p) {
    using namespace components;
    auto &table = p.Cast<ObjectTable>(kStrMe);
    auto &key = p["key"];
    auto &value = p["value"];

    if (!lexical::IsPlainType(key.GetTypeId())) {
      return Message("Invalid key type", kStateError);
    }

    auto result = table.insert(
      make_pair(DumpObject(key), DumpObject(value))
    );
    return Message();
  }

  Message Table_GetElement(ObjectMap &p) {
    // Ref: https://stackoverflow.com/questions/53149145/
    // Seems nothing to worry about pointers to element

    auto &table = p.Cast<ObjectTable>(kStrMe);
    auto &dest_key = p["key"];
    auto &result = table[dest_key];
    return Message().SetObjectRef(result);
  }

  Message Table_FindElement(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    auto &key = p["key"];
    auto it = table.find(key);
    if (it != table.end()) {
      return Message().SetObjectRef(it->second);
      //return Message().SetObject(Object().PackObject(it->second));
    }
    return Message().SetObject(Object());
  }

  Message Table_EraseElement(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    auto &key = p["key"];
    auto count = table.erase(key);
    return Message().SetObject(static_cast<int64_t>(count));
  }

  Message Table_Empty(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    return Message().SetObject(table.empty());
  }

  Message Table_GetSize(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    return Message().SetObject(static_cast<int64_t>(table.size()));
  }

  Message Table_Clear(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    table.clear();
    return Message();
  }

  void InitContainerComponents() {
    using namespace components;

    CreateStruct(kTypeIdArray);
    StructMethodGenerator(kTypeIdArray).Create(
      {
        Function(NewArray, "size|init_value", kStrInitializer, kParamAutoFill).SetLimit(0),
        Function(Array_GetElement, "index", "at"),
        Function(Array_GetSize, "", "size"),
        Function(Array_Push, "object", "push"),
        Function(Array_Pop, "", "pop"),
        Function(Array_Empty, "", "empty"),
        Function(Array_Clear, "", "clear")
      }
    );

    CreateStruct(kTypeIdPair);
    StructMethodGenerator(kTypeIdPair).Create(
      {
        Function(Pair_Left, "", "left"),
        Function(Pair_Right, "", "right")
      }
    );

    CreateStruct(kTypeIdTable);
    StructMethodGenerator(kTypeIdTable).Create(
      {
        Function(NewTable, "", kStrInitializer),
        Function(Table_Insert, "key|value", "insert"),
        Function(Table_GetElement, "key", kStrAt),
        Function(Table_FindElement, "key", "find"),
        Function(Table_EraseElement, "key", "erase"),
        Function(Table_Empty, "", "empty"),
        Function(Table_GetSize, "", "size"),
        Function(Table_Clear, "", "clear")
      }
    );

    EXPORT_CONSTANT(kTypeIdArray);
    EXPORT_CONSTANT(kTypeIdPair);
    EXPORT_CONSTANT(kTypeIdTable);
  }
}