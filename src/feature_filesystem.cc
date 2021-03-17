#include "machine.h"

namespace sapphire {
  //TODO: merge all fucntion into filesystem struct
  int ExistFSObject(State &state, ObjectMap &p) {
    auto &path = p.Cast<string>("path");
    auto exists = fs::exists(fs::path(path));
    state.PushValue(Object(exists, kTypeIdBool));
    return 0;
  }

  int CreateNewDirectory(State &state, ObjectMap &p) {
    auto &path = p.Cast<string>("path");
    auto result = fs::create_directories(path);
    state.PushValue(Object(result, kTypeIdBool));
    return 0;
  }

  int RemoveFSObject(State &state, ObjectMap &p) {
    auto &path = p.Cast<string>("path");
    auto result = fs::remove(fs::path(path));
    state.PushValue(Object(result, kTypeIdBool));
    return 0;
  }

  int RemoveFSObject_Recursive(State &state, ObjectMap &p) {
    auto &path = p.Cast<string>("path");
    auto result = fs::remove_all(fs::path(path));
    state.PushValue(Object(result, kTypeIdBool));
    return 0;
  }
  
  int CopyFSObject(State &state, ObjectMap &p) {
    auto from = p.Cast<string>("from");
    auto to = p.Cast<string>("to");
    int result = 0;

    try {
      fs::copy(fs::path(from), fs::path(to));
    }
    catch (std::exception & e) {
      result = 2;
      state.SetMsg(e.what());
    }

    return result;
  }

  int CopyFSFile(State &state, ObjectMap &p) {
    auto from = p.Cast<string>("from");
    auto to = p.Cast<string>("to");
    auto result = fs::copy_file(fs::path(from), fs::path(to));
    state.PushValue(Object(result, kTypeIdBool));
    return 0;
  }

  int SetWorkingDir(State &state, ObjectMap &p) {
    auto &dir_obj = p["dir"];
    string dest_dir;

    dest_dir = dir_obj.Cast<string>();

    bool result = runtime::SetWorkingDirectory(dest_dir);
    state.PushValue(Object(result, kTypeIdBool));
    return 0;
  }

  int StartHere(State &state, ObjectMap &p) {
    using namespace runtime;
    auto result = SetWorkingDirectory(GetScriptAbsolutePath());
    state.PushValue(Object(result, kTypeIdBool));
    return 0;
  }

  int GetWorkingDir(State &state, ObjectMap &p) {
    state.PushValue(Object(runtime::GetWorkingDirectory(), kTypeIdString));
    return 0;
  }

  int GetScriptAbsolutePath(State &state, ObjectMap &p) {
    state.PushValue(Object(runtime::GetScriptAbsolutePath(), kTypeIdString));
    return 0;
  }

  int GetCoreAbsolutePath(State &state, ObjectMap &p) {
    state.PushValue(Object(runtime::GetBinaryPath(), kTypeIdString));
    return 0;
  }

  int GetDirectoryContent(State &state, ObjectMap &p) {
    string path_str = p.Cast<string>("path");
    auto managed_array = make_shared<ObjectArray>();
    for (auto &unit : fs::directory_iterator(path_str)) {
      managed_array->emplace_back(Object(unit.path().string(), kTypeIdString));
    }

    state.PushValue(Object(managed_array, kTypeIdArray));
    return 0;
  }

  int GetFilenameExtension(State &state, ObjectMap &p) {
    fs::path value(p.Cast<string>("path"));
    state.PushValue(Object(value.extension().string(), kTypeIdString));
    return 0;
  }

  void InitConsoleComponents() {
    using namespace components;

    CreateFunctionObject(Function(StartHere, "", "starthere"));
    CreateFunctionObject(Function(SetWorkingDir, "dir", "chdir"));
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
