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
      dest_dir = runtime::GetScriptAbsolutePath();
    }
    else {
      dest_dir = dir_obj.Cast<string>();
    }

    bool result = runtime::SetWorkingDirectory(dest_dir);
    return Message().SetObject(result);
  }

  Message GetWorkingDir(ObjectMap &p) {
    return Message().SetObject(runtime::GetWorkingDirectory());
  }

  Message GetScriptAbsolutePath(ObjectMap &p) {
    return Message().SetObject(runtime::GetScriptAbsolutePath());
  }

  Message GetCoreAbsolutePath(ObjectMap &p) {
    return Message().SetObject(runtime::GetBinaryPath());
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

    CreateFunctionObject(Function(SetWorkingDir, "dir", "chdir", kParamAutoFill).SetLimit(0));
    CreateFunctionObject(Function(GetWorkingDir, "", "current_directory"));
    CreateFunctionObject(Function(GetScriptAbsolutePath, "", "boot_directory"));
    CreateFunctionObject(Function(GetCoreAbsolutePath, "", "core_directory"));
    CreateFunctionObject(Function(ExistFSObject, "path", "exist_fsobj"));
    CreateFunctionObject(Function(CreateNewDirectory, "path", "create_dir"));
    CreateFunctionObject(Function(RemoveFSObject, "path", "remove_fsobj"));
    CreateFunctionObject(Function(RemoveFSObject_Recursive, "path", "remove_all_fsobj"));
    CreateFunctionObject(Function(CopyFSObject, "from|to", "copy_fsobj"));
    CreateFunctionObject(Function(CopyFSFile, "from|to", "copy_file"));
    CreateFunctionObject(Function(GetDirectoryContent, "path", "dir_content"));
    CreateFunctionObject(Function(GetFilenameExtension, "path", "filename_ext"));
  }
}
