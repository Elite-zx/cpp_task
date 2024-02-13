#include <sys/cdefs.h>
#include <sys/types.h>

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <stdexcept>
#include <type_traits>

/**
 * Base class for managing the reference count mechanics for shared_ptr and
 * weak_ptr. This class provides a foundation for implementing custom control
 * blocks with atomic reference counting and support for custom deleters.
 */
class _sp_counted_base {
 public:
  /**
   * Constructs a _sp_counted_base object with initialized reference counts.
   * Both the use count and the weak count are initialized to 1 to account for
   * the shared_ptr that creates the control block and the potential weak_ptr
   * that may be created from it.
   */
  _sp_counted_base() noexcept : _use_count(1), _weak_count(1) {}

  /**
   * Virtual destructor.
   * Ensures derived classes are correctly destructed.
   */
  virtual ~_sp_counted_base() noexcept {}

  /**
   * Pure virtual function to be overridden by derived classes to handle object
   * deletion. This function is called when the use count drops to zero,
   * indicating that all shared_ptr instances have been destroyed or reset and
   * the managed object should be deleted.
   */
  virtual void _use_dispose() noexcept = 0;

  /**
   * Virtual function called when the weak count drops to zero.
   * At this point, all weak_ptr instances have been destroyed or reset, and if
   * the use count is also zero, it is safe to delete the control block itself.
   */
  virtual void _weak_destroy() { delete this; }

  /**
   * Pure virtual function to return a pointer to the custom deleter, if any.
   * This function is required for type-erased access to custom deleters
   * associated with the control block.
   *
   * @__ti The std::type_info object of the deleter type.
   * @return A void pointer to the deleter, or nullptr if no custom deleter is
   * set.
   */
  virtual void *_M_get_deleter(const std::type_info &) noexcept = 0;

  /**
   * Increments the use count atomically.
   * This function is called whenever a new shared_ptr is created or copied.
   */
  void _use_add_ref() { _use_count.fetch_add(1, std::memory_order_relaxed); }

  /**
   * Increments the weak count atomically.
   * This function is called whenever a new weak_ptr is created or copied.
   */
  void _weak_add_ref() { _weak_count.fetch_add(1, std::memory_order_relaxed); }

  /**
   * Attempts to increment the use count atomically, but does not throw if the
   * use count is zero. This is used by weak_ptr to create a shared_ptr only if
   * the object has not yet been deleted.
   *
   * @return True if the use count was successfully incremented; false if the
   * use count is zero.
   */
  bool _try_use_add_ref_nothrow() noexcept;

  /**
   * Attempts to increment the use count atomically, throwing std::logic_error
   * if the use count is zero. This function is used internally to enforce
   * preconditions for certain operations.
   */
  void _try_use_add_ref() {
    if (!_try_use_add_ref_nothrow())
      throw std::logic_error("Cannot increment use_count as it is zero");
  }

  /**
   * Decrements the use count atomically.
   * If the use count reaches zero as a result, _use_dispose and potentially
   * _weak_destroy are called.
   */
  void _use_sub_ref() noexcept;

  /**
   * Handles the last use count decrement.
   * This function is called by _use_sub_ref when the use count reaches zero,
   * to perform object deletion and potentially control block deletion.
   */
  void _use_sub_ref_last() noexcept;

  /**
   * Decrements the weak count atomically.
   * If the weak count reaches zero as a result, _weak_destroy is called.
   */
  void _weak_sub_ref() noexcept {
    if (_weak_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
      _weak_destroy();
  }

  /**
   * Retrieves the current use count.
   *
   * @return The current number of shared_ptr instances managing the shared
   * object.
   */
  int32_t _get_use_count() { return _use_count.load(); }

 private:
  /* Prevents copy construction and assignment. */
  _sp_counted_base(const _sp_counted_base &) = delete;
  _sp_counted_base &operator=(const _sp_counted_base &) = delete;

  /* Atomic counter for the number of shared_ptr instances.*/
  std::atomic_int32_t _use_count;
  /* Atomic counter for the number of weak_ptr instances.*/
  std::atomic_int32_t _weak_count;
};

/**
 * Definitions for the inline functions declared above.
 *
 * Attempts to atomically increment the use count of the control block,
 * but only if it is not zero.
 *
 * This function performs a lock-free check-and-increment operation on
 * the use count. It first checks if the use count is zero - if so,
 * incrementing would be unsafe as the object might be or about to be
 * destroyed, so it returns false. If the use count is not zero, it
 * attempts to atomically increment it. This operation might need
 * several attempts if other threads are concurrently modifying the
 * use count.
 *
 * The atomic operation uses compare-and-exchange in a loop until it
 * succeeds or finds the use count to be zero. The memory orders
 * specify how memory accesses are ordered around the atomic operation:
 * - std::memory_order_acq_rel ensures a happens-before relationship
 *   between the read-modify-write operation on success. This means
 *   that memory writes before the increment in other threads are
 *   visible to this thread once the increment succeeds and any writes
 *   after the increment in this thread are visible to other threads
 *   that see the incremented value.
 *
 * - std::memory_order_relaxed is used when the compare-and-exchange
 *   operation fails. It imposes no ordering constraints on memory
 *   operations, only guarantees atomicity and visibility of the
 *   failure to other threads. This reduces synchronization overhead
 *   on failure, where the operation's side-effects are not critical.
 *
 * @return True if the use count was successfully incremented from a non-zero
 *         value, false if the use count was zero.
 */
inline bool _sp_counted_base::_try_use_add_ref_nothrow() noexcept {
  int __count = _get_use_count();
  do {
    if (__count == 0) return false;
  } while (!_use_count.compare_exchange_weak(__count, __count + 1,
                                             std::memory_order_acq_rel,
                                             std::memory_order_relaxed));
  return true;
}

inline void _sp_counted_base::_use_sub_ref() noexcept {
  if (_use_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
    _use_sub_ref_last();
}

inline void _sp_counted_base::_use_sub_ref_last() noexcept {
  _use_dispose();
  if (_weak_count.fetch_sub(1, std::memory_order_acq_rel) == 1) _weak_destroy();
}

/* Forward declarations for shared and weak pointer classes. */
template <typename _Tp>
class __shared_ptr;

template <typename _Tp>
class __weak_ptr;

/* Type erasure utility for accessing shared_ptr internals. */
template <typename _Tp>
class __shared_ptr_access;

template <typename _Tp>
class shared_ptr;

template <typename _Tp>
class weak_ptr;

template <typename _Tp>
class enable_shared_from_this;

/* Classes for managing shared and weak reference counts. */
class __shared_count;
class __weak_count;

/**
 * A specialized control block for shared_ptr that manages the lifetime of the
 * pointed-to object. This class inherits from _sp_counted_base to implement the
 * custom deletion logic.
 *
 * @tparam _Ptr Type of the pointer to manage.
 */
template <typename _Ptr>
class _sp_counted_ptr final : public _sp_counted_base {
 public:
  /**
   * Constructor that initializes the control block with a pointer to the
   * managed object.
   *
   * @__p Pointer to the object managed by shared_ptr.
   */
  explicit _sp_counted_ptr(_Ptr __p) noexcept : _M_ptr(__p) {}

  /**
   * Custom disposal action that deletes the managed object.
   * This function overrides _use_dispose from _sp_counted_base and is called
   * when the use count reaches zero.
   */
  virtual void _use_dispose() noexcept override { delete _M_ptr; }

  /**
   * Destroys the control block itself.
   * This function overrides _weak_destroy from _sp_counted_base and is called
   * when both use count and weak count reach zero.
   */
  virtual void _weak_destroy() noexcept override { delete this; }

  /**
   * Returns a pointer to the custom deleter, if any.
   * This function provides a way to access a potential custom deleter
   * associated with the control block.
   *
   * @__ti The std::type_info of the requested deleter type.
   * @return Void pointer to the custom deleter or nullptr if no custom deleter
   * is set.
   */
  virtual void *_M_get_deleter(const std::type_info &__ti) noexcept override {
    return nullptr;
  }

 private:
  /* Prevent copy construction and assignment. */
  _sp_counted_ptr(const _sp_counted_ptr &) = delete;
  _sp_counted_ptr &operator=(const _sp_counted_ptr &) = delete;

  /* Pointer to the managed object. */
  _Ptr _M_ptr;
};

/**
 * Specialization of _sp_counted_ptr for std::nullptr_t.
 * This specialization ensures that calling _use_dispose on a control block
 * managing a nullptr is a no-op.
 */
template <>
inline void _sp_counted_ptr<std::nullptr_t>::_use_dispose() noexcept {}

/**
 * Manages the shared reference count for shared_ptr instances.
 * This class encapsulates the control block (_sp_counted_base) that manages the
 * shared and weak reference counts.
 */
class __shared_count {
 public:
  /**
   * Default constructor.
   * Initializes an empty __shared_count with no associated control block.
   */
  constexpr __shared_count() noexcept : _M_pi(0) {}

  /**
   * Constructs a __shared_count and creates a control block to manage the given
   * pointer.
   *
   * @tparam _Ptr Type of the pointer to be managed.
   * @__p Pointer to the object to be managed by shared_ptr.
   */
  template <typename _Ptr>
  explicit __shared_count(_Ptr __p) : _M_pi(0) {
    _M_pi = new _sp_counted_ptr<_Ptr>(__p);
  }

  /**
   * Destructor.
   * Decrements the use count of the associated control block, potentially
   * causing the managed object and control block to be deleted.
   */
  ~__shared_count() noexcept {
    if (_M_pi != nullptr) _M_pi->_use_sub_ref();
  }

  /**
   * Copy constructor.
   * Creates a new __shared_count that shares ownership of the managed object
   * with another instance.
   *
   * @__r Another __shared_count instance to share ownership with.
   */
  __shared_count(const __shared_count &__r) noexcept : _M_pi(__r._M_pi) {
    if (_M_pi != nullptr) _M_pi->_use_add_ref();
  }

  /**
   * Copy assignment operator.
   * Shares ownership of the managed object with another __shared_count
   * instance.
   *
   * @__r Another __shared_count instance to share ownership with.
   * @return *this
   */
  __shared_count &operator=(const __shared_count &__r) noexcept {
    _sp_counted_base *__tmp = __r._M_pi;
    if (__tmp != _M_pi) {
      if (__tmp != nullptr) __tmp->_use_add_ref();
      if (_M_pi != nullptr) _M_pi->_use_sub_ref();
      _M_pi = __tmp;
    }
    return *this;
  }

  /**
   * Swaps the control block managed by this instance with another
   * __shared_count instance.
   *
   * @__r Another __shared_count instance to swap with.
   */
  void __swap(__shared_count &__r) noexcept {
    _sp_counted_base *__tmp = __r._M_pi;
    __r._M_pi = _M_pi;
    _M_pi = __tmp;
  }

  /**
   * Returns the current use count of the managed object.
   *
   * @return The number of shared_ptr instances managing the same object.
   */
  int32_t _get_use_count() const noexcept {
    return _M_pi != nullptr ? _M_pi->_get_use_count() : 0;
  }

  /**
   * Checks whether this instance is the sole owner of the managed object.
   *
   * @return true if the use count is one, false otherwise.
   */
  bool __unique() const noexcept { return this->_get_use_count() == 1; }

  /**
   * Attempts to retrieve a pointer to a custom deleter associated with the
   * control block, if any.
   *
   * @__ti The std::type_info of the requested deleter type.
   * @return Void pointer to the custom deleter or nullptr if no custom deleter
   * is set.
   */
  void *_M_get_deleter(const std::type_info &__ti) const noexcept {
    return _M_pi ? _M_pi->_M_get_deleter(__ti) : nullptr;
  }

  /**
   * Compares the address of the control block managed by this instance with
   * another __shared_count instance for owner-based ordering.
   *
   * @__rhs Another __shared_count instance to compare with.
   * @return true if this instance precedes __rhs in owner-based order, false
   * otherwise.
   */
  bool __less(const __shared_count &__rhs) const noexcept {
    return std::less<_sp_counted_base *>{}(this->_M_pi, __rhs._M_pi);
  }

  /**
   * Compares the address of the control block managed by this instance with a
   * __weak_count instance for owner-based ordering.
   *
   * @__rhs A __weak_count instance to compare with.
   * @return true if this instance precedes __rhs in owner-based order, false
   * otherwise.
   */
  bool _M_less(const __weak_count &__rhs) const noexcept;

 private:
  /* Friend class declaration to allow __weak_count to access private members.
   */
  friend class __weak_count;
  /* Pointer to the control block. */
  _sp_counted_base *_M_pi;
};

/**
 * Manages the weak reference count for shared_ptr objects.
 *
 * This class is responsible for tracking the number of weak_ptr instances
 * that refer to the same shared resource. When a shared_ptr object is created
 * or destroyed, it updates the shared reference count accordingly.
 *
 * A weak_ptr does not own the object it points to; it allows access to an
 * object owned by one or more shared_ptr instances but does not extend the
 * lifetime of those objects. As such, a weak_ptr can be used to break
 * circular references that would otherwise lead to memory leaks.
 */
class __weak_count {
 public:
  /**
   * Default constructor.
   * Initializes a new instance of the __weak_count class with a null pointer.
   */
  constexpr __weak_count() noexcept : _M_pi(nullptr) {}

  /**
   * Copy constructor from a __shared_count object.
   * Increases the weak count of the control block that __r is associated with.
   *
   * @__r A __shared_count object from which to copy.
   */
  __weak_count(const __shared_count &__r) noexcept : _M_pi(__r._M_pi) {
    if (_M_pi != nullptr) _M_pi->_weak_add_ref();
  }

  /**
   * Copy constructor.
   * Creates a new __weak_count object as a copy of an existing one,
   * incrementing the weak reference count of the shared control block.
   *
   * @__r The __weak_count object to copy.
   */
  __weak_count(const __weak_count &__r) noexcept : _M_pi(__r._M_pi) {
    if (_M_pi != nullptr) _M_pi->_weak_add_ref();
  }

  /**
   * Move constructor.
   * Transfers ownership of the weak count from __r to this object.
   *
   * @__r The __weak_count object to move from.
   */
  __weak_count(__weak_count &&__r) noexcept : _M_pi(__r._M_pi) {
    __r._M_pi = nullptr;
  }

  /**
   * Destructor.
   * Decrements the weak reference count and, if the weak count reaches zero,
   * destroys the control block if there are no shared references.
   */
  ~__weak_count() noexcept {
    if (_M_pi != nullptr) _M_pi->_weak_sub_ref();
  }

  /**
   * Copy assignment operator.
   * Replaces the current weak count with that of another __weak_count object.
   *
   * @__r The __weak_count object to assign from.
   * @return A reference to this __weak_count object.
   */
  __weak_count &operator=(const __shared_count &__r) noexcept {
    _sp_counted_base *__tmp = __r._M_pi;
    if (__tmp != nullptr) __tmp->_weak_add_ref();
    if (_M_pi != nullptr) _M_pi->_weak_sub_ref();
    _M_pi = __tmp;
    return *this;
  }

  /**
   * Move assignment operator.
   * Transfers the weak count from another __weak_count object to this one.
   *
   * @__r The __weak_count object to move from.
   * @return A reference to this __weak_count object.
   */
  __weak_count &operator=(__weak_count &&__r) noexcept {
    if (_M_pi != nullptr) _M_pi->_weak_sub_ref();
    _M_pi = __r._M_pi;
    __r._M_pi = nullptr;
    return *this;
  }

  /**
   * Swaps the weak count with another __weak_count object.
   *
   * @__r The other __weak_count object to swap with.
   */
  void __swap(__weak_count &__r) noexcept {
    _sp_counted_base *__tmp = __r._M_pi;
    __r._M_pi = _M_pi;
    _M_pi = __tmp;
  }

  /**
   * Gets the use count of the shared object.
   *
   * @return The number of shared_ptr instances managing the shared object,
   * or zero if the object has already been deleted.
   */
  int32_t _get_use_count() const noexcept {
    return _M_pi != nullptr ? _M_pi->_get_use_count() : 0;
  }

  /**
   * Compares the ownership of two __weak_count objects.
   * Used to support weak_ptr comparisons.
   *
   * @__rhs The right-hand side __weak_count object for comparison.
   * @return True if this object's control block is less than that of __rhs.
   */
  bool __less(const __shared_count &__rhs) const noexcept {
    return std::less<_sp_counted_base *>{}(this->_M_pi, __rhs._M_pi);
  }

 private:
  friend class __shared_count;
  _sp_counted_base *_M_pi;
};

/* now weak_count is defined */
inline bool __shared_count::_M_less(const __weak_count &__rhs) const noexcept {
  return std::less<_sp_counted_base *>()(this->_M_pi, __rhs._M_pi);
}

template <typename _Tp>
class __enable_shared_from_this {
 protected:
  constexpr __enable_shared_from_this() noexcept {}
  __enable_shared_from_this(const __enable_shared_from_this &) noexcept {}

  __enable_shared_from_this &operator=(
      const __enable_shared_from_this &) noexcept {
    return *this;
  }

  ~__enable_shared_from_this() {}

 public:
  __shared_ptr<_Tp> share_from_this() {
    return shared_ptr<_Tp>(this->_M_weak_this);
  }
  __shared_ptr<const _Tp> share_from_this() const {
    return shared_ptr<const _Tp>(this->_M_weak_this);
  }

  __weak_ptr<_Tp> weak_from_this() noexcept { return this->_M_weak_this; }

  __weak_ptr<const _Tp> weak_from_this() const noexcept {
    return this->_M_weak_this;
  }

 private:
  void _M_weak_assign(_Tp *__p, const __shared_count &__n) const noexcept {
    _M_weak_this._M_assign(__p, __n);
  }

  friend const __enable_shared_from_this *__enable_shared_from_this_base(
      const __shared_count &, const __enable_shared_from_this *__p) {
    return __p;
  }

  template <typename>
  friend class __shared_ptr;

  mutable weak_ptr<_Tp> _M_weak_this;
};

/**
 * A smart pointer type that manages a dynamically allocated object through a
 * pointer. The object is disposed of using delete when the last shared_ptr
 * pointing to it is destroyed or reset.
 */
template <typename _Tp>
class __shared_ptr {
 public:
  /* Defines the type of the pointed-to object. */
  using element_type = typename std::remove_extent<_Tp>::type;

  /**
   * Default constructor. Constructs an empty __shared_ptr.
   */
  constexpr __shared_ptr() noexcept : _M_ptr(0), _M_ref_count() {}

  /**
   * Constructor from raw pointer. Takes ownership of the provided pointer.
   * @__p: A pointer to the dynamically allocated object.
   */
  explicit __shared_ptr(_Tp *__p) : _M_ptr(__p), _M_ref_count(__p) {
    _M_enable_shared_from_this_with(__p);
  };

  /**
   * Copy constructor. Creates a new __shared_ptr that shares ownership of the
   * object.
   * @__r Another __shared_ptr to share ownership with.
   */
  __shared_ptr(const __shared_ptr &__r) noexcept
      : _M_ptr(__r._M_ptr), _M_ref_count(__r._M_ref_count) {}

  /**
   * Move constructor. Transfers ownership of the managed object and control
   * block from another __shared_ptr instance. This constructor initializes the
   * current object's reference count to zero before swapping it with the
   * source's reference count, effectively transferring ownership and ensuring
   * that the moved-from __shared_ptr is left in a safe, empty state.
   * @__r: Another __shared_ptr instance to transfer ownership from. After
   * the operation, __r will be empty.
   */
  __shared_ptr(__shared_ptr &&__r) noexcept
      : _M_ptr(__r._M_ptr), _M_ref_count() {
    _M_ref_count.__swap(__r._M_ref_count);
    __r._M_ptr = nullptr;
  }

  /**
   * Constructs a __shared_ptr from a __weak_ptr, promoting the weak pointer to
   * a shared pointer.
   * @__r A __weak_ptr to promote to a __shared_ptr.
   */
  explicit __shared_ptr(const __weak_ptr<_Tp> &__r) noexcept
      : _M_ref_count(__r._M_ref_count) {
    _M_ptr = __r._M_ptr;
  }

  /**
   * Copy assignment operator. Shares ownership of the object pointed to by
   * another __shared_ptr.
   * @__r Another __shared_ptr to share ownership with.
   */
  __shared_ptr &operator=(const __shared_ptr &__r) noexcept {
    _M_ptr = __r._M_ptr;
    _M_ref_count = __r._M_ref_count;
    return *this;
  }

  /**
   * Move assignment operator. Transfers ownership from another __shared_ptr.
   * @__r Another __shared_ptr to transfer ownership from.
   */
  __shared_ptr &operator=(__shared_ptr &&__r) noexcept {
    __shared_ptr(std::move(__r)).swap(*this);
    return *this;
  }

  element_type &operator*() const noexcept {
    if (get() != nullptr) {
      return *get();
    } else {
      throw std::logic_error("shared_ptr is empty");
    }
  }

  element_type *operator->() const noexcept {
    if (get() != nullptr) {
      return get();
    } else {
      throw std::logic_error("shared_ptr is empty");
    }
  }

  /**
   * Resets the __shared_ptr to empty, decreasing the reference count of the
   * managed object if present.
   */
  void reset() { __shared_ptr().swap(*this); }

  /**
   * Resets the __shared_ptr to manage a different object, if the provided
   * pointer is different from the current one.
   * @__p A pointer to the dynamically allocated object to manage.
   */
  void reset(_Tp *__p) {
    if (__p != nullptr && __p != _M_ptr) {
      shared_ptr(__p).swap(*this);
    } else {
      throw std::logic_error("ptr is illegal");
    }
  }

  /**
   * Returns the stored pointer.
   */
  element_type *get() const noexcept { return _M_ptr; }

  /**
   * Checks if there is an associated managed object, i.e., whether get() !=
   * nullptr.
   */
  explicit operator bool() const noexcept { return _M_ptr != nullptr; }

  /**
   * Checks if *this is the sole owner of the managed object.
   */
  bool unique() const noexcept { return _M_ref_count.__unique(); }

  /**
   * Returns the number of __shared_ptr instances managing the current object.
   */
  long use_count() const noexcept { return _M_ref_count._get_use_count(); }

  /**
   * Swaps the contents of this __shared_ptr instance with another.
   * @__other Another __shared_ptr to swap contents with.
   */
  void swap(__shared_ptr &__other) noexcept {
    std::swap(_M_ptr, __other._M_ptr);
    _M_ref_count.__swap(__other._M_ref_count);
  }

  /* Destructor automatically defined. */
  ~__shared_ptr() = default;

  friend class __weak_ptr<_Tp>;

 private:
  /*  initialize _M_weak_this if _Tp inherits from
   * __enable_shared_from_this<_Tp>, otherwise do nothing */
  template <typename _Tp2 = typename std::remove_cv<_Tp>::type>
  typename std::enable_if<
      std::is_base_of_v<__enable_shared_from_this<_Tp2>, _Tp2>>::type
  _M_enable_shared_from_this_with(_Tp *__p) noexcept {
    if (auto __base = __enable_shared_from_this_base(_M_ref_count, __p))
      __base->_M_weak_assign(const_cast<_Tp2 *>(__p), _M_ref_count);
  }

  template <typename _Tp2 = typename std::remove_cv<_Tp>::type>
  typename std::enable_if<
      !std::is_base_of_v<__enable_shared_from_this<_Tp2>, _Tp2>>::type
  _M_enable_shared_from_this_with(_Tp *) noexcept {}

  /*Contained pointer. The raw instance pointer is stored here.*/
  element_type *_M_ptr;
  /* Reference counter, a.k.a control block. */
  __shared_count _M_ref_count;
};

/**
 * A smart pointer type for managing objects without owning them.
 * It is converted to __shared_ptr to access the object, ensuring it still
 * exists.
 */
template <typename _Tp>
class __weak_ptr {
 public:
  /* Defines the type of the pointed-to object. */
  using element_type = typename std::remove_extent<_Tp>::type;

  /**
   * Constructs a __weak_ptr that observes the object owned by a __shared_ptr.
   * @__r: A __shared_ptr whose observed object will be shared.
   */
  __weak_ptr(const __shared_ptr<_Tp> &__r) noexcept
      : _M_ptr(__r._M_ptr), _M_ref_count(__r._M_ref_count) {}

  /**
   * Copy constructor. Constructs a __weak_ptr that shares observation with
   * another __weak_ptr.
   * @__r: Another __weak_ptr to share observation with.
   */
  __weak_ptr(__weak_ptr &__r) noexcept : _M_ref_count(__r._M_ref_count) {
    _M_ptr == __r.lock().get();
  }

  /**
   * Move constructor. Constructs a __weak_ptr by transferring observation from
   * another __weak_ptr.
   * @__r Another __weak_ptr to transfer observation from.
   */
  __weak_ptr(__weak_ptr &&__r) noexcept
      : _M_ptr(__r._M_ptr), _M_ref_count(std::move(__r._M_ref_count)) {
    __r._M_ptr = nullptr;
  }

  /**
   * Copy assignment operator. Shares observation of the object managed by
   * another __weak_ptr.
   * @__r Another __weak_ptr to share observation with.
   */
  __weak_ptr &operator=(const __weak_ptr &__r) noexcept {
    _M_ptr = __r.lock().get();
    _M_ref_count = __r._M_ref_count;
    return *this;
  }

  /**
   * Assigns this __weak_ptr to observe the object managed by a __shared_ptr.
   * @__r: A __shared_ptr whose observed object will be shared.
   */
  __weak_ptr &operator=(const __shared_ptr<_Tp> &__r) noexcept {
    _M_ptr = __r._M_ptr;
    _M_ref_count = __r._M_ref_count;
    return *this;
  }

  /**
   * Move assignment operator. Transfers observation from another __weak_ptr.
   * @__r: Another __weak_ptr to transfer observation from.
   */
  __weak_ptr &operator=(__weak_ptr &&__r) noexcept {
    __weak_ptr(std::move(__r)).swap(*this);
    return *this;
  }

  /**
   * Attempts to obtain a __shared_ptr that owns the observed object.
   */
  __shared_ptr<_Tp> lock() const noexcept {
    return __shared_ptr<element_type>(*this);
  }

  /**
   * Returns the number of __shared_ptr instances managing the observed object.
   */
  long use_count() const noexcept { return _M_ref_count._get_use_count(); }

  /**
   * Checks whether the observed object has already been deleted.
   */
  bool expired() const noexcept { return _M_ref_count._get_use_count() == 0; }

  /**
   * Resets the __weak_ptr to be empty.
   */
  void reset() noexcept { __weak_ptr().swap(*this); }

  /**
   * Swaps the contents of this __weak_ptr instance with another.
   * @__s: Another __weak_ptr to swap contents with.
   */
  void swap(__weak_ptr &__s) noexcept {
    std::swap(_M_ptr, __s._M_ptr);
    _M_ref_count.__swap(__s._M_ref_count);
  }

 private:
  /* Used by __enable_shared_from_this */
  void _M_assign(_Tp *__ptr, const __shared_count &__ref_count) noexcept {
    if (use_count() == 0) {
      _M_ptr = __ptr;
      _M_ref_count = __ref_count;
    }
  }
  friend class __shared_ptr<_Tp>;
  friend class enable_shared_from_this<_Tp>;
  friend class __enable_shared_from_this<_Tp>;
  element_type *_M_ptr;      /* Contained pointer.*/
  __weak_count _M_ref_count; /* Reference counter.*/
};
