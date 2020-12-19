#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <initializer_list>
#include <iterator>
#include <new>
#include <string>
#include <type_traits>
#include <utility>

namespace rust {
inline namespace cxxbridge1 {
// #include "rust/cxx.h"

#ifndef CXXBRIDGE1_PANIC
#define CXXBRIDGE1_PANIC
template <typename Exception>
void panic [[noreturn]] (const char *msg);
#endif // CXXBRIDGE1_PANIC

struct unsafe_bitcopy_t;

namespace {
template <typename T>
class impl;
} // namespace

#ifndef CXXBRIDGE1_RUST_STRING
#define CXXBRIDGE1_RUST_STRING
class String final {
public:
  String() noexcept;
  String(const String &) noexcept;
  String(String &&) noexcept;
  ~String() noexcept;

  String(const std::string &);
  String(const char *);
  String(const char *, std::size_t);

  String &operator=(const String &) noexcept;
  String &operator=(String &&) noexcept;

  explicit operator std::string() const;

  const char *data() const noexcept;
  std::size_t size() const noexcept;
  std::size_t length() const noexcept;

  using iterator = char *;
  iterator begin() noexcept;
  iterator end() noexcept;

  using const_iterator = const char *;
  const_iterator begin() const noexcept;
  const_iterator end() const noexcept;
  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;

  bool operator==(const String &) const noexcept;
  bool operator!=(const String &) const noexcept;
  bool operator<(const String &) const noexcept;
  bool operator<=(const String &) const noexcept;
  bool operator>(const String &) const noexcept;
  bool operator>=(const String &) const noexcept;

  String(unsafe_bitcopy_t, const String &) noexcept;

private:
  std::array<std::uintptr_t, 3> repr;
};
#endif // CXXBRIDGE1_RUST_STRING

#ifndef CXXBRIDGE1_RUST_STR
#define CXXBRIDGE1_RUST_STR
class Str final {
public:
  Str() noexcept;
  Str(const String &) noexcept;
  Str(const std::string &);
  Str(const char *);
  Str(const char *, std::size_t);

  Str &operator=(const Str &) noexcept = default;

  explicit operator std::string() const;

  const char *data() const noexcept;
  std::size_t size() const noexcept;
  std::size_t length() const noexcept;

  Str(const Str &) noexcept = default;
  ~Str() noexcept = default;

  using iterator = const char *;
  using const_iterator = const char *;
  const_iterator begin() const noexcept;
  const_iterator end() const noexcept;
  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;

  bool operator==(const Str &) const noexcept;
  bool operator!=(const Str &) const noexcept;
  bool operator<(const Str &) const noexcept;
  bool operator<=(const Str &) const noexcept;
  bool operator>(const Str &) const noexcept;
  bool operator>=(const Str &) const noexcept;

private:
  friend impl<Str>;
  const char *ptr;
  std::size_t len;
};

inline const char *Str::data() const noexcept { return this->ptr; }

inline std::size_t Str::size() const noexcept { return this->len; }

inline std::size_t Str::length() const noexcept { return this->len; }
#endif // CXXBRIDGE1_RUST_STR

#ifndef CXXBRIDGE1_RUST_BOX
#define CXXBRIDGE1_RUST_BOX
template <typename T>
class Box final {
public:
  using value_type = T;
  using const_pointer =
      typename std::add_pointer<typename std::add_const<T>::type>::type;
  using pointer = typename std::add_pointer<T>::type;

  Box() = delete;
  Box(const Box &);
  Box(Box &&) noexcept;
  ~Box() noexcept;

  explicit Box(const T &);
  explicit Box(T &&);

  Box &operator=(const Box &);
  Box &operator=(Box &&) noexcept;

  const T *operator->() const noexcept;
  const T &operator*() const noexcept;
  T *operator->() noexcept;
  T &operator*() noexcept;

  template <typename... Fields>
  static Box in_place(Fields &&...);

  static Box from_raw(T *) noexcept;

  T *into_raw() noexcept;

private:
  class uninit;
  class allocation;
  Box(uninit) noexcept;
  void drop() noexcept;
  T *ptr;
};

template <typename T>
class Box<T>::uninit {};

template <typename T>
class Box<T>::allocation {
  static T *alloc() noexcept;
  static void dealloc(T *) noexcept;

public:
  allocation() noexcept : ptr(alloc()) {}
  ~allocation() noexcept {
    if (this->ptr) {
      dealloc(this->ptr);
    }
  }
  T *ptr;
};

template <typename T>
Box<T>::Box(const Box &other) : Box(*other) {}

template <typename T>
Box<T>::Box(Box &&other) noexcept : ptr(other.ptr) {
  other.ptr = nullptr;
}

template <typename T>
Box<T>::Box(const T &val) {
  allocation alloc;
  ::new (alloc.ptr) T(val);
  this->ptr = alloc.ptr;
  alloc.ptr = nullptr;
}

template <typename T>
Box<T>::Box(T &&val) {
  allocation alloc;
  ::new (alloc.ptr) T(std::move(val));
  this->ptr = alloc.ptr;
  alloc.ptr = nullptr;
}

template <typename T>
Box<T>::~Box() noexcept {
  if (this->ptr) {
    this->drop();
  }
}

template <typename T>
Box<T> &Box<T>::operator=(const Box &other) {
  if (this->ptr) {
    **this = *other;
  } else {
    allocation alloc;
    ::new (alloc.ptr) T(*other);
    this->ptr = alloc.ptr;
    alloc.ptr = nullptr;
  }
  return *this;
}

template <typename T>
Box<T> &Box<T>::operator=(Box &&other) noexcept {
  if (this->ptr) {
    this->drop();
  }
  this->ptr = other.ptr;
  other.ptr = nullptr;
  return *this;
}

template <typename T>
const T *Box<T>::operator->() const noexcept {
  return this->ptr;
}

template <typename T>
const T &Box<T>::operator*() const noexcept {
  return *this->ptr;
}

template <typename T>
T *Box<T>::operator->() noexcept {
  return this->ptr;
}

template <typename T>
T &Box<T>::operator*() noexcept {
  return *this->ptr;
}

template <typename T>
template <typename... Fields>
Box<T> Box<T>::in_place(Fields &&... fields) {
  allocation alloc;
  auto ptr = alloc.ptr;
  ::new (ptr) T{std::forward<Fields>(fields)...};
  alloc.ptr = nullptr;
  return from_raw(ptr);
}

template <typename T>
Box<T> Box<T>::from_raw(T *raw) noexcept {
  Box box = uninit{};
  box.ptr = raw;
  return box;
}

template <typename T>
T *Box<T>::into_raw() noexcept {
  T *raw = this->ptr;
  this->ptr = nullptr;
  return raw;
}

template <typename T>
Box<T>::Box(uninit) noexcept {}
#endif // CXXBRIDGE1_RUST_BOX

#ifndef CXXBRIDGE1_RUST_BITCOPY
#define CXXBRIDGE1_RUST_BITCOPY
struct unsafe_bitcopy_t final {
  explicit unsafe_bitcopy_t() = default;
};

constexpr unsafe_bitcopy_t unsafe_bitcopy{};
#endif // CXXBRIDGE1_RUST_BITCOPY

#ifndef CXXBRIDGE1_RUST_VEC
#define CXXBRIDGE1_RUST_VEC
template <typename T>
class Vec final {
public:
  using value_type = T;

  Vec() noexcept;
  Vec(std::initializer_list<T>);
  Vec(const Vec &);
  Vec(Vec &&) noexcept;
  ~Vec() noexcept;

  Vec &operator=(Vec &&) noexcept;
  Vec &operator=(const Vec &);

  std::size_t size() const noexcept;
  bool empty() const noexcept;
  const T *data() const noexcept;
  T *data() noexcept;
  std::size_t capacity() const noexcept;

  const T &operator[](std::size_t n) const noexcept;
  const T &at(std::size_t n) const;
  const T &front() const;
  const T &back() const;

  T &operator[](std::size_t n) noexcept;
  T &at(std::size_t n);
  T &front();
  T &back();

  void reserve(std::size_t new_cap);
  void push_back(const T &value);
  void push_back(T &&value);
  template <typename... Args>
  void emplace_back(Args &&... args);

  class iterator;
  iterator begin() noexcept;
  iterator end() noexcept;

  using const_iterator = typename Vec<const T>::iterator;
  const_iterator begin() const noexcept;
  const_iterator end() const noexcept;
  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;

  Vec(unsafe_bitcopy_t, const Vec &) noexcept;

private:
  static std::size_t stride() noexcept;
  void reserve_total(std::size_t cap) noexcept;
  void set_len(std::size_t len) noexcept;
  void drop() noexcept;

  std::array<std::uintptr_t, 3> repr;
};

template <typename T>
class Vec<T>::iterator final {
public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = typename std::add_pointer<T>::type;
  using reference = typename std::add_lvalue_reference<T>::type;

  reference operator*() const noexcept;
  pointer operator->() const noexcept;
  reference operator[](difference_type) const noexcept;

  iterator &operator++() noexcept;
  iterator operator++(int) noexcept;
  iterator &operator--() noexcept;
  iterator operator--(int) noexcept;

  iterator &operator+=(difference_type) noexcept;
  iterator &operator-=(difference_type) noexcept;
  iterator operator+(difference_type) const noexcept;
  iterator operator-(difference_type) const noexcept;
  difference_type operator-(const iterator &) const noexcept;

  bool operator==(const iterator &) const noexcept;
  bool operator!=(const iterator &) const noexcept;
  bool operator<(const iterator &) const noexcept;
  bool operator>(const iterator &) const noexcept;
  bool operator<=(const iterator &) const noexcept;
  bool operator>=(const iterator &) const noexcept;

private:
  friend class Vec;
  friend class Vec<typename std::remove_const<T>::type>;
  void *pos;
  std::size_t stride;
};

template <typename T>
Vec<T>::Vec(std::initializer_list<T> init) : Vec{} {
  this->reserve_total(init.size());
  std::move(init.begin(), init.end(), std::back_inserter(*this));
}

template <typename T>
Vec<T>::Vec(const Vec &other) : Vec() {
  this->reserve_total(other.size());
  std::copy(other.begin(), other.end(), std::back_inserter(*this));
}

template <typename T>
Vec<T>::Vec(Vec &&other) noexcept : repr(other.repr) {
  new (&other) Vec();
}

template <typename T>
Vec<T>::~Vec() noexcept {
  this->drop();
}

template <typename T>
Vec<T> &Vec<T>::operator=(Vec &&other) noexcept {
  if (this != &other) {
    this->drop();
    this->repr = other.repr;
    new (&other) Vec();
  }
  return *this;
}

template <typename T>
Vec<T> &Vec<T>::operator=(const Vec &other) {
  if (this != &other) {
    this->drop();
    new (this) Vec(other);
  }
  return *this;
}

template <typename T>
bool Vec<T>::empty() const noexcept {
  return size() == 0;
}

template <typename T>
T *Vec<T>::data() noexcept {
  return const_cast<T *>(const_cast<const Vec<T> *>(this)->data());
}

template <typename T>
const T &Vec<T>::operator[](std::size_t n) const noexcept {
  auto data = reinterpret_cast<const char *>(this->data());
  return *reinterpret_cast<const T *>(data + n * this->stride());
}

template <typename T>
const T &Vec<T>::at(std::size_t n) const {
  if (n >= this->size()) {
    panic<std::out_of_range>("rust::Vec index out of range");
  }
  return (*this)[n];
}

template <typename T>
const T &Vec<T>::front() const {
  return (*this)[0];
}

template <typename T>
const T &Vec<T>::back() const {
  return (*this)[this->size() - 1];
}

template <typename T>
T &Vec<T>::operator[](std::size_t n) noexcept {
  auto data = reinterpret_cast<char *>(this->data());
  return *reinterpret_cast<T *>(data + n * this->stride());
}

template <typename T>
T &Vec<T>::at(std::size_t n) {
  if (n >= this->size()) {
    panic<std::out_of_range>("rust::Vec index out of range");
  }
  return (*this)[n];
}

template <typename T>
T &Vec<T>::front() {
  return (*this)[0];
}

template <typename T>
T &Vec<T>::back() {
  return (*this)[this->size() - 1];
}

template <typename T>
void Vec<T>::reserve(std::size_t new_cap) {
  this->reserve_total(new_cap);
}

template <typename T>
void Vec<T>::push_back(const T &value) {
  this->emplace_back(value);
}

template <typename T>
void Vec<T>::push_back(T &&value) {
  this->emplace_back(std::move(value));
}

template <typename T>
template <typename... Args>
void Vec<T>::emplace_back(Args &&... args) {
  auto size = this->size();
  this->reserve_total(size + 1);
  ::new (reinterpret_cast<T *>(reinterpret_cast<char *>(this->data()) +
                               size * this->stride()))
      T(std::forward<Args>(args)...);
  this->set_len(size + 1);
}

template <typename T>
typename Vec<T>::iterator::reference
Vec<T>::iterator::operator*() const noexcept {
  return *static_cast<T *>(this->pos);
}

template <typename T>
typename Vec<T>::iterator::pointer
Vec<T>::iterator::operator->() const noexcept {
  return static_cast<T *>(this->pos);
}

template <typename T>
typename Vec<T>::iterator::reference Vec<T>::iterator::operator[](
    typename Vec<T>::iterator::difference_type n) const noexcept {
  auto pos = static_cast<char *>(this->pos) + this->stride * n;
  return *static_cast<T *>(pos);
}

template <typename T>
typename Vec<T>::iterator &Vec<T>::iterator::operator++() noexcept {
  this->pos = static_cast<char *>(this->pos) + this->stride;
  return *this;
}

template <typename T>
typename Vec<T>::iterator Vec<T>::iterator::operator++(int) noexcept {
  auto ret = iterator(*this);
  this->pos = static_cast<char *>(this->pos) + this->stride;
  return ret;
}

template <typename T>
typename Vec<T>::iterator &Vec<T>::iterator::operator--() noexcept {
  this->pos = static_cast<char *>(this->pos) - this->stride;
  return *this;
}

template <typename T>
typename Vec<T>::iterator Vec<T>::iterator::operator--(int) noexcept {
  auto ret = iterator(*this);
  this->pos = static_cast<char *>(this->pos) - this->stride;
  return ret;
}

template <typename T>
typename Vec<T>::iterator &Vec<T>::iterator::operator+=(
    typename Vec<T>::iterator::difference_type n) noexcept {
  this->pos = static_cast<char *>(this->pos) + this->stride * n;
  return *this;
}

template <typename T>
typename Vec<T>::iterator &Vec<T>::iterator::operator-=(
    typename Vec<T>::iterator::difference_type n) noexcept {
  this->pos = static_cast<char *>(this->pos) - this->stride * n;
  return *this;
}

template <typename T>
typename Vec<T>::iterator Vec<T>::iterator::operator+(
    typename Vec<T>::iterator::difference_type n) const noexcept {
  auto ret = iterator(*this);
  ret.pos = static_cast<char *>(this->pos) + this->stride * n;
  return ret;
}

template <typename T>
typename Vec<T>::iterator Vec<T>::iterator::operator-(
    typename Vec<T>::iterator::difference_type n) const noexcept {
  auto ret = iterator(*this);
  ret.pos = static_cast<char *>(this->pos) - this->stride * n;
  return ret;
}

template <typename T>
typename Vec<T>::iterator::difference_type
Vec<T>::iterator::operator-(const iterator &other) const noexcept {
  auto diff = std::distance(static_cast<char *>(other.pos),
                            static_cast<char *>(this->pos));
  return diff / this->stride;
}

template <typename T>
bool Vec<T>::iterator::operator==(const iterator &other) const noexcept {
  return this->pos == other.pos;
}

template <typename T>
bool Vec<T>::iterator::operator!=(const iterator &other) const noexcept {
  return this->pos != other.pos;
}

template <typename T>
bool Vec<T>::iterator::operator>(const iterator &other) const noexcept {
  return this->pos > other.pos;
}

template <typename T>
bool Vec<T>::iterator::operator<(const iterator &other) const noexcept {
  return this->pos < other.pos;
}

template <typename T>
bool Vec<T>::iterator::operator>=(const iterator &other) const noexcept {
  return this->pos >= other.pos;
}

template <typename T>
bool Vec<T>::iterator::operator<=(const iterator &other) const noexcept {
  return this->pos <= other.pos;
}

template <typename T>
typename Vec<T>::iterator Vec<T>::begin() noexcept {
  iterator it;
  it.pos = const_cast<typename std::remove_const<T>::type *>(this->data());
  it.stride = this->stride();
  return it;
}

template <typename T>
typename Vec<T>::iterator Vec<T>::end() noexcept {
  iterator it = this->begin();
  it.pos = static_cast<char *>(it.pos) + it.stride * this->size();
  return it;
}

template <typename T>
typename Vec<T>::const_iterator Vec<T>::begin() const noexcept {
  return this->cbegin();
}

template <typename T>
typename Vec<T>::const_iterator Vec<T>::end() const noexcept {
  return this->cend();
}

template <typename T>
typename Vec<T>::const_iterator Vec<T>::cbegin() const noexcept {
  const_iterator it;
  it.pos = const_cast<typename std::remove_const<T>::type *>(this->data());
  it.stride = this->stride();
  return it;
}

template <typename T>
typename Vec<T>::const_iterator Vec<T>::cend() const noexcept {
  const_iterator it = this->cbegin();
  it.pos = static_cast<char *>(it.pos) + it.stride * this->size();
  return it;
}

template <typename T>
Vec<T>::Vec(unsafe_bitcopy_t, const Vec &bits) noexcept : repr(bits.repr) {}
#endif // CXXBRIDGE1_RUST_VEC

#ifndef CXXBRIDGE1_RUST_ERROR
#define CXXBRIDGE1_RUST_ERROR
class Error final : public std::exception {
public:
  Error(const Error &);
  Error(Error &&) noexcept;
  ~Error() noexcept override;

  Error &operator=(const Error &);
  Error &operator=(Error &&) noexcept;

  const char *what() const noexcept override;

private:
  Error() noexcept = default;
  friend impl<Error>;
  const char *msg;
  std::size_t len;
};
#endif // CXXBRIDGE1_RUST_ERROR

namespace detail {
template <typename T, typename = void *>
struct operator_new {
  void *operator()(::std::size_t sz) { return ::operator new(sz); }
};

template <typename T>
struct operator_new<T, decltype(T::operator new(sizeof(T)))> {
  void *operator()(::std::size_t sz) { return T::operator new(sz); }
};
} // namespace detail

template <typename T>
union MaybeUninit {
  T value;
  void *operator new(::std::size_t sz) { return detail::operator_new<T>{}(sz); }
  MaybeUninit() {}
  ~MaybeUninit() {}
};

namespace {
namespace repr {
struct PtrLen final {
  void *ptr;
  ::std::size_t len;
};
} // namespace repr

template <>
class impl<Str> final {
public:
  static repr::PtrLen repr(Str str) noexcept {
    return repr::PtrLen{const_cast<char *>(str.ptr), str.len};
  }
};

template <>
class impl<Error> final {
public:
  static Error error(repr::PtrLen repr) noexcept {
    Error error;
    error.msg = static_cast<const char *>(repr.ptr);
    error.len = repr.len;
    return error;
  }
};
} // namespace
} // namespace cxxbridge1
} // namespace rust

struct TreeNode;
struct MatchGroup;
struct Match;
struct Matches;
struct Regex;

#ifndef CXXBRIDGE1_STRUCT_TreeNode
#define CXXBRIDGE1_STRUCT_TreeNode
struct TreeNode final {
  ::rust::String title;
  ::rust::String content;
  ::std::uint32_t start;
  ::std::uint32_t end;
  ::rust::Vec<::TreeNode> children;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_TreeNode

#ifndef CXXBRIDGE1_STRUCT_MatchGroup
#define CXXBRIDGE1_STRUCT_MatchGroup
struct MatchGroup final {
  ::rust::String text;
  ::std::uint32_t start;
  ::std::uint32_t end;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_MatchGroup

#ifndef CXXBRIDGE1_STRUCT_Match
#define CXXBRIDGE1_STRUCT_Match
struct Match final {
  ::rust::Vec<::MatchGroup> groups;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_Match

#ifndef CXXBRIDGE1_STRUCT_Matches
#define CXXBRIDGE1_STRUCT_Matches
struct Matches final {
  ::rust::Vec<::rust::String> group_names;
  ::rust::Vec<::Match> matches;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_Matches

extern "C" {
::rust::repr::PtrLen cxxbridge1$regex_parse(::rust::repr::PtrLen s, bool ignore_whitespace, ::TreeNode *return$) noexcept;

::rust::repr::PtrLen cxxbridge1$regex_new(::rust::repr::PtrLen re, bool ignore_whitespace, bool case_insensitive, bool multi_line, bool dot_matches_new_line, ::rust::Box<::Regex> *return$) noexcept;

::rust::repr::PtrLen cxxbridge1$regex_match(const ::rust::Box<::Regex> &re, ::rust::repr::PtrLen text, ::Matches *return$) noexcept;

void cxxbridge1$regex_replace(const ::rust::Box<::Regex> &re, ::rust::repr::PtrLen text, ::rust::repr::PtrLen rep, ::rust::String *return$) noexcept;

void cxxbridge1$regex_split(const ::rust::Box<::Regex> &re, ::rust::repr::PtrLen text, ::rust::Vec<::rust::String> *return$) noexcept;
} // extern "C"

::TreeNode regex_parse(::rust::Str s, bool ignore_whitespace) {
  ::rust::MaybeUninit<::TreeNode> return$;
  ::rust::repr::PtrLen error$ = cxxbridge1$regex_parse(::rust::impl<::rust::Str>::repr(s), ignore_whitespace, &return$.value);
  if (error$.ptr) {
    throw ::rust::impl<::rust::Error>::error(error$);
  }
  return ::std::move(return$.value);
}

::rust::Box<::Regex> regex_new(::rust::Str re, bool ignore_whitespace, bool case_insensitive, bool multi_line, bool dot_matches_new_line) {
  ::rust::MaybeUninit<::rust::Box<::Regex>> return$;
  ::rust::repr::PtrLen error$ = cxxbridge1$regex_new(::rust::impl<::rust::Str>::repr(re), ignore_whitespace, case_insensitive, multi_line, dot_matches_new_line, &return$.value);
  if (error$.ptr) {
    throw ::rust::impl<::rust::Error>::error(error$);
  }
  return ::std::move(return$.value);
}

::Matches regex_match(const ::rust::Box<::Regex> &re, ::rust::Str text) {
  ::rust::MaybeUninit<::Matches> return$;
  ::rust::repr::PtrLen error$ = cxxbridge1$regex_match(re, ::rust::impl<::rust::Str>::repr(text), &return$.value);
  if (error$.ptr) {
    throw ::rust::impl<::rust::Error>::error(error$);
  }
  return ::std::move(return$.value);
}

::rust::String regex_replace(const ::rust::Box<::Regex> &re, ::rust::Str text, ::rust::Str rep) noexcept {
  ::rust::MaybeUninit<::rust::String> return$;
  cxxbridge1$regex_replace(re, ::rust::impl<::rust::Str>::repr(text), ::rust::impl<::rust::Str>::repr(rep), &return$.value);
  return ::std::move(return$.value);
}

::rust::Vec<::rust::String> regex_split(const ::rust::Box<::Regex> &re, ::rust::Str text) noexcept {
  ::rust::MaybeUninit<::rust::Vec<::rust::String>> return$;
  cxxbridge1$regex_split(re, ::rust::impl<::rust::Str>::repr(text), &return$.value);
  return ::std::move(return$.value);
}

extern "C" {
#ifndef CXXBRIDGE1_RUST_VEC_TreeNode
#define CXXBRIDGE1_RUST_VEC_TreeNode
void cxxbridge1$rust_vec$TreeNode$new(const ::rust::Vec<::TreeNode> *ptr) noexcept;
void cxxbridge1$rust_vec$TreeNode$drop(::rust::Vec<::TreeNode> *ptr) noexcept;
::std::size_t cxxbridge1$rust_vec$TreeNode$len(const ::rust::Vec<::TreeNode> *ptr) noexcept;
::std::size_t cxxbridge1$rust_vec$TreeNode$capacity(const ::rust::Vec<::TreeNode> *ptr) noexcept;
const ::TreeNode *cxxbridge1$rust_vec$TreeNode$data(const ::rust::Vec<::TreeNode> *ptr) noexcept;
void cxxbridge1$rust_vec$TreeNode$reserve_total(::rust::Vec<::TreeNode> *ptr, ::std::size_t cap) noexcept;
void cxxbridge1$rust_vec$TreeNode$set_len(::rust::Vec<::TreeNode> *ptr, ::std::size_t len) noexcept;
::std::size_t cxxbridge1$rust_vec$TreeNode$stride() noexcept;
#endif // CXXBRIDGE1_RUST_VEC_TreeNode

#ifndef CXXBRIDGE1_RUST_VEC_MatchGroup
#define CXXBRIDGE1_RUST_VEC_MatchGroup
void cxxbridge1$rust_vec$MatchGroup$new(const ::rust::Vec<::MatchGroup> *ptr) noexcept;
void cxxbridge1$rust_vec$MatchGroup$drop(::rust::Vec<::MatchGroup> *ptr) noexcept;
::std::size_t cxxbridge1$rust_vec$MatchGroup$len(const ::rust::Vec<::MatchGroup> *ptr) noexcept;
::std::size_t cxxbridge1$rust_vec$MatchGroup$capacity(const ::rust::Vec<::MatchGroup> *ptr) noexcept;
const ::MatchGroup *cxxbridge1$rust_vec$MatchGroup$data(const ::rust::Vec<::MatchGroup> *ptr) noexcept;
void cxxbridge1$rust_vec$MatchGroup$reserve_total(::rust::Vec<::MatchGroup> *ptr, ::std::size_t cap) noexcept;
void cxxbridge1$rust_vec$MatchGroup$set_len(::rust::Vec<::MatchGroup> *ptr, ::std::size_t len) noexcept;
::std::size_t cxxbridge1$rust_vec$MatchGroup$stride() noexcept;
#endif // CXXBRIDGE1_RUST_VEC_MatchGroup

#ifndef CXXBRIDGE1_RUST_VEC_Match
#define CXXBRIDGE1_RUST_VEC_Match
void cxxbridge1$rust_vec$Match$new(const ::rust::Vec<::Match> *ptr) noexcept;
void cxxbridge1$rust_vec$Match$drop(::rust::Vec<::Match> *ptr) noexcept;
::std::size_t cxxbridge1$rust_vec$Match$len(const ::rust::Vec<::Match> *ptr) noexcept;
::std::size_t cxxbridge1$rust_vec$Match$capacity(const ::rust::Vec<::Match> *ptr) noexcept;
const ::Match *cxxbridge1$rust_vec$Match$data(const ::rust::Vec<::Match> *ptr) noexcept;
void cxxbridge1$rust_vec$Match$reserve_total(::rust::Vec<::Match> *ptr, ::std::size_t cap) noexcept;
void cxxbridge1$rust_vec$Match$set_len(::rust::Vec<::Match> *ptr, ::std::size_t len) noexcept;
::std::size_t cxxbridge1$rust_vec$Match$stride() noexcept;
#endif // CXXBRIDGE1_RUST_VEC_Match

#ifndef CXXBRIDGE1_RUST_BOX_Regex
#define CXXBRIDGE1_RUST_BOX_Regex
::Regex *cxxbridge1$box$Regex$alloc() noexcept;
void cxxbridge1$box$Regex$dealloc(::Regex *) noexcept;
void cxxbridge1$box$Regex$drop(::rust::Box<::Regex> *ptr) noexcept;
#endif // CXXBRIDGE1_RUST_BOX_Regex
} // extern "C"

namespace rust {
inline namespace cxxbridge1 {
template <>
Vec<::TreeNode>::Vec() noexcept {
  cxxbridge1$rust_vec$TreeNode$new(this);
}
template <>
void Vec<::TreeNode>::drop() noexcept {
  return cxxbridge1$rust_vec$TreeNode$drop(this);
}
template <>
::std::size_t Vec<::TreeNode>::size() const noexcept {
  return cxxbridge1$rust_vec$TreeNode$len(this);
}
template <>
::std::size_t Vec<::TreeNode>::capacity() const noexcept {
  return cxxbridge1$rust_vec$TreeNode$capacity(this);
}
template <>
const ::TreeNode *Vec<::TreeNode>::data() const noexcept {
  return cxxbridge1$rust_vec$TreeNode$data(this);
}
template <>
void Vec<::TreeNode>::reserve_total(::std::size_t cap) noexcept {
  return cxxbridge1$rust_vec$TreeNode$reserve_total(this, cap);
}
template <>
void Vec<::TreeNode>::set_len(::std::size_t len) noexcept {
  return cxxbridge1$rust_vec$TreeNode$set_len(this, len);
}
template <>
::std::size_t Vec<::TreeNode>::stride() noexcept {
  return cxxbridge1$rust_vec$TreeNode$stride();
}
template <>
Vec<::MatchGroup>::Vec() noexcept {
  cxxbridge1$rust_vec$MatchGroup$new(this);
}
template <>
void Vec<::MatchGroup>::drop() noexcept {
  return cxxbridge1$rust_vec$MatchGroup$drop(this);
}
template <>
::std::size_t Vec<::MatchGroup>::size() const noexcept {
  return cxxbridge1$rust_vec$MatchGroup$len(this);
}
template <>
::std::size_t Vec<::MatchGroup>::capacity() const noexcept {
  return cxxbridge1$rust_vec$MatchGroup$capacity(this);
}
template <>
const ::MatchGroup *Vec<::MatchGroup>::data() const noexcept {
  return cxxbridge1$rust_vec$MatchGroup$data(this);
}
template <>
void Vec<::MatchGroup>::reserve_total(::std::size_t cap) noexcept {
  return cxxbridge1$rust_vec$MatchGroup$reserve_total(this, cap);
}
template <>
void Vec<::MatchGroup>::set_len(::std::size_t len) noexcept {
  return cxxbridge1$rust_vec$MatchGroup$set_len(this, len);
}
template <>
::std::size_t Vec<::MatchGroup>::stride() noexcept {
  return cxxbridge1$rust_vec$MatchGroup$stride();
}
template <>
Vec<::Match>::Vec() noexcept {
  cxxbridge1$rust_vec$Match$new(this);
}
template <>
void Vec<::Match>::drop() noexcept {
  return cxxbridge1$rust_vec$Match$drop(this);
}
template <>
::std::size_t Vec<::Match>::size() const noexcept {
  return cxxbridge1$rust_vec$Match$len(this);
}
template <>
::std::size_t Vec<::Match>::capacity() const noexcept {
  return cxxbridge1$rust_vec$Match$capacity(this);
}
template <>
const ::Match *Vec<::Match>::data() const noexcept {
  return cxxbridge1$rust_vec$Match$data(this);
}
template <>
void Vec<::Match>::reserve_total(::std::size_t cap) noexcept {
  return cxxbridge1$rust_vec$Match$reserve_total(this, cap);
}
template <>
void Vec<::Match>::set_len(::std::size_t len) noexcept {
  return cxxbridge1$rust_vec$Match$set_len(this, len);
}
template <>
::std::size_t Vec<::Match>::stride() noexcept {
  return cxxbridge1$rust_vec$Match$stride();
}
template <>
::Regex *Box<::Regex>::allocation::alloc() noexcept {
  return cxxbridge1$box$Regex$alloc();
}
template <>
void Box<::Regex>::allocation::dealloc(::Regex *ptr) noexcept {
  cxxbridge1$box$Regex$dealloc(ptr);
}
template <>
void Box<::Regex>::drop() noexcept {
  cxxbridge1$box$Regex$drop(this);
}
} // namespace cxxbridge1
} // namespace rust
