#include "management.h"

namespace sapphire::components {
  //New implementation for built-in function and types
  unordered_map<string, Object> &GetBuiltinComponentsObjBase() {
    static unordered_map<string, Object> base;
    return base;
  }

  void CreateFunctionObject(Function impl) {
    auto &base = GetBuiltinComponentsObjBase();
    base.try_emplace(impl.GetId(), Object(impl, kTypeIdFunction));
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

  AnnotatedAST *FindScriptByPath(string path) {
    AnnotatedAST result = nullptr;
    auto &storage = GetScriptStorage();
    auto it = storage.find(path);
    
    if (it != storage.end()) return &(it->second);

    return nullptr;
  }

  AnnotatedAST &AppendScript(string path, AnnotatedAST &code) {
    auto &storage = GetScriptStorage();
    ScriptStorage::iterator it;
    
    it = storage.find(path);

    if (it != storage.end()) return it->second;

    AnnotatedAST script;
    
    storage.insert(make_pair(path, code));
    it = storage.find(path);

    return it->second;
  }

  AnnotatedAST &AppendBlankScript(string path) {
    auto &storage = GetScriptStorage();
    ScriptStorage::iterator it;

    it = storage.find(path);

    if (it != storage.end()) return it->second;

    AnnotatedAST script;


    storage.insert(make_pair(path, AnnotatedAST()));
    it = storage.find(path);

    return it->second;
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