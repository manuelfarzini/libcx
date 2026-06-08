/** @file libcx/mem/shared_pointer.hh **/
#include <libcx/mem/memory.hh>
#include <atomic>

// TODO: single pointer instead of vtable.

namespace cx::mem {

struct control_block_base;
struct control_block_vtable;
template<typename T> struct control_block_inplace;
template<typename T> struct shared_pointer;

typedef auto deleter_proc(control_block_base*) noexce -> void;

///
struct control_block_base {

  ///
  std::atomic<usize> strong_count;

  ///
  std::atomic<usize> weak_count;

  ///
  control_block_vtable const* vtable_ptr;
};

struct control_block_vtable {

  ///
  deleter_proc* destroy_object;

  ///
  deleter_proc* dealloc_ctrl_block;
};

///
template<typename T> struct control_block_inplace : control_block_base {

  ///
  alignas(T) u8 object_storage[sizeof(T)];

  /* static auto im__destroy_object(control_block_base* base_ptr) noexce -> void {

    auto* self = static_cast<control_block_inplace<T>*>(base_ptr);
    self->object_ptr()->~T();
  }
  static auto im__dealloc_ctrl_block(control_block_base* base_ptr) noexce -> void {
    auto* self = static_cast<control_block_inplace<T>*>(base_ptr);
    cx::mem::deallocate<control_block_inplace<T>>(self);
  } */

  static cons control_block_vtable vtable = {
      [](control_block_base* base_ptr) noexce {
        auto* self = static_cast<control_block_inplace<T>*>(base_ptr);
        self->object_ptr()->~T();
      },
      [](control_block_base* base_ptr) noexce {
        auto* self = static_cast<control_block_inplace<T>*>(base_ptr);
        cx::mem::deallocate<control_block_inplace<T>>(self);
      }};

  ///
  control_block_inplace() noexce { vtable_ptr = &vtable; }

  ///
  inln cons auto storage_ptr() noexce -> T* {
    return reinterpret_cast<T*>(object_storage);
  }

  ///
  inln cons auto object_ptr() noexce -> T* {
    return std::launder(reinterpret_cast<T*>(object_storage));
  }
};


///
template<typename T> struct shared_pointer {

  static_assert(!std::is_polymorphic_v<T> || std::has_virtual_destructor_v<T> ||
                    std::is_final_v<T>,
                "Polymorphic base T should have a virtual destructor or be final.");

  ///
  struct adopt_tag_t {};

  ///
  static cons adopt_tag_t adopt_tag{};

  ///
  using element_type = T;

  ///
  control_block_base* ctrl_ptr{nullptr};

  ///
  element_type* obj_ptr{nullptr};


  ///
  cons shared_pointer() = default;

  ///
  shared_pointer(shared_pointer const& that) noexce
      : ctrl_ptr(that.ctrl_ptr), obj_ptr(that.obj_ptr) {
    if (ctrl_ptr != nullptr) {
      ctrl_ptr->strong_count.fetch_add(1, std::memory_order_relaxed);
    }
  }

  ///
  cons shared_pointer(shared_pointer&& that) noexce
      : ctrl_ptr(that.ctrl_ptr), obj_ptr(that.obj_ptr) {
    that.ctrl_ptr = nullptr;
    that.obj_ptr = nullptr;
  }

  ///
  auto operator=(shared_pointer const& that) noexce -> shared_pointer& {
    if (this == &that) {
      return *this;
    }
    if (that.ctrl_ptr != nullptr) {
      that.ctrl_ptr->strong_count.fetch_add(1, std::memory_order_relaxed);
    }
    release();
    ctrl_ptr = that.ctrl_ptr;
    obj_ptr = that.obj_ptr;
    return *this;
  }

  ///
  auto operator=(shared_pointer&& that) noexce -> shared_pointer& {
    if (this == &that) {
      return *this;
    }
    release();
    ctrl_ptr = that.ctrl_ptr;
    obj_ptr = that.obj_ptr;
    that.ctrl_ptr = nullptr;
    that.obj_ptr = nullptr;
    return *this;
  }

  ///
  ~shared_pointer() { release(); }


  ///
  template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
  cons shared_pointer(shared_pointer<U> const& that) noexce
      : ctrl_ptr(that.ctrl_ptr), obj_ptr(static_cast<T*>(that.obj_ptr)) {
    if (ctrl_ptr != nullptr) {
      ctrl_ptr->strong_count.fetch_add(1, std::memory_order_relaxed);
    }
  }

  ///
  template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
  cons shared_pointer(shared_pointer<U>&& that) noexce
      : ctrl_ptr(that.ctrl_ptr), obj_ptr(static_cast<T*>(that.obj_ptr)) {
    that.ctrl_ptr = nullptr;
    that.obj_ptr = nullptr;
  }


  ///
  inln cons auto object_ptr() const noexce -> T* { return obj_ptr; }

  ///
  inln cons auto operator*() const noexce -> T& {
    assert(obj_ptr && "dereferencing nullptr shared_pointer");
    return *obj_ptr;
  }

  ///
  inln cons auto operator->() const noexce -> T* {
    assert(obj_ptr && "dereferencing nullptr shared_pointer");
    return obj_ptr;
  }

  ///
  inln cons explicit operator bool() const noexce {
    return obj_ptr != nullptr;
  }

  ///
  inln cons auto use_count() const noexce -> usize {
    return ctrl_ptr ? ctrl_ptr->strong_count.load(std::memory_order_acquire) : 0;
  }


  ///
  auto release() noexce -> bool {
    if (ctrl_ptr == nullptr) {
      return false;
    }
    if (ctrl_ptr->strong_count.fetch_sub(1, std::memory_order_release) == 1) {
      std::atomic_thread_fence(std::memory_order_acquire);
      ctrl_ptr->vtable_ptr->destroy_object(ctrl_ptr);

      if (ctrl_ptr->weak_count.fetch_sub(1, std::memory_order_release) == 1) {
        std::atomic_thread_fence(std::memory_order_acquire);
        ctrl_ptr->vtable_ptr->dealloc_ctrl_block(ctrl_ptr);
      }
    }
    ctrl_ptr = nullptr;
    obj_ptr = nullptr;
    return true;
  }

  /// Aliasing constructor.
  /// Shares ownership with `other` but points to `alias_ptr`.
  template<typename U>
  shared_pointer(shared_pointer<U> const& other, T* alias_ptr) noexce
      : ctrl_ptr(other.ctrl_ptr), obj_ptr(alias_ptr) {
    if (ctrl_ptr != nullptr) {
      ctrl_ptr->strong_count.fetch_add(1, std::memory_order_relaxed);
    }
  }

  /// XXX: Is this needed ?
  /// Constructs a shared_pointer adopting an existing control block and object.
  /// Does not modify reference counters.
  shared_pointer(control_block_base* base_ptr, T* obj_ptr, adopt_tag_t) noexce
      : ctrl_ptr(base_ptr), obj_ptr(obj_ptr) {}
};


/// Weak Pointer.
template<typename T> struct weak_pointer {

  ///
  using element_type = T;

  ///
  control_block_base* ctrl_ptr{nullptr};

  ///
  element_type* obj_ptr{nullptr};


  ///
  cons weak_pointer() = default;

  ///
  weak_pointer(weak_pointer const& that) noexce
      : ctrl_ptr(that.ctrl_ptr), obj_ptr(that.obj_ptr) {
    if (ctrl_ptr != nullptr) {
      ctrl_ptr->weak_count.fetch_add(1, std::memory_order_relaxed);
    }
  }

  ///
  weak_pointer(weak_pointer&& that) noexce
      : ctrl_ptr(that.ctrl_ptr), obj_ptr(that.obj_ptr) {
    that.ctrl_ptr = nullptr;
    that.obj_ptr = nullptr;
  }

  ///
  auto operator=(weak_pointer const& that) noexce -> weak_pointer& {
    if (this == &that) {
      return *this;
    }
    if (that.ctrl_ptr != nullptr) {
      that.ctrl_ptr->weak_count.fetch_add(1, std::memory_order_relaxed);
    }
    release();
    ctrl_ptr = that.ctrl_ptr;
    obj_ptr = that.obj_ptr;
    return *this;
  }

  ///
  auto operator=(weak_pointer&& that) noexce -> weak_pointer& {
    if (this == &that) {
      return *this;
    }
    release();
    ctrl_ptr = that.ctrl_ptr;
    obj_ptr = that.obj_ptr;
    that.ctrl_ptr = nullptr;
    that.obj_ptr = nullptr;
    return *this;
  }

  ///
  ~weak_pointer() { release(); }


  ///
  weak_pointer(shared_pointer<T> const& that) noexce
      : ctrl_ptr(that.ctrl_ptr), obj_ptr(that.obj_ptr) {
    if (ctrl_ptr != nullptr) {
      ctrl_ptr->weak_count.fetch_add(1, std::memory_order_relaxed);
    }
  }


  ///
  inln cons auto expired() const noexce -> bool {
    return use_count() == 0;
  }

  ///
  inln cons auto use_count() const noexce -> usize {
    return ctrl_ptr ? ctrl_ptr->strong_count.load(std::memory_order_acquire) : 0;
  }

  ///
  auto lock() const noexce -> shared_pointer<T> {
    if (ctrl_ptr == nullptr) {
      return {};
    }

    usize count = ctrl_ptr->strong_count.load(std::memory_order_acquire);
    while (count != 0) {
      if (ctrl_ptr->strong_count.compare_exchange_weak(count, count + 1,
                                                       std::memory_order_acq_rel,
                                                       std::memory_order_acquire)) {
        return shared_pointer<T>{ctrl_ptr, obj_ptr, shared_pointer<T>::adopt_tag};
      }
    }
    return {};
  }


  ///
  auto release() noexce -> bool {
    if (ctrl_ptr == nullptr) {
      return false;
    }

    if (ctrl_ptr->weak_count.fetch_sub(1, std::memory_order_release) == 1) {
      std::atomic_thread_fence(std::memory_order_acquire);
      ctrl_ptr->vtable_ptr->dealloc_ctrl_block(ctrl_ptr);
    }

    ctrl_ptr = nullptr;
    obj_ptr = nullptr;
    return true;
  }


  /// Constructs a weak_pointer adopting an existing control block and object.
  /// Does not modify reference counters.
  weak_pointer(control_block_base* base_ptr, T* obj_ptr,
               shared_pointer<T>::adopt_tag_t) noexce
      : ctrl_ptr(base_ptr), obj_ptr(obj_ptr) {}
};


///
template<typename T, typename... Args>
auto make_shared(Args&&... args) -> Maybe<shared_pointer<T>> {
  control_block_inplace<T>* ctrl = cx::mem::make<control_block_inplace<T>>(1);
  if (ctrl == nullptr) {
    return std::nullopt;
  }

  ctrl->strong_count.store(1, std::memory_order_relaxed);
  ctrl->weak_count.store(1, std::memory_order_relaxed);
  cx::mem::place<T>(ctrl->storage_ptr(), 1, cx::uti::forward<Args>(args)...);

  shared_pointer<T> res{};
  res.ctrl_ptr = ctrl;
  res.obj_ptr = ctrl->object_ptr();
  return res;
}

};  // namespace cx::mem

//==============================================================================


namespace cx::uti {

template<typename T> inln cons T& take(Maybe<T>& m) {
  return m.value();
}

template<typename T> inln cons T&& take(Maybe<T>&& m) {
  return cx::uti::move(m).value();
}

template<typename T> inln cons cx::mem::shared_pointer<T>&&
take(Maybe<cx::mem::shared_pointer<T>>& m) noexce {
  return std::move(m).value();
}
}  // namespace cx::uti
