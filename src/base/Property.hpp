#pragma once

//#include <function2.hpp>
#include <nbglobals.h>
#include <FastDelegate.h>
//#include <FastDelegateBind.h>

template <typename T>
class propertyBase
{
CUSTOM_MEM_ALLOCATION_IMPL
public:
  T dummy = T();
  T &obj  = dummy;

  explicit propertyBase(T &Value):
    obj(Value)
  {}
};

// Read/Write specialization
template <typename T, bool canWrite = true, bool isPod = true>
class Property : private propertyBase<T>
{
CUSTOM_MEM_ALLOCATION_IMPL
public:
  Property() = default;

  Property(const Property<T, true, true> &Value):
    propertyBase<T>(Value.obj)
  {}

  explicit Property(T &Value):
    propertyBase<T>(Value)
  {}

  T &operator=(const Property<T, true, true> &Value)
  {
    this->obj = Value.obj;
    return this->obj;
  }

  T &operator=(const T &Value)
  {
    this->obj = Value;
    return this->obj;
  }

  T &operator=(T &Value)
  {
    this->obj = Value;

    return this->obj;
  }

  operator T &() const
  {
    return this->obj;
  }

};

// Read only specialization
template <typename T>
class Property<T, false, true> : private propertyBase<T>
{
CUSTOM_MEM_ALLOCATION_IMPL
public:
  Property() = default;

  Property(const Property<T, false, true> &Value):
    propertyBase<T>(Value.obj)
  {}

  explicit Property(T &Value) :
    propertyBase<T>(Value)
  {}

  Property &operator=(const Property &Value)
  {
    this->obj = Value.obj;
    return *this;
  }

  operator T() const
  {
    return this->obj;
  }

};

// Read/Write non-pod specialization
template <typename T>
class Property<T, true, false> : public T
{
CUSTOM_MEM_ALLOCATION_IMPL
public:
  using T::T;

  explicit Property(const T &Value):
    T(Value)
  {}
};

// Read only non-pod specialization
template <typename T>
class Property<T, false, false> : public T
{
  const T *const _obj = this;
public:
  using T::T;

  explicit Property(const T &Value):
    T(Value)
  {}

  const T *const operator->() const
  {
    return this->_obj;
  }
};

template <typename T>
using roProperty = Property<T, false, std::is_pod<T>::value>;

template <typename T>
using rwProperty = Property<T, true, std::is_pod<T>::value>;

// 40 bytes using fu2::function
// 16 bytes using FastDelegate
template <typename T>
class ROProperty
{
CUSTOM_MEM_ALLOCATION_IMPL
private:
//  typedef fu2::function<T() const> TGetValueFunctor;
  using TGetValueFunctor = fastdelegate::FastDelegate0<T>;
  TGetValueFunctor _getter;
public:
  ROProperty() = delete;
  explicit ROProperty(const TGetValueFunctor &Getter) noexcept :
    _getter(Getter)
  {
    Expects(_getter != nullptr);
  }
  ROProperty(const ROProperty&) = default;
  ROProperty(ROProperty&&) = default;
  ROProperty& operator=(const ROProperty&) = default;
  ROProperty& operator=(ROProperty&&) = default;
//  ROProperty(const T& in) : data(in) {}
//  ROProperty(T&& in) : data(std::forward<T>(in)) {}
  constexpr T operator()() const
  {
    Expects(_getter);
    return _getter();
  }
  constexpr operator T() const
  {
    Expects(_getter);
    return _getter();
  }
  constexpr const T operator->() const
  {
    Expects(_getter);
    return _getter();
  }
  constexpr T operator->()
  {
    Expects(_getter);
    return _getter();
  }
  constexpr decltype(auto) operator*() const { return *_getter(); }

  friend bool constexpr inline operator==(const ROProperty &lhs, const ROProperty &rhs)
  {
    Expects(lhs._getter);
    Expects(rhs._getter);
    return lhs._getter() == rhs._getter();
  }
  friend bool constexpr inline operator==(const ROProperty &lhs, const T &rhs)
  {
    Expects(lhs._getter);
    return lhs._getter() == rhs;
  }
  friend bool constexpr inline operator!=(const ROProperty &lhs, const ROProperty &rhs)
  {
    Expects(lhs._getter);
    Expects(rhs._getter);
    return lhs._getter() != rhs._getter();
  }
  friend bool constexpr inline operator!=(ROProperty &lhs, const T &rhs)
  {
    Expects(lhs._getter);
    return lhs._getter() != rhs;
  }
};

template <typename T>
class ROPropertySimple //8 bytes
{
CUSTOM_MEM_ALLOCATION_IMPL
private:
  const T *_value{nullptr};
public:
  ROPropertySimple() = delete;
  explicit ROPropertySimple(const T *Value) noexcept :
    _value(Value)
  {
    Expects(_value != nullptr);
  }
  ROPropertySimple(const ROPropertySimple&) = default;
  ROPropertySimple(ROPropertySimple&&) = default;
  ROPropertySimple& operator=(const ROPropertySimple&) = default;
  ROPropertySimple& operator=(ROPropertySimple&&) = default;
  constexpr T operator()() const
  {
    Expects(_value);
    return *_value;
  }
  constexpr operator T() const
  {
    Expects(_value);
    return *_value;
  }
  constexpr const T operator->() const
  {
    Expects(_value);
    return _value();
  }
  constexpr T operator->()
  {
    Expects(_value);
    return *_value;
  }
  constexpr decltype(auto) operator*() const { return *_value; }

  friend bool constexpr inline operator==(const ROPropertySimple &lhs, const ROPropertySimple &rhs)
  {
    Expects(lhs._value);
    Expects(rhs._value);
    return *lhs._value == *rhs._value;
  }
  friend bool constexpr inline operator==(const ROPropertySimple &lhs, const T &rhs)
  {
    Expects(lhs._value);
    return *lhs._value == rhs;
  }
  friend bool constexpr inline operator!=(const ROPropertySimple &lhs, const ROPropertySimple &rhs)
  {
    Expects(lhs._value);
    Expects(rhs._value);
    return *lhs._value != *rhs._value;
  }
  friend bool constexpr inline operator!=(ROPropertySimple &lhs, const T &rhs)
  {
    Expects(lhs._value);
    return *lhs._value != rhs;
  }
};

// 80 bytes using fu2::function
// 32 bytes using FastDelegate
template <typename T>
class RWProperty
{
CUSTOM_MEM_ALLOCATION_IMPL
private:
//  typedef fu2::function<T() const> TGetValueFunctor;
//  typedef fu2::function<void(T)> TSetValueFunctor;
  using TGetValueFunctor = fastdelegate::FastDelegate0<T>;
  using TSetValueFunctor = fastdelegate::FastDelegate1<void, T>;
  TGetValueFunctor _getter;
  TSetValueFunctor _setter;
public:
  RWProperty() = delete;
  explicit RWProperty(const TGetValueFunctor &Getter, const TSetValueFunctor &Setter) noexcept :
    _getter(Getter),
    _setter(Setter)
  {
    Expects(_getter != nullptr);
    Expects(_setter != nullptr);
  }
  RWProperty(const RWProperty&) = default;
  RWProperty(RWProperty&&) = default;
  RWProperty& operator=(const RWProperty&) = default;
  RWProperty& operator=(RWProperty&&) = default;
//  RWProperty(const T& in) : data(in) {}
//  RWProperty(T&& in) : data(std::forward<T>(in)) {}
//  T const& get() const {
//      return data;
//  }

//  T&& unwrap() && {
//      return std::move(data);
//  }
  constexpr T operator()() const
  {
    Expects(_getter);
    return _getter();
  }
  constexpr operator T() const
  {
    Expects(_getter);
    return _getter();
  }
  /*operator T&() const
  {
    Expects(_getter);
    return _getter();
  }*/
  constexpr T operator->() const
  {
    return _getter();
  }
  constexpr decltype(auto) operator*() const { return *_getter(); }
  void operator()(const T &Value)
  {
    Expects(_setter);
    _setter(Value);
  }
  void operator=(T Value)
  {
    Expects(_setter);
    _setter(Value);
  }
  constexpr bool operator==(T Value) const
  {
    Expects(_getter);
    return _getter() == Value;
  }
  friend bool inline operator==(const RWProperty &lhs, const RWProperty &rhs)
  {
    Expects(lhs._getter);
    return (lhs._getter == rhs._getter) && (lhs._setter == rhs._setter);
  }
  friend bool inline operator!=(RWProperty &lhs, const T &rhs)
  {
    Expects(lhs._getter);
    return lhs._getter() != rhs;
  }
};

template <typename T>
class RWProperty2
{
CUSTOM_MEM_ALLOCATION_IMPL
private:
  T *_value{nullptr};
public:
  RWProperty2() = delete;
  explicit RWProperty2(T *Value) noexcept :
    _value(Value)
  {
    Expects(_value != nullptr);
  }
  RWProperty2(const RWProperty2&) = default;
  RWProperty2(RWProperty2&&) = default;
  RWProperty2& operator=(const RWProperty2&) = default;
  RWProperty2& operator=(RWProperty2&&) = default;
  constexpr T operator()() const
  {
    Expects(_value);
    return *_value;
  }
  constexpr operator T() const
  {
    Expects(_value);
    return *_value;
  }
  constexpr const T operator->() const
  {
    Expects(_value);
    return _value();
  }
  constexpr T operator->()
  {
    Expects(_value);
    return *_value;
  }
  constexpr decltype(auto) operator*() const { return *_value; }

  void operator()(const T &Value)
  {
    Expects(_value);
    *_value = Value;
  }
  RWProperty2& operator=(const T Value)
  {
    Expects(_value);
    *_value = Value;
    return *this;
  }

  friend bool constexpr inline operator==(const RWProperty2 &lhs, const RWProperty2 &rhs)
  {
    Expects(lhs._value);
    Expects(rhs._value);
    return *lhs._value == *rhs._value;
  }
  friend bool constexpr inline operator==(const RWProperty2 &lhs, const T &rhs)
  {
    Expects(lhs._value);
    return *lhs._value == rhs;
  }
  friend bool constexpr inline operator!=(const RWProperty2 &lhs, const RWProperty2 &rhs)
  {
    Expects(lhs._value);
    Expects(rhs._value);
    return *lhs._value != *rhs._value;
  }
  friend bool constexpr inline operator!=(RWProperty2 &lhs, const T &rhs)
  {
    Expects(lhs._value);
    return *lhs._value != rhs;
  }
};

template <typename T>
class RWPropertySimple
{
CUSTOM_MEM_ALLOCATION_IMPL
private:
  using TSetValueFunctor = fastdelegate::FastDelegate1<void, T>;
  T *_value{nullptr};
  TSetValueFunctor _setter;
public:
  RWPropertySimple() = delete;
  explicit RWPropertySimple(T *Value, const TSetValueFunctor &Setter) noexcept :
    _value(Value),
    _setter(Setter)
  {
    Expects(_value != nullptr);
    Expects(_setter != nullptr);
  }
  RWPropertySimple(const RWPropertySimple&) = default;
  RWPropertySimple(RWPropertySimple&&) = default;
  RWPropertySimple& operator=(const RWPropertySimple&) = default;
  RWPropertySimple& operator=(RWPropertySimple&&) = default;
  constexpr T operator()() const
  {
    Expects(_value);
    return *_value;
  }
  constexpr operator T() const
  {
    Expects(_value);
    return *_value;
  }
  constexpr const T operator->() const
  {
    Expects(_value);
    return _value();
  }
  constexpr T operator->()
  {
    Expects(_value);
    return *_value;
  }
  constexpr decltype(auto) operator*() const { return *_value; }

  void operator()(const T &Value)
  {
    Expects(_setter);
    _setter(Value);
  }
  void operator=(T Value)
  {
    Expects(_setter);
    _setter(Value);
  }

  friend bool constexpr inline operator==(const RWPropertySimple &lhs, const RWPropertySimple &rhs)
  {
    Expects(lhs._value);
    Expects(rhs._value);
    return *lhs._value == *rhs._value;
  }
  friend bool constexpr inline operator==(const RWPropertySimple &lhs, const T &rhs)
  {
    Expects(lhs._value);
    return *lhs._value == rhs;
  }
  friend bool constexpr inline operator!=(const RWPropertySimple &lhs, const RWPropertySimple &rhs)
  {
    Expects(lhs._value);
    Expects(rhs._value);
    return *lhs._value != *rhs._value;
  }
  friend bool constexpr inline operator!=(RWPropertySimple &lhs, const T &rhs)
  {
    Expects(lhs._value);
    return *lhs._value != rhs;
  }
};

//template<int s> struct CheckSizeT;
//CheckSizeT<sizeof(ROProperty<int>)> checkSize;
//template<int s> struct CheckSizeT;
//CheckSizeT<sizeof(ROPropertySimple<double>)> checkSize;