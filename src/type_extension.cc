#include "machine.h"

namespace sapphire {
  Message GetFunctionPointer(ObjectMap &p) {
    //auto tc = TypeChecking(
    //  {
    //    Expect("library", kTypeIdString),
    //    Expect("id", kTypeIdString)
    //  }, p);
    //if (TC_FAIL(tc)) return TC_ERROR(tc);

#ifdef _WIN32
    wstring path = s2ws(p.Cast<string>("library"));
    string id = p.Cast<string>("id");
    HMODULE mod = LoadLibraryW(path.data());

    if (mod == nullptr) return Message().SetObject(int64_t(0));

    auto func = GenericFunctionPointer(GetProcAddress(mod, id.data()));
#else
    string path = p.Cast<string>("library");
    string id = p.Cast<string>("id");
    void *mod = dlopen(path.data(), RTLD_LAZY);

    if (mod == nullptr) return Message().SetObject(int64_t(0));

    auto func = GenericFunctionPointer(dlsym(mod, id.data()));
#endif
    return Message().SetObject(Object(func, kTypeIdFunctionPointer));
  }

  Message NewExtension(ObjectMap &p) { 
    using namespace extension;
    //auto tc = TypeChecking({ Expect("path",kTypeIdString) }, p);
    //if (TC_FAIL(tc)) return TC_ERROR(tc);
    auto &path = p.Cast<string>("path");
    ManagedExtension extension = make_shared<Extension>(path);
    auto loader = extension->GetExtensionLoader();

    if (loader == nullptr) {
      return Message("Invalid extension interface", StateLevel::Error);
    }

    ExtInterfaces interfaces{
      DisposeMemoryUnit,
      FetchObjectType,
      ReceiveError,
      FetchDescriptor,
      FetchArrayElementDescriptor,
      DumpObjectFromDescriptor,
      GetArrayObjectCapacity
    };

    auto result = loader(&interfaces);

    if (result != 1) {
      return Message("Error is occurred while core is trying to load extension");
    }

    return Message().SetObject(Object(extension, kTypeIdExtension));
  }

  Message Extension_Good(ObjectMap &p) {
    auto &extension = p.Cast<Extension>(kStrMe);
    return Message().SetObject(extension.Good());
  }

  Message Extension_FetchFunction(ObjectMap &p) {
    //TODO:Variable arugment
    //auto tc = TypeChecking({ Expect("id",kTypeIdString) }, p);
    //if (TC_FAIL(tc)) return TC_ERROR(tc);
    auto &extension = p.Cast<Extension>(kStrMe);
    auto &id = p.Cast<string>("id");
    auto activity = extension.FetchFunction(id);
    auto informer = extension.GetParameterInformer();

    if (activity != nullptr && informer != nullptr) {
      string param_pattern(informer(id.data()));
      auto mode = ParameterPattern::Fixed;

      if (!param_pattern.empty() && param_pattern.front() == '@') {
        mode = ParameterPattern::Variable;
        if (param_pattern.size() == 1) 
          return Message("Invalid argument size for variable function", StateLevel::Error);
        param_pattern = param_pattern.substr(1, param_pattern.size() - 1);
      }

      shared_ptr<Function> impl_ptr =
        make_shared<Function>(activity, id, param_pattern, mode);

      return Message().SetObject(Object(impl_ptr, kTypeIdFunction));
    }

    return Message().SetObject(Object());
  }

  void InitExtensionComponents() {
    using namespace components;

    CreateFunctionObject(Function(GetFunctionPointer, "library|id", "get_function_ptr"));

    CreateStruct(kTypeIdExtension);
    StructMethodGenerator(kTypeIdExtension).Create(
      {
        Function(NewExtension, "path", kStrInitializer),
        Function(Extension_Good, "", "good"),
        Function(Extension_FetchFunction, "id", "fetch")
      }
    );

    EXPORT_CONSTANT(kTypeIdExtension);
  }
}