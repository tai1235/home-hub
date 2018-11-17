/*
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 */

#ifndef ADDON_UTILS_H_
#define ADDON_UTILS_H_

#include <nan.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace artik {

template<typename T>
class Optional {
 public:
  Optional() : m_val(nullptr) {}
  Optional(const T& val) : m_val(new T(val)) {}

  ~Optional() { if (m_val) delete m_val; }

  explicit operator bool() const { return m_val != nullptr; }
  bool has_value() const { return m_val != nullptr; }

  T& value() { return *m_val; }
  const T& value() const { return *m_val; }

 private:
  T* m_val;
};

namespace details {

template<typename Q>
struct converter {
  template<typename T = Q>
  static typename std::enable_if<
    !std::is_same<T, uint16_t>::value
  && !std::is_same<T, unsigned char>::value, Optional<T>>::type
  js_type_to_cpp(v8::Local<v8::Value> val) {
    Nan::Maybe<T> maybe_val = Nan::To<T>(val);

    if (maybe_val.IsNothing())
      return Optional<T>();

    return Optional<T>(maybe_val.FromJust());
  }

  template<typename T = Q>
  static typename std::enable_if<
      std::is_same<T, uint16_t>::value ||
      std::is_same<T, unsigned char>::value, Optional<T>>::type
      js_type_to_cpp(v8::Local<v8::Value> val) {
    auto value = converter<uint32_t>::js_type_to_cpp(val);

    if (value)
      return Optional<T>();

    return Optional<T>(value.value());
  }
};

template<>
struct converter<std::string> {
  static Optional<std::string> js_type_to_cpp(v8::Local<v8::Value> val);
};

template<>
struct converter<v8::Local<v8::Array>> {
  static Optional<v8::Local<v8::Array>> js_type_to_cpp(
      v8::Local<v8::Value> val);
};

template<>
struct converter<v8::Local<v8::Function>> {
  static Optional<v8::Local<v8::Function>> js_type_to_cpp(
      v8::Local<v8::Value> val);
};

template<typename T>
struct converter<std::vector<T>> {
  static Optional<std::vector<T>> js_type_to_cpp(v8::Local<v8::Value> val) {
    std::vector<T> vect;
    if (!val->IsArray())
      return Optional<std::vector<T>>();

    v8::Local<v8::Array> array = val.As<v8::Array>();

    for (unsigned int i = 0; i < array->Length(); ++i) {
      v8::Local<v8::Value> js_val = Nan::Get(array, i).ToLocalChecked();

      Optional<T> oval = converter<T>::js_type_to_cpp(js_val);
      if (!oval)
        return Optional<std::vector<T>>();

      vect.push_back(oval.value());
    }

    return Optional<std::vector<T>>(vect);
  }
};

template <typename T>
struct CustomCompare {
  static bool compare(const T& val1, const T& val2) {
    return val1 == val2;
  }
};

template <>
struct CustomCompare<const char *> {
  static bool compare(const char* string1, const char* string2) {
    return !strcmp(string1, string2);
  }
};
}  // namespace details

template <typename T, typename Container>
static Optional<T> to_artik_parameter(const Container& container,
    typename Container::value_type value) {
  typename Container::const_iterator cit = std::find_if(
      std::begin(container),
      std::end(container),
      [&value](const typename Container::value_type& val) {
        return
          details::CustomCompare<typename Container::value_type>::compare(val,
              value);
      });

  if (cit == std::end(container))
    return Optional<T>();

  return Optional<T>(static_cast<T>(cit - std::begin(container)));
}

template<typename T>
Optional<T> js_type_to_cpp(v8::Local<v8::Value> val) {
  return details::converter<T>::js_type_to_cpp(val);
}

template<typename T>
Optional<T> js_object_attribute_to_cpp(v8::Local<v8::Value> obj,
    const std::string& field) {
  auto val = js_object_attribute_to_cpp<v8::Local<v8::Value>>(obj, field);

  if (!val)
    return Optional<T>();

  return js_type_to_cpp<T>(val.value());
}

template<>
Optional<v8::Local<v8::Value>>
js_object_attribute_to_cpp<v8::Local<v8::Value>>(v8::Local<v8::Value> obj,
    const std::string& field);

}  // namespace artik

#endif  // ADDON_UTILS_H_
