#include <sys/cdefs.h>
#include <sys/types.h>

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <stdexcept>
#include <type_traits>

class _sp_counted_base {
 public:
  _sp_counted_base() noexcept : _use_count(1), _weak_count(1) {}

  virtual ~_sp_counted_base() noexcept {}

  /**** called when _use_count drops to zero, to release the resources managed
   * by *this ****/
  virtual void _use_dispose() noexcept = 0;

  /**** called when _weak_count drops to zero ****/
  virtual void _weak_destroy() { delete this; }

  /**** increment ****/
  void _use_add_ref() { _use_count.fetch_add(1, std::memory_order_relaxed); }

  void _weak_add_ref() { _weak_count.fetch_add(1, std::memory_order_relaxed); }

  bool _try_use_add_ref_nothrow() noexcept;

  void _try_use_add_ref() {
    if (!_try_use_add_ref_nothrow())
      throw std::logic_error("Cannot increment use_count as it is zero");
  }
  /**** decrement ****/
  void _use_sub_ref() noexcept;

  void _use_sub_ref_last() noexcept;

  void _weak_sub_ref() noexcept {
    if (_weak_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
      _weak_destroy();
  }

  int32_t _get_use_count() { return _use_count.load(); }

 private:
  _sp_counted_base(const _sp_counted_base &) = delete;
  _sp_counted_base &operator=(const _sp_counted_base &) = delete;

  std::atomic_int32_t _use_count;
  std::atomic_int32_t _weak_count;
};

inline bool _sp_counted_base::_try_use_add_ref_nothrow() noexcept {
  int expected = _use_count.load();
  while (expected != 0) {
    if (_use_count.compare_exchange_weak(expected, expected + 1,
                                         std::memory_order_relaxed))
      return true;
  }
  return false;
}

inline void _sp_counted_base::_use_sub_ref() noexcept {
  if (_use_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
    _use_sub_ref_last();
}

inline void _sp_counted_base::_use_sub_ref_last() noexcept {
  _use_dispose();
  if (_weak_count.fetch_sub(1, std::memory_order_acq_rel) == 1) _weak_destroy();
}

/* forward declarations */
template <typename _Tp>
class __shared_ptr;

template <typename _Tp>
class __weak_ptr;

template <typename _Tp>
class shared_ptr;

template <typename _Tp>
class weak_ptr;

class __shared_count;
class __weak_count;

template <typename _Ptr>
class _sp_counted_ptr : public _sp_counted_base {
 public:
  explicit _sp_counted_ptr(_Ptr __p) noexcept : _M_ptr(__p) {}

  virtual void _use_dispose() noexcept { delete _M_ptr; }

  virtual void _weak_destroy() noexcept { delete this; }

 private:
  _sp_counted_ptr(const _sp_counted_ptr &) = delete;
  _sp_counted_ptr &operator=(const _sp_counted_ptr &) = delete;

  _Ptr _M_ptr;
};

/*  specialization for nullptr_t */
template <>
inline void _sp_counted_ptr<std::nullptr_t>::_use_dispose() noexcept {}

class __shared_count {
 public:
  /**** constructor ****/
  constexpr __shared_count() noexcept : _M_pi(0) {}

  template <typename _Ptr>
  explicit __shared_count(_Ptr __p) : _M_pi(0) {
    _M_pi = new _sp_counted_ptr<_Ptr>(__p);
  }

  /**** destructor ****/
  ~__shared_count() noexcept {
    if (_M_pi != nullptr) _M_pi->_use_sub_ref();
  }

  /**** assignment constructor ****/
  __shared_count(const __shared_count &__r) noexcept : _M_pi(__r._M_pi) {
    if (_M_pi != nullptr) _M_pi->_use_add_ref();
  }

  __shared_count &operator=(const __shared_count &__r) noexcept {
    _sp_counted_base *__tmp = __r._M_pi;
    if (__tmp != _M_pi) {
      if (__tmp != nullptr) __tmp->_use_add_ref();
      if (_M_pi != nullptr) _M_pi->_use_sub_ref();
      _M_pi = __tmp;
    }
    return *this;
  }

  void __swap(__shared_count &__r) noexcept {
    _sp_counted_base *__tmp = __r._M_pi;
    __r._M_pi = _M_pi;
    _M_pi = __tmp;
  }

  int32_t _get_use_count() const noexcept {
    return _M_pi != nullptr ? _M_pi->_get_use_count() : 0;
  }

  bool __unique() const noexcept { return this->_get_use_count() == 1; }

  /**** check whether this < rhs ****/
  bool __less(const __shared_count &__rhs) const noexcept {
    return std::less<_sp_counted_base *>{}(this->_M_pi, __rhs._M_pi);
  }

 private:
  friend class __weak_count;
  _sp_counted_base *_M_pi;
};

class __weak_count {
 public:
  /**** constructor ****/
  constexpr __weak_count() noexcept : _M_pi(nullptr) {}

  /**** assignment constructor ****/
  __weak_count(const __shared_count &__r) noexcept : _M_pi(__r._M_pi) {
    if (_M_pi != nullptr) _M_pi->_weak_add_ref();
  }

  __weak_count(const __weak_count &__r) noexcept : _M_pi(__r._M_pi) {
    if (_M_pi != nullptr) _M_pi->_weak_add_ref();
  }

  /**** move constructor ****/
  __weak_count(__weak_count &&__r) noexcept : _M_pi((__r._M_pi)) {
    __r._M_pi = nullptr;
  }

  /**** destructor ****/
  ~__weak_count() noexcept {
    if (_M_pi != nullptr) _M_pi->_weak_sub_ref();
  }

  /**** assignment operator ****/
  __weak_count &operator=(const __shared_count &__r) noexcept {
    _sp_counted_base *__tmp = __r._M_pi;
    if (__tmp != nullptr) __tmp->_weak_add_ref();
    if (_M_pi != nullptr) _M_pi->_weak_sub_ref();
    _M_pi = __tmp;
    return *this;
  }

  __weak_count &operator=(const __weak_count &__r) noexcept {
    _sp_counted_base *__tmp = __r._M_pi;
    if (__tmp != nullptr) __tmp->_weak_add_ref();
    if (_M_pi != nullptr) _M_pi->_weak_sub_ref();
    _M_pi = __tmp;
    return *this;
  }

  /**** move assignment operator ****/
  __weak_count &operator=(__weak_count &&__r) noexcept {
    if (_M_pi != nullptr) _M_pi->_weak_sub_ref();
    _M_pi = __r._M_pi;
    __r._M_pi = nullptr;
    return *this;
  }

  void __swap(__weak_count &__r) noexcept {
    _sp_counted_base *__tmp = __r._M_pi;
    __r._M_pi = _M_pi;
    _M_pi = __tmp;
  }

  int32_t _get_use_count() const noexcept {
    return _M_pi != nullptr ? _M_pi->_get_use_count() : 0;
  }

  /**** check whether this < rhs ****/
  bool __less(const __shared_count &__rhs) const noexcept {
    return std::less<_sp_counted_base *>{}(this->_M_pi, __rhs._M_pi);
  }

  bool __less(const __weak_count &__rhs) const noexcept {
    return std::less<_sp_counted_base *>{}(this->_M_pi, __rhs._M_pi);
  }

 private:
  friend class __shared_count;
  _sp_counted_base *_M_pi;
};

template <typename _Tp>
class __shared_ptr {
 public:
  using element_type = typename std::remove_extent<_Tp>;
  /**** default constructor ****/
  constexpr __shared_ptr() noexcept : _M_ptr(0), _M_ref_count() {}

  template <typename _Yp>
  __shared_ptr(const __shared_ptr<_Yp> &__r) noexcept
      : _M_ptr(__r._M_ptr), _M_ref_count(__r._M_ref_count) {}

  template <typename _Yp>
  __shared_ptr(__shared_ptr<_Yp> &&__r) noexcept
      : _M_ptr(__r._M_ptr), _M_ref_count() {
    _M_ref_count.__swap(__r._M_ref_count);
    __r._M_ptr = nullptr;
  }

  template <typename _Yp>
  __shared_ptr(const __weak_ptr<_Yp> &__r) noexcept
      : _M_ref_count(__r._M_ref_count) {
    _M_ptr = __r._M_ptr;
  }

  __shared_ptr(const __shared_ptr &) noexcept = default;
  __shared_ptr &operator=(const __shared_ptr &) noexcept = default;
  ~__shared_ptr() = default;

 private:
  /*Contained pointer. The raw instance pointer is stored here.*/
  element_type *_M_ptr;
  /* Reference counter. */
  __shared_count _M_ref_count;
};
