//BSD 2 - Clause License
//
//Copyright(c) 2017 - 2018, Suzu Nakamura
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met :
//
//*Redistributions of source code must retain the above copyright notice, this
//list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//this list of conditions and the following disclaimer in the documentation
//and/or other materials provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#include "template.h"

namespace kagami {
  //FileStream

  //RawString
  shared_ptr<void> StringCopy(shared_ptr<void> target) {
    string temp(*static_pointer_cast<string>(target));
    return make_shared<string>(temp);
  }

  //Array
  shared_ptr<void> ArrayCopy(shared_ptr<void> target) {
    auto ptr = static_pointer_cast<deque<Object>>(target);
    size_t size = ptr->size(), count = 0;
    deque<Object> base;

    for (auto &unit : *ptr) {
      base.push_back(unit);
    }
    return make_shared<deque<Object>>(base);
  }

  //Cube

  //Regex
  

  //Null
  shared_ptr<void> NullCopy(shared_ptr<void> target) {
    return nullptr;
  }

  Message ArrayConstructor(ObjectMap &p) {
    Message result;
    Attribute attribute("", false);
    Object size = p.at("size"), init_value = p.at("init_value");
    int size_value = stoi(*static_pointer_cast<string>(size.get()));
    int count = 0;
    shared_ptr<void> init_ptr = nullptr;
    deque<Object> base;

    //error:wrong size
    if (size_value <= 0) {
      result.combo(kStrFatalError, kCodeIllegalArgs, "Illegal array size.");
      return result;
    }

    attribute = init_value.getTag();
    attribute.ro = false;

    for (count = 0; count < size_value; count++) {
      init_ptr = type::GetObjectCopy(init_value);
      base.push_back(Object().set(init_ptr, init_value.GetTypeId(), Kit().MakeAttrTagStr(attribute)));
    }

    result.combo(kTypeIdArrayBase, kCodeObject, "__result");
    result.GetPtr() = make_shared<deque<Object>>(base);

    return result;
  }
  
  Message GetElement(ObjectMap &p) {
    Message result;
    Object object = p.at("object"), subscript_1 = p.at("subscript_1");
    string type_id = object.GetTypeId();
    int size = 0;
    int count0 = 0;

    if (type_id == kTypeIdRawString) {
      count0 = stoi(*static_pointer_cast<string>(subscript_1.get()));
      size = static_pointer_cast<string>(object.get())->size();
      if (count0 <= size - 1) {
        result.combo(kStrRedirect, kCodeSuccess, string().append(1,
          static_pointer_cast<string>(object.get())->at(count0)));
      }
    }
    else if (type_id == kTypeIdArrayBase) {
      count0 = stoi(*static_pointer_cast<string>(subscript_1.get()));
      size = static_pointer_cast<deque<Object>>(object.get())->size();
      if (count0 <= size - 1) {
        auto &target = static_pointer_cast<deque<Object>>(object.get())->at(count0);
        result.combo(target.GetTypeId(), kCodeObject, "__element");
        result.GetPtr() = target.get();
      }
    }

    return result;
  }

  Message GetSize(ObjectMap &p) {
    Message result;
    Object object = p.at("object");
    string type_id = object.GetTypeId();

    if (type_id == kTypeIdArrayBase) {
      result.SetDetail(to_string(static_pointer_cast<deque<Object>>(object.get())->size()));
    }
    else if (type_id == kTypeIdRawString) {
      auto str = *static_pointer_cast<string>(object.get());
      result.SetDetail(to_string(str.size()));
    }

    result.SetValue(kStrRedirect);
    return result;
  }



  Message GetElement_2Dimension(ObjectMap &p) {
    Message result;
    Object object = p.at("object"), subscript_1 = p.at("subscript_1"), subscript_2 = p.at("subscript_2");
    string type_id = object.GetTypeId();
    int size = 0;
    int count0 = 0, count1 = 0;

    //TODO:

    return result;
  }

  void InitTemplates() {
    using type::AddTemplate;
    AddTemplate(kTypeIdRawString, ObjTemplate(SimpleSharedPtrCopy<string>, "size|substr|at"));
    AddTemplate(kTypeIdArrayBase, ObjTemplate(ArrayCopy, "size|at"));
    AddTemplate(kTypeIdNull, ObjTemplate(NullCopy, ""));
  }

  void InitMethods() {
    using namespace entry;
    ActivityTemplate temp;
    //types
    Inject(EntryProvider(temp.set("array", ArrayConstructor, kFlagNormalEntry, kCodeAutoFill, "size|init_value")));
    //methods
    Inject(EntryProvider(temp.set("at", GetElement, kFlagMethod, kCodeNormalArgs, "object|subscript_1", kTypeIdRawString)));
    Inject(EntryProvider(temp.set("at", GetElement, kFlagMethod, kCodeNormalArgs, "object|subscript_1", kTypeIdArrayBase)));
    Inject(EntryProvider(temp.set("at", GetElement_2Dimension, kFlagMethod, kCodeNormalArgs, "object|subscript_1|subscript_2", kTypeIdCubeBase)));
    Inject(EntryProvider(temp.set("size", GetSize, kFlagMethod, kCodeNormalArgs, "object", kTypeIdRawString)));
    Inject(EntryProvider(temp.set("size", GetSize, kFlagMethod, kCodeNormalArgs, "object", kTypeIdArrayBase)));
  }
}