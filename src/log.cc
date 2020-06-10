#include "log.h"

namespace sapphire {
  using namespace std;

  bool StandardWriter::Write(char c) {
    return fputc(c, ptr_) != EOF;
  }

  bool StandardWriter::Write(CharList &data) {
    int flag = 0;
    for (const auto &unit : data) {
      flag = fputc(unit, ptr_);
      if (flag == EOF) break;
    }

    return flag != EOF;
  }

  void StandardWriter::operator=(StandardWriter &&rhs) {
    if (ptr_ != nullptr) {
      fclose(ptr_);
    }
    ptr_ = rhs.ptr_;
  }

  bool StandardWriter::Write(const char *data, size_t size) {
    size_t counter = 0;
    int flag = 0;
    char const *pos = data;
    while (*pos != '\0' || (size != 0 && counter < size)) {
      flag = fputc(*pos, ptr_);
      counter += 1;
      ++pos;
      if (flag == EOF) break;
    }

    return flag != EOF;
  }

  bool StandardWriter::Write(string &data) {
    int flag = 0;
    for (const auto &unit : data) {
      flag = fputc(unit, ptr_);
      if (flag == EOF) break;
    }
    
    return flag != EOF;
  }

  void StandardDecorator::WriteHead(CharList &dest) {
    auto now = time(nullptr);
    auto *pos = ctime(&now);
    dest.push_back('[');
    while (*pos != '\n' && *pos != '\0') {
      dest.push_back(*pos);
      ++pos;
    }
    dest.push_back(']');
  }

  bool StandardDecorator::WriteHead(Writer *dest) {
    auto now = time(nullptr);
    auto *pos = ctime(&now);
    bool flag = true;
    dest->Write('[');
    while (*pos != '\n' && *pos != '\0') {
      flag = dest->Write(*pos);
      if (!flag) break;
      ++pos;
    }
    dest->Write(']');
    return flag;
  }
}