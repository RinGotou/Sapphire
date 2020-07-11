#include "machine.h"

namespace sapphire {
  Message ExistFSObject(ObjectMap &p) {
    auto tc = TypeChecking(
      { Expect("path", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &path = p.Cast<string>("path");
    auto exists = fs::exists(fs::path(path));

    return Message().SetObject(exists);
  }

  Message CreateNewDirectory(ObjectMap &p) {
    auto tc = TypeChecking(
      { Expect("path", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &path = p.Cast<string>("path");
    auto result = fs::create_directories(path);
    return Message().SetObject(result);
  }

  Message RemoveFSObject(ObjectMap &p) {
    auto tc = TypeChecking(
      { Expect("path", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &path = p.Cast<string>("path");
    auto result = fs::remove(fs::path(path));

    return Message().SetObject(result);
  }

  Message RemoveFSObject_Recursive(ObjectMap &p) {
    auto tc = TypeChecking(
      { Expect("path", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &path = p.Cast<string>("path");
    auto result = fs::remove_all(fs::path(path));
    
    return Message().SetObject(int64_t(result));
  }

  Message CopyFSObject(ObjectMap &p) {
    auto tc = TypeChecking(
      { 
        Expect("from", kTypeIdString), 
        Expect("to", kTypeIdString)
      }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto from = p.Cast<string>("from");
    auto to = p.Cast<string>("to");
    Message result;

    try {
      fs::copy(fs::path(from), fs::path(to));
    }
    catch (std::exception & e) {
      result = Message(e.what(), kStateError);
    }

    return result;
  }

  Message CopyFSFile(ObjectMap &p) {
    auto tc = TypeChecking(
      {
        Expect("from", kTypeIdString),
        Expect("to", kTypeIdString)
      }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto from = p.Cast<string>("from");
    auto to = p.Cast<string>("to");
    auto result = fs::copy_file(fs::path(from), fs::path(to));
    return Message().SetObject(result);
  }

  Message SetWorkingDir(ObjectMap &p) {
    auto tc_result = TypeChecking(
      { Expect("dir", kTypeIdString) }, p, { "dir" });

    if (TC_FAIL(tc_result)) return TC_ERROR(tc_result);

    auto &dir_obj = p["dir"];
    string dest_dir;

    if (dir_obj.Null()) {
      dest_dir = mgmt::runtime::GetScriptAbsolutePath();
    }
    else {
      dest_dir = dir_obj.Cast<string>();
    }

    bool result = mgmt::runtime::SetWorkingDirectory(dest_dir);
    return Message().SetObject(result);
  }

  Message GetWorkingDir(ObjectMap &p) {
    return Message().SetObject(mgmt::runtime::GetWorkingDirectory());
  }

  Message GetScriptAbsolutePath(ObjectMap &p) {
    return Message().SetObject(mgmt::runtime::GetScriptAbsolutePath());
  }

  Message GetCoreAbsolutePath(ObjectMap &p) {
    return Message().SetObject(mgmt::runtime::GetBinaryPath());
  }

  Message GetDirectoryContent(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("path", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    string path_str = p.Cast<string>("path");
    auto managed_array = make_shared<ObjectArray>();
    for (auto &unit : fs::directory_iterator(path_str)) {
      managed_array->emplace_back(Object(unit.path().string(), kTypeIdString));
    }

    return Message().SetObject(Object(managed_array, kTypeIdArray));
  }

  Message GetFilenameExtension(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("path", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    fs::path value(p.Cast<string>("path"));
    return Message().SetObject(Object(value.extension().string(), kTypeIdString));
  }

  void InitConsoleComponents() {
    using namespace components;

    CreateFunctionObject(FunctionImpl(SetWorkingDir, "dir", "chdir", kParamAutoFill).SetLimit(0));
    CreateFunctionObject(FunctionImpl(GetWorkingDir, "", "current_directory"));
    CreateFunctionObject(FunctionImpl(GetScriptAbsolutePath, "", "boot_directory"));
    CreateFunctionObject(FunctionImpl(GetCoreAbsolutePath, "", "core_directory"));
    CreateFunctionObject(FunctionImpl(ExistFSObject, "path", "exist_fsobj"));
    CreateFunctionObject(FunctionImpl(CreateNewDirectory, "path", "create_dir"));
    CreateFunctionObject(FunctionImpl(RemoveFSObject, "path", "remove_fsobj"));
    CreateFunctionObject(FunctionImpl(RemoveFSObject_Recursive, "path", "remove_all_fsobj"));
    CreateFunctionObject(FunctionImpl(CopyFSObject, "from|to", "copy_fsobj"));
    CreateFunctionObject(FunctionImpl(CopyFSFile, "from|to", "copy_file"));
    CreateFunctionObject(FunctionImpl(GetDirectoryContent, "path", "dir_content"));
    CreateFunctionObject(FunctionImpl(GetFilenameExtension, "path", "filename_ext"));
  }
}
