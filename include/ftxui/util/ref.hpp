#ifndef FTXUI_UTIL_REF_HPP
#define FTXUI_UTIL_REF_HPP

#include <ftxui/screen/string.hpp>
#include <string>
#include <string_view>

namespace ftxui {

/// @brief An adapter. Own or reference an immutable object.
template <typename T>
class ConstRef {
 public:
  constexpr ConstRef() = default;
  constexpr ConstRef(T t) : owned_(t) {}
  constexpr ConstRef(const T* t) : address_(t) {}
  constexpr const T& operator*() const noexcept {
    return address_ ? *address_ : owned_;
  }
  constexpr const T& operator()() const noexcept {
    return address_ ? *address_ : owned_;
  }
  constexpr const T* operator->() const noexcept {
    return address_ ? address_ : &owned_;
  }

 private:
  T owned_{};
  const T* address_{nullptr};
};

/// @brief An adapter. Own or reference an mutable object.
template <typename T>
class Ref {
 public:
  constexpr Ref() = default;
  Ref(T t) : owned_(t) {}
  Ref(T* t) : address_(t) {}
  constexpr T& operator*() noexcept { return address_ ? *address_ : owned_; }
  constexpr T& operator()() noexcept { return address_ ? *address_ : owned_; }
  constexpr T* operator->() noexcept { return address_ ? address_ : &owned_; }

 private:
  T owned_{};
  T* address_{nullptr};
};

/// @brief An adapter. Own or reference a constant string. For convenience, this
/// class convert multiple mutable string toward a shared representation.
class StringRef {
 public:
  StringRef(std::string* ref) : address_(ref) {}
  StringRef(std::string ref) : owned_(std::move(ref)) {}
  StringRef(const wchar_t* ref) : StringRef(to_string(std::wstring(ref))) {}
  StringRef(const char* ref) : StringRef(std::string(ref)) {}
  constexpr std::string& operator*() noexcept {
    return address_ ? *address_ : owned_;
  }
  constexpr std::string* operator->() noexcept {
    return address_ ? address_ : &owned_;
  }

 private:
  std::string owned_{};
  std::string* address_{nullptr};
};

/// @brief An adapter. Own or reference a constant string. For convenience, this
/// class convert multiple mutable string toward a shared representation.
class WideStringRef {
 public:
  WideStringRef(std::wstring* ref) : address_(ref) {}
  WideStringRef(std::wstring ref) : owned_(std::move(ref)) {}
  WideStringRef(const wchar_t* ref) : WideStringRef(std::wstring(ref)) {}
  WideStringRef(const char* ref)
      : WideStringRef(to_wstring(std::string(ref))) {}
  constexpr std::wstring& operator*() noexcept {
    return address_ ? *address_ : owned_;
  }
  constexpr std::wstring* operator->() noexcept {
    return address_ ? address_ : &owned_;
  }

 private:
  std::wstring owned_{};
  std::wstring* address_{nullptr};
};

/// @brief An adapter. Own or reference a constant string. For convenience, this
/// class convert multiple immutable string toward a shared representation.
class ConstStringRef {
 public:
  ConstStringRef(const std::string* ref) : address_(ref) {}
  ConstStringRef(const std::string_view* ref)
      : ConstStringRef(to_wstring(std::string(ref->data()))) {}
  ConstStringRef(const std::wstring* ref) : ConstStringRef(to_string(*ref)) {}
  ConstStringRef(std::string ref) : owned_(std::move(ref)) {}
  ConstStringRef(std::wstring ref) : ConstStringRef(to_string(ref)) {}
  ConstStringRef(std::string_view ref)
      : ConstStringRef(to_wstring(std::string(ref.data(), ref.size()))) {}
  ConstStringRef(const wchar_t* ref) : ConstStringRef(std::wstring(ref)) {}
  ConstStringRef(const char* ref)
      : ConstStringRef(to_wstring(std::string(ref))) {}
  constexpr const std::string& operator*() const noexcept {
    return address_ ? *address_ : owned_;
  }
  constexpr const std::string* operator->() const noexcept {
    return address_ ? address_ : &owned_;
  }

 private:
  const std::string owned_{};
  const std::string* address_{nullptr};
};

/// @brief An adapter. Reference a list of strings.
class ConstStringListRef {
 public:
  ConstStringListRef(const std::vector<std::string>* ref) : ref_(ref) {}
  ConstStringListRef(const std::vector<std::wstring>* ref) : ref_wide_(ref) {}

  size_t size() const noexcept {
    return ref_ ? ref_->size() : ref_wide_->size();
  }
  std::string operator[](size_t i) const noexcept {
    return ref_ ? (*ref_)[i] : to_string((*ref_wide_)[i]);
  }

 private:
  const std::vector<std::string>* ref_{nullptr};
  const std::vector<std::wstring>* ref_wide_{nullptr};
};

}  // namespace ftxui

#endif /* end of include guard: FTXUI_UTIL_REF_HPP */

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
