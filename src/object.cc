#include "object.h"

namespace sapphire {
  vector<string> BuildStringVector(string source) {
    vector<string> result;
    string temp;
    for (auto unit : source) {
      if (unit == '|') {
        result.push_back(temp);
        temp.clear();
        continue;
      }
      temp.append(1, unit);
    }
    if (temp != "") result.push_back(temp);
    return result;
  }

  string CombineStringVector(vector<string> target) {
    string result;
    for (size_t i = 0; i < target.size(); ++i) {
      result = result + target[i] + "|";
    }
    result.pop_back();
    return result;
  }

  size_t PointerHasher(shared_ptr<void> ptr) {
    auto hasher = std::hash<shared_ptr<void>>();
    return hasher(ptr);
  }

  shared_ptr<void> ShallowDelivery(shared_ptr<void> target) {
    return target;
  }

  Object &Object::operator=(const Object &object) {
    info_ = object.info_;

    if (info_.mode != kObjectRef) {
      info_.real_dest = nullptr;
      dynamic_cast<shared_ptr<void> *>(this)->operator=(object);
    }
    else {
      reset();
    }

    return *this;
  }

  Object &Object::PackContent(shared_ptr<void> ptr, string type_id) {
    if (info_.mode == kObjectRef) {
      return static_cast<ObjectPointer>(info_.real_dest)
        ->PackContent(ptr, type_id);
    }

    dynamic_cast<shared_ptr<void> *>(this)->operator=(ptr);
    info_.type_id = type_id;
    return *this;
  }

  Object &Object::swap(Object &obj) {
    dynamic_cast<shared_ptr<void> *>(this)->swap(obj);
    std::swap(info_.type_id, obj.info_.type_id);
    std::swap(info_.mode, obj.info_.mode);
    std::swap(info_.delivering, obj.info_.delivering);
    std::swap(info_.real_dest, obj.info_.real_dest);
    std::swap(info_.sub_container, obj.info_.sub_container);
    std::swap(info_.alive, obj.info_.alive);
    return *this;
  }

  Object &Object::PackObject(Object &object) {
    reset();
    info_.type_id = object.info_.type_id;
    info_.mode = kObjectRef;

    if (!object.IsRef()) {
      info_.real_dest = &object;
      info_.alive = true;
    }
    else {
      info_.real_dest = object.info_.real_dest;
      info_.alive = object.info_.alive;
    }

    if (object.IsAlive()) EstablishRefLink();

    return *this;
  }

  bool ObjectContainer::Add(string id, Object &source, size_t token_id) {
    if (IsDelegated()) return delegator_->Add(id, source);
    auto result = container_.try_emplace(id, source);
    if (result.second && token_id != 0) {
      token_cache_[token_id] = &result.first->second;
    }

    return result.second;
  }

  bool ObjectContainer::Add(string id, Object &&source, size_t token_id) {
    if (IsDelegated()) return delegator_->Add(id, std::move(source));
    auto result = container_.try_emplace(id, source);
    if (result.second && token_id != 0) {
      token_cache_[token_id] = &result.first->second;
    }

    return result.second;
  }

  void ObjectContainer::Replace(string id, Object &source, size_t token_id) {
    if (IsDelegated()) delegator_->Replace(id, source);

    container_[id] = source;
    if (token_id != 0) {
      token_cache_[token_id] = &container_[id];
    }
  }

  void ObjectContainer::Replace(string id, Object &&source, size_t token_id) {
    if (IsDelegated()) delegator_->Replace(id, std::move(source));

    container_[id] = source;
    if (token_id != 0) {
      token_cache_[token_id] = &container_[id];
    }
  }

  Object *ObjectContainer::Find(const string &id, bool forward_seeking) {
    if (IsDelegated()) return delegator_->Find(id, forward_seeking);
    ObjectPointer ptr = nullptr;

    auto it = container_.find(id);

    if (it != container_.end()) {
      ptr = &it->second;
    }
    else if (prev_ != nullptr && forward_seeking) {
      ptr = prev_->Find(id, forward_seeking);
    }

    return ptr;
  }

  Object *ObjectContainer::FindByTokenId(size_t token_id, bool forward_seeking) {
    if (IsDelegated()) return delegator_->FindByTokenId(token_id, forward_seeking);
    ObjectPointer ptr = nullptr;
    
    auto it = token_cache_.find(token_id);

    if (it != token_cache_.end()) {
      ptr = it->second;
    }
    else if (prev_ != nullptr && forward_seeking) {
      ptr = prev_->FindByTokenId(token_id, forward_seeking);
    }

    return ptr;
  }

  Object *ObjectContainer::FindWithDomain(const string &id, 
    const string &domain, bool forward_seeking) {
    if (IsDelegated()) return delegator_->FindWithDomain(id, domain, forward_seeking);
  
    if (container_.empty() && prev_ == nullptr) return nullptr;

    ObjectPointer container_ptr = Find(domain, true);

    if (container_ptr == nullptr || (!container_ptr->IsSubContainer())) return nullptr;

    auto &sub_container = container_ptr->Cast<ObjectStruct>();
    return sub_container.Find(id, false);
  }

  Object *ObjectContainer::FindWithDomainByTokenId(size_t token_id, const string &id, bool forward_seeking) {
    if (IsDelegated()) return delegator_->FindWithDomainByTokenId(token_id, id, forward_seeking);
    
    if (token_cache_.empty() && prev_ == nullptr) return nullptr;

    auto container_ptr = FindByTokenId(token_id, true);
    if (container_ptr == nullptr || (!container_ptr->IsSubContainer())) return nullptr;

    auto &sub_container = container_ptr->Cast<ObjectStruct>();
    return sub_container.Find(id, false);
  }

  bool ObjectContainer::IsInside(Object *ptr) {
    if (IsDelegated()) return delegator_->IsInside(ptr);

    bool result = false;
    for (const auto &unit : container_) {
      if (&unit.second == ptr) result = true;
    }
    return result;
  }

  void ObjectContainer::ClearExcept(string exceptions) {
    if (IsDelegated()) delegator_->ClearExcept(exceptions);
    using Iterator = unordered_map<string, Object>::iterator;

    auto obj_list = BuildStringVector(exceptions);
    Iterator ready_to_del;
    Iterator it = container_.begin();

    while (it != container_.end()) {
      if (!find_in_vector(it->first, obj_list)) {
        ready_to_del = it;
        ++it;
        container_.erase(ready_to_del);
        continue;
      }

      ++it;
    }
  }

  ObjectMap &ObjectMap::operator=(const initializer_list<NamedObject> &rhs) {
    this->clear();
    for (const auto &unit : rhs) {
      this->insert(unit);
    }

    return *this;
  }

  ObjectMap &ObjectMap::operator=(const ObjectMap &rhs) {
    this->clear();
    for (const auto &unit : rhs) {
      this->insert(unit);
    }
    return *this;
  }

  void ObjectMap::Naturalize(ObjectContainer &container) {
    for (auto it = begin(); it != end(); ++it) {
      if (it->second.IsRef() && container.IsInside(it->second.GetRealDest())) {
        it->second = it->second.Unpack();
      }
    }
  }

  void ObjectStack::MergeMap(ObjectMap &p) {
    if (p.empty()) return;

    auto &container = base_.back();
    for (auto &unit : p) {
      container.Add(unit.first, unit.second.IsRef() ?
        Object().PackObject(unit.second) :
        unit.second);
    }
  }

  Object *ObjectStack::Find(const string &id, size_t token_id) {
    if (base_.empty() && prev_ == nullptr) return nullptr;
    //default: forward_seeking = true
    ObjectPointer ptr = token_id == 0 ?
      base_.back().Find(id) : 
      base_.back().FindByTokenId(token_id);

    if (prev_ != nullptr && ptr == nullptr) {
      ptr = prev_->Find(id);
    }

    return ptr;
  }

  Object *ObjectStack::Find(const string &id, const string &domain, size_t token_id) {
    if (base_.empty() && prev_ == nullptr) return nullptr;
    //default: forward_seeking = true
    ObjectPointer ptr = token_id == 0 ?
      base_.back().FindWithDomain(id, domain) :
      base_.back().FindWithDomainByTokenId(token_id, id);

    if (prev_ != nullptr && ptr == nullptr) {
      ptr = prev_->Find(id, domain);
    }

    return ptr;
  }

  bool ObjectStack::CreateObject(string id, Object &obj, size_t token_id) {
    if (!creation_info_.empty() && !creation_info_.top().first) {
      ScopeCreation(creation_info_.top().second);
      creation_info_.top().first = true;
    }

    if (base_.empty()) {
      if (prev_ == nullptr) {
        return false;
      }
      return prev_->CreateObject(id, obj, token_id);
    }
    auto &top = base_.back();

    return top.Add(id, obj, token_id);
  }

  bool ObjectStack::CreateObject(string id, Object &&obj, size_t token_id) {
    if (!creation_info_.empty() && !creation_info_.top().first) {
      ScopeCreation(creation_info_.top().second);
      creation_info_.top().first = true;
    }

    if (base_.empty()) {
      if (prev_ == nullptr) {
        return false;
      }
      return prev_->CreateObject(id, std::move(obj), token_id);
    }
    auto &top = base_.back();
    
    return top.Add(id, std::move(obj), token_id);
  }
}