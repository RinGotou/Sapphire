#include "machine.h"

namespace sapphire {
  int NewArray(State &state, ObjectMap &p) {
    ManagedArray base = make_shared<ObjectArray>();
    auto args = p.Cast<ObjectArray>("args");

    if (args.size() > 2) {
      state.SetMsg("Too much arguments for constructing array");
      return 2;
    }

    auto size = args[0].Cast<int64_t>();

    if (args.size() == 1) {
      for (size_t idx = 0; idx < size_t(size); idx += 1) {
        base->emplace_back(Object());
      }
    }
    else {
      ObjectView view(&args[1]);
      view.source = ObjectViewSource::Ref;

      for (size_t idx = 0; idx < size_t(size); idx += 1) {
        base->emplace_back(components::DumpObject(view.Seek()));
      }
    }

    state.PushValue(Object(base, kTypeIdArray));
    return 0;
  }

  int Array_GetElement(State &state, ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrMe);
    auto &idx = p.Cast<int64_t>("index");
    size_t size = base.size();

    if (size_t(idx) >= size || size_t(idx) < 0) {
      state.SetMsg("Index is out of range");
      return 2;
    }

    state.PushView(base[size_t(idx)]);
    return 0;
  }

  int Array_GetSize(State &state, ObjectMap &p) {
    auto &obj = p[kStrMe];
    int64_t size = static_cast<int64_t>(obj.Cast<ObjectArray>().size());
    state.PushValue(Object(size, kTypeIdInt));
    return 0;
  }

  int Array_Empty(State &state, ObjectMap &p) {
    state.PushValue(Object(p[kStrMe].Cast<ObjectArray>().empty(), kTypeIdBool));
    return 0;
  }

  int Array_Push(State &state, ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrMe);
    Object obj = components::DumpObject(p["object"]);
    base.emplace_back(obj);
    return 0;
  }

  int Array_Pop(State &state, ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrMe);
    if (!base.empty()) base.pop_back();
    state.PushValue(Object(base.empty(), kTypeIdBool));
    return 0;
  }

  int Array_Clear(State &state, ObjectMap &p) {
    auto &base = p.Cast<ObjectArray>(kStrMe);
    base.clear();
    base.shrink_to_fit();
    return 0;
  }

  int NewPair(State &state, ObjectMap &p) {
    auto &left = p["left"];
    auto &right = p["right"];
    ManagedPair pair = make_shared<ObjectPair>(
      components::DumpObject(left),
      components::DumpObject(right));
    state.PushValue(Object(pair, kTypeIdPair));
    return 0;
  }

  int Pair_Left(State &state, ObjectMap &p) {
    auto &base = p.Cast<ObjectPair>(kStrMe);
    state.PushView(base.first);
    return 0;
  }

  int Pair_Right(State &state, ObjectMap &p) {
    auto &base = p.Cast<ObjectPair>(kStrMe);
    state.PushView(base.second);
    return 0;
  }

  int NewTable(State &state, ObjectMap &p) {
    ManagedTable table = make_shared<ObjectTable>();
    state.PushValue(Object(table, kTypeIdTable));
    return 0;
  }

  int Table_Insert(State &state, ObjectMap &p) {
    using namespace components;
    auto &table = p.Cast<ObjectTable>(kStrMe);
    auto &key = p["key"];
    auto &value = p["value"];

    if (!lexical::IsPlainType(key.GetTypeId())) {
      state.SetMsg("Invalid key type");
      return 2;
    }

    auto result = table.insert(
      make_pair(DumpObject(key), DumpObject(value))
    );

    state.PushValue(Object(result.second, kTypeIdBool));
    return 0;
  }

  int Table_GetElement(State &state, ObjectMap &p) {
    // Ref: https://stackoverflow.com/questions/53149145/
    // Seems nothing to worry about pointers to element

    auto &table = p.Cast<ObjectTable>(kStrMe);
    auto &dest_key = p["key"];
    auto &result = table[dest_key];
    state.PushView(result);

    return 0;
  }

  int Table_FindElement(State &state, ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    auto &key = p["key"];
    auto it = table.find(key);

    if (it != table.end()) {
      state.PushView(it->second);
    }
    else {
      state.PushValue(Object());
    }

    return 0;
  }

  int Table_EraseElement(State &state, ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    auto &key = p["key"];
    auto count = table.erase(key);
    state.PushValue(Object(static_cast<int64_t>(count), kTypeIdInt));
    return 0;
  }

  int Table_Empty(State &state, ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    state.PushValue(Object(table.empty(), kTypeIdBool));
    return 0;
  }

  int Table_GetSize(State &state, ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    state.PushValue(Object(static_cast<int64_t>(table.size()), kTypeIdInt));
    return 0;
  }

  int Table_Clear(State &state, ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    table.clear();
    return 0;
  }

  void InitContainerComponents() {
    using namespace components;

    CreateStruct(kTypeIdArray);
    StructMethodGenerator(kTypeIdArray).Create(
      {
        Function(NewArray, "args", kStrInitializer, true),
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