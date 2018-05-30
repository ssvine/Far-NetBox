#pragma once

#include <function2.hpp>
#include <nbglobals.h>

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

  explicit Property(T &Value):
    propertyBase<T>(Value)
  {}

  T &operator=(const Property &Value)
  {
    this->obj = Value.obj;
    return this->obj;
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

template <typename T>
class ROProperty
{
CUSTOM_MEM_ALLOCATION_IMPL
private:
  typedef fu2::function<T() const> TGetValueFunctor;
  TGetValueFunctor _getter;
public:
  explicit ROProperty(const TGetValueFunctor &Getter) :
    _getter(Getter)
  {
    Expects(_getter != nullptr);
  }
  T operator()() const
  {
    Expects(_getter);
    return _getter();
  }
  operator T() const
  {
    Expects(_getter);
    return _getter();
  }
  const T operator->() const
  {
    Expects(_getter);
    return _getter();
  }
  T operator->()
  {
    Expects(_getter);
    return _getter();
  }
  friend bool inline operator==(const ROProperty &lhs, const ROProperty &rhs)
  {
    Expects(lhs._getter);
    Expects(rhs._getter);
    return lhs._getter() == rhs._getter();
  }
  friend bool inline operator==(const ROProperty &lhs, const T &rhs)
  {
    Expects(lhs._getter);
    return lhs._getter() == rhs;
  }
  friend bool inline operator!=(const ROProperty &lhs, const ROProperty &rhs)
  {
    Expects(lhs._getter);
    Expects(rhs._getter);
    return lhs._getter() != rhs._getter();
  }
  friend bool inline operator!=(ROProperty &lhs, const T &rhs)
  {
    Expects(lhs._getter);
    return lhs._getter() != rhs;
  }
};

template <typename T>
class RWProperty
{
CUSTOM_MEM_ALLOCATION_IMPL
private:
  typedef fu2::function<T() const> TGetValueFunctor;
  typedef fu2::function<void(T)> TSetValueFunctor;
  TGetValueFunctor _getter;
  TSetValueFunctor _setter;
public:
  explicit RWProperty(const TGetValueFunctor &Getter, const TSetValueFunctor &Setter) :
    _getter(Getter),
    _setter(Setter)
  {
    Expects(_getter != nullptr);
    Expects(_setter != nullptr);
  }
  T operator()() const
  {
    Expects(_getter);
    return _getter();
  }
  operator T() const
  {
    Expects(_getter);
    return _getter();
  }
  /*operator T&() const
  {
    Expects(_getter);
    return _getter();
  }*/
  T operator->() const
  {
    Expects(_getter);
    return _getter();
  }
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
  bool operator==(T Value) const
  {
    Expects(_getter);
    return _getter() == Value;
  }
  friend bool inline operator==(const RWProperty &lhs, const RWProperty &rhs)
  {
    Expects(lhs._getter);
    return lhs._getter == rhs._getter && lhs._setter == rhs._setter;
  }
  friend bool inline operator!=(RWProperty &lhs, const T &rhs)
  {
    Expects(lhs._getter);
    return lhs._getter() != rhs;
  }
};
