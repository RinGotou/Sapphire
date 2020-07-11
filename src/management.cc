#include "management.h"

namespace sapphire::components {
  //New implementation for built-in function and types
  unordered_map<string, Object> &GetBuiltinComponentsObjBase() {
    static unordered_map<string, Object> base;
    return base;
  }

  void CreateFunctionObject(Function impl) {
    auto &base = GetBuiltinComponentsObjBase();
    base.try_emplace(impl.GetId(), Object(std::move(impl), kTypeIdFunction));
  }

  //We don't need FindFunction() in new implementation.
  //Every machine will copy these info into their own base scope
  
  //Functions that are deprecated:
  //  GetMethods() CheckMethod()

  void CreateStruct(string id) {
    auto &base = GetBuiltinComponentsObjBase();
    auto obj_struct = make_shared<ObjectStruct>();
    auto result = base.try_emplace(id, Object(obj_struct, kTypeIdStruct));
    auto &struct_base = result.first->second.Cast<ObjectStruct>();
    struct_base.Add(kStrStructId, Object(id));
  }

  bool StructMethodGenerator::Create(initializer_list<Function> &&impls) {
    auto &base = GetBuiltinComponentsObjBase();
    auto it = base.find(id_);

    if (it == base.end()) return false;

    auto &struct_base = it->second.Cast<ObjectStruct>();
    
    for (auto &unit : impls) {
      struct_base.Add(unit.GetId(), Object(make_shared<Function>(unit), kTypeIdFunction));
    }

    return true;
  }

  void DumpObject(ObjectView source, ObjectView dest) {
    if (!lexical::IsPlainType(source.Seek().GetTypeId())) {
      dest.Seek() = source.Seek();
      return;
    }

    auto type_id = source.Seek().GetTypeId();

#define DUMP_VALUE(_Type, _Id)                                \
    auto &value = source.Seek().Cast<_Type>();                \
    dest.Seek().PackContent(make_shared<_Type>(value), _Id);

    if (type_id == kTypeIdInt) {
      DUMP_VALUE(int64_t, kTypeIdInt)
    }
    else if (type_id == kTypeIdFloat) {
      DUMP_VALUE(double, kTypeIdFloat)
    }
    else if (type_id == kTypeIdString) {
      DUMP_VALUE(string, kTypeIdString)
    }
    else if (type_id == kTypeIdBool) {
      DUMP_VALUE(bool, kTypeIdBool)
    }
#undef DUMP_VALUE
  }

  Object DumpObject(Object &source) {
    Object result;
    DumpObject(ObjectView(&source), ObjectView(&result));
    return result;
  }
}

namespace sapphire::constant {
  auto &GetConstantBase() {
    static ObjectContainer base;
    return base;
  }

  static mutex constant_creation_gate;

  Object *CreateConstantObject(string id, Object &object) {
    lock_guard<mutex> guard(constant_creation_gate);
    ObjectContainer &base = GetConstantBase();

    if (base.Find(id) != nullptr) return nullptr;

    base.Add(id, object);
    auto result = base.Find(id);
    return result;
  }

  Object *CreateConstantObject(string id, Object &&object) {
    lock_guard<mutex> guard(constant_creation_gate);
    ObjectContainer &base = GetConstantBase();

    if (base.Find(id) != nullptr) return nullptr;

    base.Add(id, std::move(object));
    auto result = base.Find(id);
    return result;
  }

  Object *GetConstantObject(string &id) {
    ObjectContainer &base = GetConstantBase();
    auto ptr = base.Find(id);
    return ptr;
  }
}

namespace sapphire::script {
  mutex script_storage_gate;

  auto &GetScriptStorage() {
    static ScriptStorage storage;
    lock_guard<mutex> guard(script_storage_gate);
    return storage;
  }

  VMCode *FindScriptByPath(string path) {
    VMCode result = nullptr;
    auto &storage = GetScriptStorage();
    auto it = storage.find(path);
    
    if (it != storage.end()) return &(it->second);

    return nullptr;
  }

  VMCode &AppendScript(string path, VMCode &code) {
    auto &storage = GetScriptStorage();
    ScriptStorage::iterator it;
    
    it = storage.find(path);

    if (it != storage.end()) return it->second;

    VMCode script;
    
    storage.insert(make_pair(path, code));
    it = storage.find(path);

    return it->second;
  }

  VMCode &AppendBlankScript(string path) {
    auto &storage = GetScriptStorage();
    ScriptStorage::iterator it;

    it = storage.find(path);

    if (it != storage.end()) return it->second;

    VMCode script;


    storage.insert(make_pair(path, VMCode()));
    it = storage.find(path);

    return it->second;
  }
}

namespace sapphire::extension {
  template <ObjectType _TypeCode>
  struct _Deleter {
    _Deleter(void *ptr) {
      if constexpr (_IsStringObject(_TypeCode)) {
        delete[] (typename _CharTypeTraitS<_TypeCode>::Type *)ptr;
      }
      else {
        delete (typename _ObjectTypeTrait<_TypeCode>::Type *)ptr;
      }
    }
  };

  void DisposeMemoryUnit(void *ptr, int type) {
    switch (type) {
    case kExtTypeInt:_Deleter<kExtTypeInt>((void *)ptr); break;
    case kExtTypeFloat:_Deleter<kExtTypeFloat>((void *)ptr); break;
    case kExtTypeBool:_Deleter<kExtTypeBool>((void *)ptr); break;
    case kExtTypeFunctionPointer:
      _Deleter<kExtTypeFunctionPointer>((void *)ptr); break;
    case kExtTypeObjectPointer:
      _Deleter<kExtTypeObjectPointer>((void *)ptr); break;
    default:break;
    }
  }

  inline ObjectType MatchExtType(string type_id) {
    auto result = kExtUnsupported;
    auto it = kExtTypeMatcher.find(type_id);
    if (it != kExtTypeMatcher.end()) result = it->second;
    return result;
  }

  int FetchDescriptor(Descriptor *descriptor, void *obj_map, const char *id) {
    auto *source = static_cast<ObjectMap *>(obj_map);
    auto it = source->find(string(id));
    if (it == source->end()) return 0;
    *descriptor = Descriptor{ &it->second, MatchExtType(it->second.GetTypeId()) };
    return 1;
  }

  //for variable argument pattern
  // -1 : type mismatch
  // 0 : index out of range
  int FetchArrayElementDescriptor(Descriptor *arr_desc, Descriptor *dest, size_t index) {
    if (arr_desc->type != kExtTypeArray) return -1;
    auto &arr_obj = *static_cast<Object *>(arr_desc->ptr);
    auto &arr = arr_obj.Cast<ObjectArray>();
    if (index >= arr.size()) return 0;
    void *elem_ptr = &arr[index];
    *dest = Descriptor{ elem_ptr, MatchExtType(arr[index].GetTypeId()) };
    return 1;
  }

  size_t GetArrayObjectCapacity(Descriptor desc) {
    if (desc.type != kExtTypeArray) return 0;
    auto &arr = *static_cast<Object *>(desc.ptr);
    return arr.Cast<ObjectArray>().size();
  }

  template <typename _Type>
  void _DumpObjectA(Object &obj, void **dest) {
    auto &value = obj.Cast<_Type>();
    *dest = new _Type(value);
  }

  template <typename _CharType>
  using _StrCpyFunc = _CharType * (*)(_CharType *, const _CharType *);

  template <typename _CharType>
  struct _StrCpySelector{ 
    _StrCpyFunc<_CharType> func;
  };

  template <>
  struct _StrCpySelector<char> {
    _StrCpyFunc<char> func;
    _StrCpySelector() : func(std::strcpy) {}
  };

  template <>
  struct _StrCpySelector<wchar_t> {
    _StrCpyFunc<wchar_t> func;
    _StrCpySelector() : func(std::wcscpy) {}
  };

  template <typename _CharType, typename _StringType>
  void _DumpObjectS(Object &obj, void **dest) {
    auto &value = obj.Cast<_StringType>();
    *dest = new _CharType[value.size() + 1];
    _StrCpySelector<_CharType>().func((_CharType *)*dest, value.data());
  }

  //Old fetching facilities will be deprecated in the future
  // 0 : type mismatch
  // -1 : unsupported
  int DumpObjectFromDescriptor(Descriptor *descriptor, void **dest) {
    auto &obj = *static_cast<ObjectPointer>(descriptor->ptr);

    if (descriptor->type == kExtUnsupported) return -1;
    if (descriptor->type != MatchExtType(obj.GetTypeId())) return 0;

    auto is_string = [](auto type) -> bool {
      return type == kExtTypeString || type == kExtTypeWideString; };

    if (is_string(descriptor->type)) {
      switch (descriptor->type) {
      case kExtTypeString:_DumpObjectS<char, string>(obj, dest); break;
      case kExtTypeWideString:_DumpObjectS<wchar_t, wstring>(obj, dest); break;
      default:break;
      }
    }
    else {
      switch (descriptor->type) {
      case kExtTypeInt:_DumpObjectA<int64_t>(obj, dest); break;
      case kExtTypeFloat:_DumpObjectA<double>(obj, dest); break;
      case kExtTypeBool:_DumpObjectA<int>(obj, dest); break;
      case kExtTypeFunctionPointer:
        _DumpObjectA<GenericFunctionPointer>(obj, dest);break;
      case kExtTypeObjectPointer:_DumpObjectA<GenericPointer>(obj, dest); break;
      default:break;
      }
    }

    return 1;
  }

  int FetchCustomTypes(void **target, void *obj_map, const char *id) {
    auto *source = static_cast<ObjectMap *>(obj_map);
    auto it = source->find(string(id));
    if (it == source->end()) return 0;
    if (it->second.GetMode() != kObjectExternal) return -1;
    *target = it->second.GetExternalPointer();
    return 1;
  }

  int FetchObjectType(void *obj_map, const char *id) {
    auto &p = *static_cast<ObjectMap *>(obj_map);
    auto it = p.find(string(id));
    int result = kExtTypeNull;

    if (it != p.end()) {
      auto type = it->second.GetTypeId();
      if (type == kTypeIdInt) result = kExtTypeInt;
      else if (type == kTypeIdFloat) result = kExtTypeFloat;
      else if (type == kTypeIdBool) result = kExtTypeBool;
      else if (type == kTypeIdString) result = kExtTypeString;
      else if (type == kTypeIdWideString) result = kExtTypeWideString;
      //TODO:
    }

    return result;
  }
}

namespace sapphire::runtime {
  static string binary_name;
  static string binary_path;
  static string script_work_dir;
  static fs::path script_absolute_path;

  void InformBinaryPathAndName(string info) {
    fs::path processed_path(info);
    binary_name = processed_path.filename().string();
    binary_path = processed_path.parent_path().string();
  }

  string GetBinaryPath() { return binary_path; }
  string GetBinaryName() { return binary_name; }

  string GetWorkingDirectory() {
#ifdef _WIN32
    //using recommended implementation from Microsoft's document
    auto *buffer = _getcwd(nullptr, 0);
#else
    auto *buffer = getcwd(nullptr, 0);
#endif

    if (buffer == nullptr) return "";

    string result(buffer);
    free(buffer);
    return result;
  }

  bool SetWorkingDirectory(string dir) {
#ifdef _WIN32
    int ret = _chdir(dir.data());
#else
    int ret = chdir(dir.data());
#endif

    return ret == 0;
  }

  void InformScriptPath(string path) {
    script_absolute_path = fs::absolute(fs::path(path)).parent_path();
  }

  string GetScriptAbsolutePath() {
    return script_absolute_path.string();
  }
}