#include <cstdio>
#include <iostream>
#include <memory>

template <typename _Tp>
class shared_ptr {
 public:
  /**** default constructor ****/
  constexpr shared_ptr() noexcept : _M_ptr(nullptr), _M_ref_count(new int(0)) {}

  /**** constructor from raw pointer ****/
  template <typename _Yp>
  explicit shared_ptr(_Yp *__p) : _M_ptr(__p), _M_ref_count(new int(1)) {}

  /**** destructor ****/
  ~shared_ptr() {
    if (_M_ref_count && --(*_M_ref_count) == 0) {
      delete _M_ptr;
      delete _M_ref_count;
    }
  }

  /**** copy constructor ****/
  template <typename _Yp>
  shared_ptr(const shared_ptr<_Yp> &__r) noexcept
      : _M_ptr(__r._M_ptr), _M_ref_count(__r._M_ref_count) {
    (*_M_ref_count)++;
  }

  /**** move constructor ****/
  template <typename _Yp>
  shared_ptr(shared_ptr<_Yp> &&__r) noexcept
      : _M_ptr(__r._M_ptr), _M_ref_count(__r._M_ref_count) {
    __r._M_ptr = nullptr;
    __r._M_ref_count = new int(0);
  }

  /**** copy assignment operator ****/
  template <typename _Yp>
  shared_ptr &operator=(const shared_ptr<_Yp> &__r) noexcept {
    if (this != &__r) {
      /* Decrement the old reference count and delete if it's zero */
      if (_M_ref_count && --(*_M_ref_count) == 0) {
        delete _M_ptr;
        delete _M_ref_count;
      }
      /* Copy the new data */
      _M_ptr = __r._M_ptr;
      _M_ref_count = __r._M_ref_count;
      (*_M_ref_count)++;
    }
    return *this;
  }

  /**** move assignment operator ****/
  template <typename _Yp>
  shared_ptr &operator=(shared_ptr<_Yp> &&__r) noexcept {
    if (this != &__r) {
      std::swap(_M_ptr, __r._M_ptr);
      std::swap(_M_ref_count, __r._M_ref_count);
    }
    return *this;
  }

  /* The swap operation needs to ensure that the types match exactly */
  void swap(shared_ptr<_Tp> &__r) noexcept {
    std::swap(_M_ptr, __r._M_ptr);
    std::swap(_M_ref_count, __r._M_ref_count);
  }

  _Tp *operator->() { return _M_ptr; }

  _Tp &operator*() { return *_M_ptr; }

  _Tp *get() { return _M_ptr; }

  void reset() noexcept { shared_ptr().swap(*this); }

  template <typename _Yp>
  void reset(_Yp *__p) {
    if (__p != nullptr && __p != _M_ptr) {
      shared_ptr(__p).swap(*this);
    } else {
      throw std::logic_error("ptr is illegal");
    }
  }

  int use_count() { return *_M_ref_count; }

  shared_ptr(const shared_ptr &) noexcept = default;
  shared_ptr &operator=(const shared_ptr &) noexcept = default;

 private:
  /* Contained pointer. The raw instance pointer is stored here.*/
  _Tp *_M_ptr;
  /* Reference counter. */
  int *_M_ref_count;
};
