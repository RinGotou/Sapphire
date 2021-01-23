#include "containers.h"

namespace sapphire {
  //Message IteratorStepForward(ObjectMap &p) {
  //  auto &it = p[kStrMe].Cast<UnifiedIterator>();
  //  it.StepForward();
  //  return Message();
  //}

  //Message IteratorStepBack(ObjectMap &p) {
  //  auto &it = p[kStrMe].Cast<UnifiedIterator>();
  //  it.StepBack();
  //  return Message();
  //}

  //Message IteratorOperatorCompare(ObjectMap &p) {
  //  auto &rhs = p[kStrRightHandSide].Cast<UnifiedIterator>();
  //  auto &lhs = p[kStrMe].Cast<UnifiedIterator>();
  //  return Message().SetObject(lhs.Compare(rhs));
  //}

  //Message IteratorGet(ObjectMap &p) {
  //  auto &it = p[kStrMe].Cast<UnifiedIterator>();
  //  return Message().SetObject(it.Unpack());
  //}

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

  Message ArrayGetElement(ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrMe);
    auto &idx = p.Cast<int64_t>("index");
    size_t size = base.size();

    if (size_t(idx) >= size) return Message("Index is out of range", kStateError);
    if (size_t(idx) < 0) return Message("Index is out of range");

    return Message().SetObjectRef(base[size_t(idx)]);
  }

  Message ArrayGetSize(ObjectMap &p) {
    auto &obj = p[kStrMe];
    int64_t size = static_cast<int64_t>(obj.Cast<ObjectArray>().size());
    return Message().SetObject(Object(make_shared<int64_t>(size), kTypeIdInt));
  }

  Message ArrayEmpty(ObjectMap &p) {
    return Message().SetObject(p[kStrMe].Cast<ObjectArray>().empty());
  }

  Message ArrayPush(ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrMe);
    Object obj = components::DumpObject(p["object"]);
    base.emplace_back(obj);

    return Message();
  }

  Message ArrayPop(ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrMe);
    if (!base.empty()) base.pop_back();

    return Message().SetObject(base.empty());
  }

  //Message ArrayHead(ObjectMap &p) {
  //  auto &base = p[kStrMe].Cast<ObjectArray>();
  //  shared_ptr<UnifiedIterator> it = 
  //    make_shared<UnifiedIterator>(base.begin(), kContainerObjectArray);
  //  return Message().SetObject(Object(it, kTypeIdIterator));
  //}

  //Message ArrayTail(ObjectMap &p) {
  //  auto &base = p[kStrMe].Cast<ObjectArray>();
  //  shared_ptr<UnifiedIterator> it = 
  //    make_shared<UnifiedIterator>(base.end(), kContainerObjectArray);
  //  return Message().SetObject(Object(it, kTypeIdIterator));
  //}

  Message ArrayClear(ObjectMap &p) {
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

  Message PairLeft(ObjectMap &p) {
    auto &base = p.Cast<ObjectPair>(kStrMe);
    return Message().SetObject(Object().PackObject(base.first));
  }

  Message PairRight(ObjectMap &p) {
    auto &base = p.Cast<ObjectPair>(kStrMe);
    return Message().SetObject(Object().PackObject(base.second));
  }

  Message NewTable(ObjectMap &p) {
    ManagedTable table = make_shared<ObjectTable>();
    return Message().SetObject(Object(table, kTypeIdTable));
  }

  Message TableInsert(ObjectMap &p) {
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

  Message TableGetElement(ObjectMap &p) {
    // Ref: https://stackoverflow.com/questions/53149145/
    // Seems nothing to worry about pointers to element

    auto &table = p.Cast<ObjectTable>(kStrMe);
    auto &dest_key = p["key"];
    auto &result = table[dest_key];
    return Message().SetObjectRef(result);
  }

  Message TableFindElement(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    auto &key = p["key"];
    auto it = table.find(key);
    if (it != table.end()) {
      return Message().SetObjectRef(it->second);
      //return Message().SetObject(Object().PackObject(it->second));
    }
    return Message().SetObject(Object());
  }

  Message TableEraseElement(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    auto &key = p["key"];
    auto count = table.erase(key);
    return Message().SetObject(static_cast<int64_t>(count));
  }

  Message TableEmpty(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    return Message().SetObject(table.empty());
  }

  Message TableSize(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    return Message().SetObject(static_cast<int64_t>(table.size()));
  }

  Message TableClear(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    table.clear();
    return Message();
  }

  //Message TableHead(ObjectMap &p) {
  //  auto &table = p.Cast<ObjectTable>(kStrMe);
  //  shared_ptr<UnifiedIterator> it =
  //    make_shared<UnifiedIterator>(table.begin(), kContainerObjectTable);
  //  return Message().SetObject(Object(it, kTypeIdIterator));
  //}

  //Message TableTail(ObjectMap &p) {
  //  auto &table = p.Cast<ObjectTable>(kStrMe);
  //  shared_ptr<UnifiedIterator> it =
  //    make_shared<UnifiedIterator>(table.end(), kContainerObjectTable);
  //  return Message().SetObject(Object(it, kTypeIdIterator));
  //}

  void InitContainerComponents() {
    using namespace components;

    CreateStruct(kTypeIdArray);
    StructMethodGenerator(kTypeIdArray).Create(
      {
        Function(NewArray, "size|init_value", kStrInitializer, kParamAutoFill).SetLimit(0),
        Function(ArrayGetElement, "index", "at"),
        Function(ArrayGetSize, "", "size"),
        Function(ArrayPush, "object", "push"),
        Function(ArrayPop, "", "pop"),
        Function(ArrayEmpty, "", "empty"),
        //Function(ArrayHead, "", "head"),
        //Function(ArrayTail, "", "tail"),
        Function(ArrayClear, "", "clear")
      }
    );
    
    //todo:remove iterator type
    //CreateStruct(kTypeIdIterator);
    //StructMethodGenerator(kTypeIdIterator).Create(
    //  {
    //    Function(IteratorGet, "", "obj"),
    //    Function(IteratorStepForward, "", "step_forward"),
    //    Function(IteratorStepBack, "", "step_back"),
    //    Function(IteratorOperatorCompare, kStrRightHandSide, kStrCompare)
    //  }
    //);

    CreateStruct(kTypeIdPair);
    StructMethodGenerator(kTypeIdPair).Create(
      {
        //Function(NewPair, "left|right", kStrInitializer),
        Function(PairLeft, "", "left"),
        Function(PairRight, "", "right")
      }
    );

    CreateStruct(kTypeIdTable);
    StructMethodGenerator(kTypeIdTable).Create(
      {
        Function(NewTable, "", kStrInitializer),
        Function(TableInsert, "key|value", "insert"),
        Function(TableGetElement, "key", kStrAt),
        Function(TableFindElement, "key", "find"),
        Function(TableEraseElement, "key", "erase"),
        Function(TableEmpty, "", "empty"),
        Function(TableSize, "", "size"),
        Function(TableClear, "", "clear")
        //Function(TableHead, "", "head"),
        //Function(TableTail, "", "tail")
      }
    );

    EXPORT_CONSTANT(kTypeIdArray);
    //EXPORT_CONSTANT(kTypeIdIterator);
    EXPORT_CONSTANT(kTypeIdPair);
    EXPORT_CONSTANT(kTypeIdTable);
  }
}