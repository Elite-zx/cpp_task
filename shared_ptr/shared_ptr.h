#include <new>

#include "shared_ptr_base.h"

/**
 * A smart pointer that retains shared ownership of an object through a pointer.
 * Several shared_ptr objects may own the same object. The object is destroyed
 * and its memory deallocated when either of the following happens:
 * - the last remaining shared_ptr owning the object is destroyed
 * - the last remaining shared_ptr owning the object is assigned another pointer
 * via operator= or reset().
 */
template <typename _Tp>
class shared_ptr : public __shared_ptr<_Tp> {
 public:
  using element_type = typename std::remove_extent<_Tp>;

  /**
   * Default constructor. Constructs an empty shared_ptr.
   */
  constexpr shared_ptr() noexcept : __shared_ptr<_Tp>() {}

  /**
   * Constructs a shared_ptr that owns the given raw pointer.
   * @param __p A raw pointer to the object to be managed.
   */
  explicit shared_ptr(_Tp* __p) : __shared_ptr<_Tp>(__p) {}

  /**
   * Copy constructor. Creates a new shared_ptr that shares ownership of the
   * object managed by __r.
   * @param __r Another shared_ptr to share the managed object with.
   */
  shared_ptr(const shared_ptr& __r) noexcept : __shared_ptr<_Tp>(__r) {}

  /**
   * Move constructor. Transfers ownership from __r to this shared_ptr.
   * @param __r Another shared_ptr to transfer the ownership from.
   */
  shared_ptr(shared_ptr&& __r) noexcept : __shared_ptr<_Tp>(std::move(__r)) {}

  /**
   * Constructs a shared_ptr that shares ownership of the object managed by a
   * weak_ptr.
   * @param __r A weak_ptr that manages the desired object.
   */
  explicit shared_ptr(const weak_ptr<_Tp>& __r) : __shared_ptr<_Tp>(__r) {}

  /**
   * Copy assignment operator. Replaces the managed object with the one managed
   * by __r.
   * @param __r Another shared_ptr to assign from.
   */
  shared_ptr& operator=(const shared_ptr& __r) noexcept {
    this->__shared_ptr<_Tp>::operator=(__r);
    return *this;
  }

  /**
   * Move assignment operator. Transfers ownership from __r to this shared_ptr.
   * @param __r Another shared_ptr to assign from.
   */
  shared_ptr& operator=(shared_ptr&& __r) noexcept {
    this->__shared_ptr<_Tp>::operator=(std::move(__r));
    return *this;
  }

 private:
  /* used by weak_ptr::lock() */
  shared_ptr(const weak_ptr<_Tp>& __r, std::nothrow_t) noexcept
      : __shared_ptr<_Tp>(__r, std::nothrow) {}

  friend class weak_ptr<_Tp>;
};

template <typename _Tp, typename... _Args>
inline shared_ptr<_Tp> make_shared(_Args... __args) {
  return shared_ptr<_Tp>(new _Tp(std::forward<_Args>(__args)...));
}

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

/**
 * Swaps the contents of two shared_ptr objects.
 * @param __a One shared_ptr to swap.
 * @param __b Another shared_ptr to swap with.
 */
template <typename _Tp>
inline void swap(shared_ptr<_Tp>& __a, shared_ptr<_Tp>& __b) {
  __a.swap(__b);
}

template <typename _Tp>
class weak_ptr : public __weak_ptr<_Tp> {
 public:
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

  /**
   * Locks this weak_ptr, returning a shared_ptr that owns the object.
   * If the object has already been deleted, the returned shared_ptr is empty.
   */
  shared_ptr<_Tp> lock() const noexcept {
    return shared_ptr<_Tp>(*this, std::nothrow);
  }
};

/**
 * Swaps the contents of two weak_ptr objects.
 * @param __a One weak_ptr to swap.
 * @param __b Another weak_ptr to swap with.
 */
template <typename _Tp>
inline void swap(weak_ptr<_Tp>& __a, weak_ptr<_Tp>& __b) noexcept {
  __a.swap(__b);
}

template <typename _Tp>
class enable_shared_from_this {
 protected:
  constexpr enable_shared_from_this() noexcept {}
  enable_shared_from_this(const enable_shared_from_this&) noexcept {}

  enable_shared_from_this& operator=(const enable_shared_from_this&) noexcept {
    return *this;
  }

  ~enable_shared_from_this() {}

 public:
  shared_ptr<_Tp> shared_from_this() {
    return shared_ptr<_Tp>(this->_M_weak_this);
  }
  shared_ptr<const _Tp> shared_from_this() const {
    return shared_ptr<const _Tp>(this->_M_weak_this);
  }

  weak_ptr<_Tp> weak_from_this() noexcept { return this->_M_weak_this; }

  weak_ptr<const _Tp> weak_from_this() const noexcept {
    return this->_M_weak_this;
  }

 private:
  void _M_weak_assign(_Tp* __p, const __shared_count& __n) const noexcept {
    _M_weak_this._M_assign(__p, __n);
  }

  friend const enable_shared_from_this* __enable_shared_from_this_base(
      const __shared_count&, const enable_shared_from_this* __p) {
    return __p;
  }

  template <typename>
  friend class __shared_ptr;

  mutable weak_ptr<_Tp> _M_weak_this;
};
