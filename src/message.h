#pragma once
#include "lexical.h"
#include "object.h"

namespace sapphire {
  enum class StateLevel { Normal, Error, Warning };

  struct ObjectPrototype { ObjectInfo info; shared_ptr<void> ptr; };

  class Message {
  private:
    StateLevel level_;
    string detail_;
    optional<ObjectPrototype> slot_;
    size_t idx_;

  public:
    Message() :
      level_(StateLevel::Normal), detail_(""), slot_(std::nullopt), idx_(0) {}
    Message(Message &msg) :
      level_(msg.level_), detail_(msg.detail_), slot_(msg.slot_), idx_(msg.idx_) {}
    Message(Message &&msg) :
      level_(msg.level_), detail_(std::forward<string>(msg.detail_)),
      slot_(std::forward<optional<ObjectPrototype>>(msg.slot_)), idx_(msg.idx_) {}
    Message(string detail, StateLevel level = StateLevel::Normal) :
      level_(level), detail_(detail), idx_(0) {}

    Message &operator=(Message &msg) {
      level_ = msg.level_;
      detail_ = msg.detail_;
      slot_ = msg.slot_;
      idx_ = msg.idx_;
      return *this;
    }

    Message &operator=(Message &&msg) {
      return this->operator=(msg);
    }

    StateLevel GetLevel() const { return level_; }
    string GetDetail() const { return detail_; }
    size_t GetIndex() const { return idx_; }
    bool HasObject() const { return slot_.has_value(); }
    const ObjectInfo &GetObjectInfo() const { return slot_.value().info; }
    const shared_ptr<void> &GetPtr() const { return slot_.value().ptr; }

    Object GetObj() const {
      if (slot_.has_value()) {
        auto &prototype = slot_.value();
        return Object(prototype.info, prototype.ptr);
      }

      return Object();
    }

    Message &SetObject(Object &object) {
      slot_ = ObjectPrototype{ object.GetObjectInfoTable(), object.Get() };
      return *this;
    }

    Message &SetObject(Object &&object) {
      return this->SetObject(object);
    }

    Message &SetObjectRef(Object &obj) {
      slot_ = ObjectPrototype{
        ObjectInfo{&obj, ObjectMode::Ref, true, false, true, obj.GetTypeId()}, nullptr
      };

      return *this;
    }

#define MAKE_PROTOTYPE(_Id, _Type)  ObjectPrototype {                     \
      ObjectInfo{ nullptr, ObjectMode::Normal, true, false, true, _Id },      \
      make_shared<_Type>(value)                                          \
    }

    Message &SetObject(bool value) {
      slot_ = MAKE_PROTOTYPE(kTypeIdBool, bool);
      return *this;
    }

    Message &SetObject(int64_t value) {
      slot_ = MAKE_PROTOTYPE(kTypeIdInt, int64_t);
      return *this;
    }

    Message &SetObject(double value) {
      slot_ = MAKE_PROTOTYPE(kTypeIdFloat, double);
      return *this;
    }

    Message &SetObject(string value) {
      slot_ = MAKE_PROTOTYPE(kTypeIdString, string);
      return *this;
    }

#undef MAKE_PROTOTYPE

    Message &SetLevel(StateLevel level) {
      level_ = level;
      return *this;
    }

    Message &SetDetail(const string &detail) {
      detail_ = detail;
      return *this;
    }

    Message &SetIndex(const size_t index) {
      idx_ = index;
      return *this;
    }

    void Clear() {
      level_ = StateLevel::Normal;
      detail_.clear();
      detail_.shrink_to_fit();
      slot_ = std::nullopt;
      idx_ = 0;
    }
  };

  using CommentedResult = tuple<bool, string>;
}