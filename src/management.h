#pragma once
#include "function.h"
#include "extension.h"
#include "filestream.h"

namespace sapphire::components {
  unordered_map<string, Object> &GetBuiltinComponentsObjBase();

  void CreateFunctionObject(Function impl);
  void CreateStruct(string id);
  void DumpObject(ObjectView source, ObjectView dest);
  Object DumpObject(Object &source);

  class StructMethodGenerator {
  protected:
    string id_;
  public:
    StructMethodGenerator() = delete;
    StructMethodGenerator(StructMethodGenerator &) = delete;
    StructMethodGenerator(StructMethodGenerator &&) = delete;
    StructMethodGenerator(string id) : id_(id) {}
    bool Create(initializer_list<Function> &&impls);
  };

  //lexical::IsPlainType() is existed inside lexical.h

  //Dump plain object
  void DumpObject(ObjectView source, ObjectView dest);

  struct PlainTypeHash {
    size_t operator()(Object const &rhs) const {
      auto copy = rhs; //bypass
      size_t value = 0;
      auto type_id = rhs.GetTypeId();
      
#define GET_HASH(_Type) value = std::hash<_Type>()(copy.Cast<_Type>())

      if (type_id == kTypeIdInt) GET_HASH(int64_t);
      else if (type_id == kTypeIdFloat) GET_HASH(double);
      else if (type_id == kTypeIdString) GET_HASH(string);
      else if (type_id == kTypeIdBool) GET_HASH(bool);

      return value;
#undef GET_HASH
    }
  };

  struct PlainTypeComparaion {
    bool operator()(Object const &lhs, Object const &rhs) const {
      //bypass
      auto copy_lhs = lhs, copy_rhs = rhs;
      bool result = false;
      if (lhs.GetTypeId() != rhs.GetTypeId()) return result;
#define COMPARE(_Type) (copy_lhs.Cast<_Type>() == copy_rhs.Cast<_Type>())
      auto type_id = lhs.GetTypeId();
      if (type_id == kTypeIdInt) result = COMPARE(int64_t);
      else if (type_id == kTypeIdFloat) result = COMPARE(double);
      else if (type_id == kTypeIdString) result = COMPARE(string);
      else if (type_id == kTypeIdBool) result = COMPARE(bool);
      return result;
#undef COMPARE
    }
  };
}

namespace sapphire::constant {
  Object *CreateConstantObject(string id, Object &object);
  Object *CreateConstantObject(string id, Object &&object);
  Object *GetConstantObject(string &id);
}

namespace sapphire::script {
  using ProcessedScript = pair<string, VMCode>;
  using ScriptStorage = unordered_map<string, VMCode>;

  VMCode *FindScriptByPath(string path);
  VMCode &AppendScript(string path, VMCode &code);
  VMCode &AppendBlankScript(string path);
}

namespace sapphire::extension {
  void DisposeMemoryUnit(void *ptr, int type);
  int FetchDescriptor(Descriptor *descriptor, void *obj_map, const char *id);
  int FetchArrayElementDescriptor(Descriptor *arr_desc, Descriptor *dest, size_t index);
  size_t GetArrayObjectCapacity(Descriptor desc);
  int DumpObjectFromDescriptor(Descriptor *descriptor, void **dest);
  int FetchObjectType(void *obj_map, const char *id);
}

namespace sapphire::runtime {
  void InformBinaryPathAndName(string info);
  string GetBinaryPath();
  string GetBinaryName();
  string GetWorkingDirectory();
  bool SetWorkingDirectory(string dir);
  void InformScriptPath(string path);
  string GetScriptAbsolutePath();
}

namespace sapphire {
  using ObjectTable = unordered_map<Object, Object, 
    components::PlainTypeHash, components::PlainTypeComparaion>;
  using ManagedTable = shared_ptr<ObjectTable>;
}

#define EXPORT_CONSTANT(ID) constant::CreateConstantObject(#ID, Object(ID))


