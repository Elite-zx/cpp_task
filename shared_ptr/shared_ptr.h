#include <cstddef>
#include <functional>
#include <string_view>

#include "shared_ptr_base.h"

template <typename _Tp>
class shared_ptr : public __shared_ptr<_Tp> {
 public:
  using element_type = typename std::remove_extent<_Tp>;

  constexpr shared_ptr() noexcept : __shared_ptr<_Tp>() {}

  explicit shared_ptr(_Tp* __p) : __shared_ptr<_Tp>(__p) {}

  shared_ptr(const shared_ptr& __r) noexcept : __shared_ptr<_Tp>(__r) {}

  shared_ptr(const shared_ptr&& __r) noexcept
      : __shared_ptr<_Tp>(std::move(__r)) {}

  explicit shared_ptr(const weak_ptr<_Tp>& __r) : __shared_ptr<_Tp>(__r) {}

  shared_ptr& operator=(const shared_ptr& __r) noexcept {
    this->__shared_ptr<_Tp>::operator=(__r);
    return *this;
  }

  shared_ptr& operator=(const shared_ptr&& __r) noexcept {
    this->__shared_ptr<_Tp>::operator=(std::move(__r));
    return *this;
  }

  friend class weak_ptr<_Tp>;
};

template <typename _Tp>
inline bool operator==(const shared_ptr<_Tp>& __a,
                       const shared_ptr<_Tp>& __b) noexcept {
  return __a.get() == __b.get();
}

/*  shared_ptr comparison with nullptr */
template <typename _Tp>
inline bool operator==(const shared_ptr<_Tp>& __a, std::nullptr_t) noexcept {
  return !__a;
}

template <typename _Tp>
inline bool operator==(std::nullptr_t, const shared_ptr<_Tp>& __a) noexcept {
  return !__a;
}

template <typename _Tp>
inline bool operator!=(const shared_ptr<_Tp>& __a,
                       const shared_ptr<_Tp>& __b) noexcept {
  return __a.get() != __b.get();
}

/*  shared_ptr comparison with nullptr */
template <typename _Tp>
inline bool operator!=(const shared_ptr<_Tp>& __a, std::nullptr_t) noexcept {
  return (bool)__a;
}

/*  shared_ptr comparison with nullptr */
template <typename _Tp>
inline bool operator!=(std::nullptr_t, const shared_ptr<_Tp>& __a) noexcept {
  return (bool)__a;
}

template <typename _Tp>
inline bool operator<(const shared_ptr<_Tp>& __a,
                      const shared_ptr<_Tp>& __b) noexcept {
  return std::less<_Tp>(__a.get(), __b.get());
}

/*  shared_ptr comparison with nullptr */
template <typename _Tp>
inline bool operator<(const shared_ptr<_Tp>& __a, std::nullptr_t) noexcept {
  using _Tp_elt = typename shared_ptr<_Tp>::element_type;
  return std::less<_Tp_elt*>()(__a.get(), nullptr);
}

/*  shared_ptr comparison with nullptr */
template <typename _Tp>
inline bool operator<(std::nullptr_t, const shared_ptr<_Tp>& __a) noexcept {
  using _Tp_elt = typename shared_ptr<_Tp>::element_type;
  return std::less<_Tp_elt*>()(nullptr, __a.get());
}

template <typename _Tp>
inline bool operator<=(const shared_ptr<_Tp>& __a,
                       const shared_ptr<_Tp>& __b) noexcept {
  return !(__b < __a);
}

/*  shared_ptr comparison with nullptr */
template <typename _Tp>
inline bool operator<=(const shared_ptr<_Tp>& __a, std::nullptr_t) noexcept {
  return !(nullptr < __a);
}

/*  shared_ptr comparison with nullptr */
template <typename _Tp>
inline bool operator<=(std::nullptr_t, const shared_ptr<_Tp>& __a) noexcept {
  return !(__a < nullptr);
}

template <typename _Tp>
inline bool operator>(const shared_ptr<_Tp>& __a,
                      const shared_ptr<_Tp>& __b) noexcept {
  return (__b < __a);
}

/*  shared_ptr comparison with nullptr */
template <typename _Tp>
inline bool operator>(const shared_ptr<_Tp>& __a, std::nullptr_t) noexcept {
  return nullptr < __a;
}

/*  shared_ptr comparison with nullptr */
template <typename _Tp>
inline bool operator>(std::nullptr_t, const shared_ptr<_Tp>& __a) noexcept {
  return __a < nullptr;
}

template <typename _Tp>
inline bool operator>=(const shared_ptr<_Tp>& __a,
                       const shared_ptr<_Tp>& __b) noexcept {
  return !(__a < __b);
}

/*  shared_ptr comparison with nullptr */
template <typename _Tp>
inline bool operator>=(const shared_ptr<_Tp>& __a, std::nullptr_t) noexcept {
  return !(__a < nullptr);
}

/*  shared_ptr comparison with nullptr */
template <typename _Tp>
inline bool operator>=(std::nullptr_t, const shared_ptr<_Tp>& __a) noexcept {
  return !(nullptr < __a);
}

template <typename _Tp>
inline void swap(shared_ptr<_Tp>& __a, shared_ptr<_Tp>& __b) {
  __a.swap(__b);
}

template <typename _Tp>
class weak_ptr : public __weak_ptr<_Tp> {
  constexpr weak_ptr() noexcept = default;

  weak_ptr(const shared_ptr<_Tp>& __r) noexcept : __weak_ptr<_Tp>(__r) {}

  weak_ptr(const weak_ptr& __r) noexcept : __weak_ptr<_Tp>(__r) {}
  weak_ptr(const weak_ptr&& __r) noexcept : __weak_ptr<_Tp>(std::move(__r)) {}

  weak_ptr& operator=(const weak_ptr& __r) noexcept {
    this->__weak_ptr<_Tp>::operator=(__r);
    return *this;
  }

  weak_ptr& operator=(const shared_ptr<_Tp>& __r) noexcept {
    this->__weak_ptr<_Tp>::operator=(__r);
    return *this;
  }

  weak_ptr& operator=(const weak_ptr<_Tp>&& __r) noexcept {
    this->__weak_ptr<_Tp>::operator=(std::move(__r));
    return *this;
  }

  shared_ptr<_Tp> lock() const noexcept { return shared_ptr<_Tp>(*this); }
};

template <typename _Tp>
inline void swap(weak_ptr<_Tp>& __a, weak_ptr<_Tp>& __b) noexcept {
  __a.swap(__b);
}
