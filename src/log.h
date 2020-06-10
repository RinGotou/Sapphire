#pragma once
#include <cstdio>
#include <ctime>
#include <cstring>
#include <string>
#include <exception>
#include <utility>
#include <type_traits>
#include <list>
#include <memory>

#if defined(_MSC_VER)
#pragma warning(disable:4996)
#endif

#define DEFAULT_OUTPUT "sapphire.log"

namespace sapphire {
  using std::string;
  using std::exception;
  using std::move;
  using std::is_same;
  using std::is_base_of;
  using std::list;
  using std::strcmp;
  using std::unique_ptr;
  using std::make_unique;

  using CharList = list<char>;

  class Writer {
  public:
    virtual ~Writer() {}
    virtual bool Write(char) = 0;
    virtual bool Write(CharList &) = 0;
    virtual bool Write(const char *, size_t) = 0;
    virtual bool Write(string &) = 0;
    virtual bool Write(string &&) = 0;
    virtual bool Good() const = 0;
  };

  class StandardWriter : public Writer {
  protected:
    FILE *ptr_;
  public:
    virtual ~StandardWriter() {
      if (ptr_ != nullptr && ptr_ != stdout) fclose(ptr_);
    }

    StandardWriter() = delete;
    StandardWriter(FILE *ptr) : ptr_(ptr) {}
    StandardWriter(string path, string mode) :
      ptr_(fopen(path.data(), mode.data())) {}
    StandardWriter(StandardWriter &) = delete;
    StandardWriter(StandardWriter &&rhs) :
      ptr_(rhs.ptr_) { rhs.ptr_ = nullptr; }

    void operator=(StandardWriter &) = delete;
    void operator=(StandardWriter &&rhs);
    virtual bool Write(char c) override;
    virtual bool Write(CharList &data) override;
    virtual bool Write(const char *data, size_t size = 0) override;
    virtual bool Write(string &data) override;
    virtual bool Write(string &&data) override { return Write(data); }
    virtual bool Good() const override { return ptr_ != nullptr; }
  };

  class Decorator {
  public:
    virtual ~Decorator() {}
    virtual void WriteHead(CharList &) = 0;
    virtual bool WriteHead(Writer *) = 0;
    virtual void WriteTail(CharList &) = 0;
    virtual bool WriteTail(Writer *) = 0;
  };

  class StandardDecorator : public Decorator {
  public:
    void WriteHead(CharList &) override;
    bool WriteHead(Writer *) override;
    void WriteTail(CharList &) override { /* Do Nothing */ }
    bool WriteTail(Writer *) override { return true; } /* Do Nothing */

  };

  class StandardLogger {
  public:
    virtual ~StandardLogger() {}
    virtual bool WriteLine(const char *, size_t) = 0;
    virtual bool WriteLine(string &) = 0;
    virtual bool WriteLine(string &&) = 0;
    virtual bool WriteLine(exception *e) = 0;
    virtual bool Good() const = 0;
  };

  template <
    typename _Writer = StandardWriter,
    typename _Decorator = StandardDecorator>
  class CachedLogger : public StandardLogger {
    static_assert(is_base_of<Decorator, _Decorator>::value, "Decorator error");
    static_assert(is_base_of<Writer, _Writer>::value, "Writer error");
  protected:
    string dest_;
    string mode_;
    list<CharList> cache_;
    _Decorator decorator_;

  protected:
    void CopyToCache(const char *data, size_t size = 0);
    void CopyToCache(string &data);

  public:
    virtual ~CachedLogger();

    CachedLogger() : 
      dest_(DEFAULT_OUTPUT),
      mode_("a"), cache_(), decorator_() {}

    CachedLogger(string dest, string mode) :
      dest_(dest), mode_(mode),
      cache_(), decorator_() {}

    bool WriteLine(const char *data, size_t size = 0) override;
    bool WriteLine(string &data) override;
    bool WriteLine(string &&data) override { return WriteLine(data); }
    bool WriteLine(exception *e) override;
    bool Good() const override { return true; } //Do nothing
  };

  template<typename _Writer, typename _Decorator>
  void CachedLogger<_Writer,_Decorator>::CopyToCache(const char *data, size_t size) {
    size_t counter = 0;
    char const *pos = data;
    auto &dest_list = cache_.back();

    while (*pos != '\0' || (size != 0 && counter < size)) {
      dest_list.push_back(*pos);
      ++pos;
      counter += 1;
    }
  }

  template<typename _Writer, typename _Decorator>
  void CachedLogger<_Writer, _Decorator>::CopyToCache(string &data) {
    auto &dest_list = cache_.back();

    for (const auto &unit : data) {
      dest_list.push_back(unit);
    }
  }

  template<typename _Writer, typename _Decorator>
  inline CachedLogger<_Writer, _Decorator>::~CachedLogger() {
    Writer *writer = (strcmp(dest_.data(), "stdout") == 0) ?
      writer = new _Writer(stdout) :
      writer = new _Writer(dest_, mode_);
    
    for (auto it = cache_.begin(); it != cache_.end(); ++it) {
      writer->Write(*it);
      writer->Write("\n", 0);
    }

    delete writer;
  }

  template<typename _Writer, typename _Decorator>
  inline bool CachedLogger<_Writer, _Decorator>::WriteLine(const char *data, size_t size) {
    cache_.push_back(CharList());
    decorator_.WriteHead(cache_.back());
    CopyToCache(data, size);
    decorator_.WriteTail(cache_.back());
    return true;
  }

  template<typename _Writer, typename _Decorator>
  inline bool CachedLogger<_Writer, _Decorator>::WriteLine(string &data) {
    cache_.push_back(CharList());
    decorator_.WriteHead(cache_.back());
    CopyToCache(data);
    decorator_.WriteTail(cache_.back());
    return true;
  }

  template<typename _Writer, typename _Decorator>
  inline bool CachedLogger<_Writer, _Decorator>::WriteLine(exception *e) {
    cache_.push_back(CharList());
    decorator_.WriteHead(cache_.back());
    CopyToCache(e->what());
    decorator_.WriteTail(cache_.back());
    return true;
  }

  template <
    typename _Writer = StandardWriter,
    typename _Decorator = StandardDecorator>
  class RTLogger : public StandardLogger {
    static_assert(is_base_of<Decorator, _Decorator>::value, "Decorator error");
    static_assert(is_base_of<Writer, _Writer>::value, "Writer error");
  protected:
    _Decorator decorator_;
    _Writer writer_;
    
  public:
    virtual ~RTLogger() {}
    RTLogger() : writer_(DEFAULT_OUTPUT, "a") {}

    RTLogger(const char *dest, const char *mode) :
      writer_(dest, mode) {}

    RTLogger(string dest, string mode) :
      writer_(dest.data(), mode.data()) {}

    bool WriteLine(const char *data, size_t size = 0) override;
    bool WriteLine(string &data) override;
    bool WriteLine(string &&data) override { return WriteLine(data); }
    bool WriteLine(exception *e) override;
    bool Good() const override { return writer_.Good(); }
  };

  template<typename _Writer, typename _Decorator>
  inline bool RTLogger<_Writer, _Decorator>::WriteLine(const char *data, size_t size) {
    if (!writer_.Good()) return false;
    bool result = true;
    result = decorator_.WriteHead(&writer_);
    result = writer_.Write(data, size);
    result = decorator_.WriteTail(&writer_);
    result = writer_.Write('\n');
    return result;
  }

  template<typename _Writer, typename _Decorator>
  inline bool RTLogger<_Writer, _Decorator>::WriteLine(string &data) {
    if (!writer_.Good()) return false;
    bool result = true;
    result = decorator_.WriteHead(&writer_);
    result = writer_.Write(data);
    result = decorator_.WriteTail(&writer_);
    result = writer_.Write('\n');
    return result;
  }

  template<typename _Writer, typename _Decorator>
  inline bool RTLogger<_Writer, _Decorator>::WriteLine(exception *e) {
    if (!writer_.Good()) return false;
    bool result = true;
    result = decorator_.WriteHead(&writer_);
    result = writer_.Write(e->what(), 0);
    result = decorator_.WriteTail(&writer_);
    result = writer_.Write('\n');
    return result;
  }

  using StandardCachedLogger = CachedLogger<>;
  using StandardRTLogger = RTLogger<>;
}