/** @file libcx/mem/shared_pointer_mk2.hh **/
/// Not supported:
///   - weak pointers,
///   - multiple inheritance,
/// Supported:
///   - multi level single inheritance
///   - adopting
///   - aliasing
// NOTE 
// mk2 version is the encapsulation version
// mk3 version is the no-encapsulation version
#include <libcx/uti/utilities.hh>
#include <libcx/mem/memory.hh>
#include <libcx/mem/order.hh>
#include <atomic>


namespace cx::mem {

struct IControlBlock;
template<typename T> struct ControlBlock;
template<typename T> struct SharedPointer;
CX_CONCEPT_GEN(IControlBlock, is_control_block_base, CControlBlockBase);
CX_CONCEPT_GEN_TEMPL(ControlBlock, is_control_block, CControlBlock, typename T, T);
CX_CONCEPT_GEN_TEMPL(SharedPointer, is_shared_pointer, SharedPointerT, typename T, T);
#define Control_Block_Base CControlBlockBase auto
#define Control_Block_Impl CControlBlock auto
#define Shared_Pointer SharedPointerT auto
typedef fn DeleterProc(IControlBlock*) noexce -> void;


///
struct IControlBlock {
  std::atomic<usize> ref_count{0};
  DeleterProc* release{nullptr};
};

///
template<typename T> struct ControlBlock final : IControlBlock {
  using Elem = T;

  alignas(T) u8 storage[size_of(T)]{};

  cons ControlBlock() noexce { release = __release; }

  /// Returns: a `T` pointer to the raw memory of an `ErasedContainer` `c`
  /// Note: used to construct the element(s) in `c`.
  inln cons auto object_ptr(this auto& cb) noexce -> T* {
    return std::launder(reinterpret_cast<T*>(cb.storage));
  }

  /// Returns: a `T` pointer to the object in an `ErasedContainer` `c`
  /// Note: used to get the fully constructed object in `c`.
  inln cons T* placement_ptr(this auto& cb) noexce {
    return reinterpret_cast<T*>(cb.storage);
  }

  inln glob cons void __release(IControlBlock* base) noexce {
    auto* self = static_cast<ControlBlock<T>*>(base);
    if cons (!std::is_trivially_destructible_v<T>) {
      self->object_ptr()->~T();
    }
    mem::deallocate<ControlBlock<T>>(self);
  }
};

///
template<typename T> struct SharedPointer {
  static_assert(!std::is_void_v<T>,
                "SharedPointer<T>: the element type T must not be 'void'");
  static_assert(!uti::is_array<T>,
                "SharedPointer<T>: T must not be an array type.");
  static_assert(!std::is_reference_v<T> && !std::is_pointer_v<T>,
                "SharedPointer<T>: T must not be a reference or pointer type.");
  static_assert(std::is_destructible_v<T>,
                "SharedPointer<T>: the element type T must be destructible. "
                "The expression 'declval<T&>().~T()' must be accessible.");
  static_assert(std::is_nothrow_destructible_v<T>,
                "SharedPointer<T>: destructor of T must be noexce.");
  static_assert(!std::is_polymorphic_v<T> || std::has_virtual_destructor_v<T> ||
                    std::is_final_v<T>,
                "SharedPointer<T>: polymorphic base T must have a virtual "
                "destructor or be declared final");

  using Elem = T;
  template<typename U> using EnableUpcast = std::enable_if<
      std::is_convertible_v<U*, T*>, int>;
  inline static cons struct AdoptTag {} adopt_tag;

  /// The pointer to control block.
  IControlBlock* ctrl_ptr{nullptr};
  /// The pointer to the owned object.
  Elem* obj_ptr{nullptr};

  /// Default constructor.
  cons SharedPointer() = default;

  /// Copy constructor.
  cons SharedPointer(SharedPointer const& that) noexce
      : ctrl_ptr(that.ctrl_ptr), obj_ptr(that.obj_ptr) {
    if (ctrl_ptr != nullptr) {
      ctrl_ptr->ref_count.fetch_add(1, mem::order_relaxed);
    }
  }

  /// Polymorphic copy constructor.
  template<typename U, EnableUpcast<U> = 0>
  SharedPointer(SharedPointer<U> const& that) noexce
      : ctrl_ptr(that.ctrl_ptr), obj_ptr(static_cast<T*>(that.obj_ptr)) {
    if (ctrl_ptr != nullptr) {
      ctrl_ptr->ref_count.fetch_add(1, mem::order_relaxed);
    }
  }

  /// Move constructor.
  cons SharedPointer(SharedPointer&& that) noexce
      : ctrl_ptr(that.ctrl_ptr), obj_ptr(that.obj_ptr) {
    that.ctrl_ptr = nullptr;
    that.obj_ptr = nullptr;
  }

  /// Polymorphic move constructor
  template<typename U, EnableUpcast<U> = 0>
  cons SharedPointer(SharedPointer<U>&& that) noexce
      : ctrl_ptr(that.ctrl_ptr), obj_ptr(static_cast<T*>(that.obj_ptr)) {
    that.ctrl_ptr = nullptr;
    that.obj_ptr = nullptr;
  }

  /// Aliasing contructor.
  template<typename U>
  cons SharedPointer(SharedPointer<U> const& other, T* alias_ptr) noexce
      : ctrl_ptr(other.ctrl_ptr), obj_ptr(alias_ptr) {
    if (ctrl_ptr != nullptr) {
      ctrl_ptr->ref_count.fetch_add(1, mem::order_relaxed);
    }
  }

  /// Polymorphic aliasing constructor.
  template<typename U, EnableUpcast<U> = 0>
  cons SharedPointer(SharedPointer<U> const& other, T* alias_ptr) noexce
      : ctrl_ptr(other.ctrl_ptr), obj_ptr(alias_ptr) {
    if (ctrl_ptr != nullptr) {
      ctrl_ptr->ref_count.fetch_add(1, mem::order_relaxed);
    }
  }

  /// Adopting constructor.
  /// Constructs a `SharedPointer` adopting existing control block and object.
  /// Does *not* modify reference counters.
  SharedPointer(IControlBlock* base_ptr, T* obj_ptr, AdoptTag) noexce
  //  Required to enable (explicit) structural binding.
      : ctrl_ptr(base_ptr), obj_ptr(obj_ptr) {}


  /// Copy assignment operator.
  cons auto operator=(SharedPointer const& that) -> SharedPointer& {
    if (this == &that) {
      return *this;
    }
    if (that.ctrl_ptr != nullptr) {
      that.ctrl_ptr->ref_count.fetch_add(1, mem::order_relaxed);
    }
    if (ctrl_ptr != nullptr) {
      decrement_shared();
    }
    ctrl_ptr = that.ctrl_ptr;
    obj_ptr = that.obj_ptr;
    return *this;
  }

  /// Polymorphic copy assignment operator.
  template<typename U, EnableUpcast<U> = 0>
  cons auto operator=(SharedPointer<U> const& that) -> SharedPointer& {
    if (ctrl_ptr == that.ctrl_ptr) {
      return *this;
    }
    if (that.ctrl_ptr != nullptr) {
      that.ctrl_ptr->ref_count.fetch_add(1, mem::order_relaxed);
    }
    if (ctrl_ptr != nullptr) {
      decrement_shared();
    }
    ctrl_ptr = that.ctrl_ptr;
    obj_ptr = that.obj_ptr;
    return *this;
  }

  /// Move assignment operator
  cons auto operator=(SharedPointer&& that) noexce -> SharedPointer& {
    if (this == &that) {
      return *this;
    }
    if (ctrl_ptr != nullptr) {
      decrement_shared();
    }
    ctrl_ptr = that.ctrl_ptr;
    obj_ptr = that.obj_ptr;
    that.ctrl_ptr = nullptr;
    that.obj_ptr = nullptr;
    return *this;
  }

  /// Polymorphic move assignment operator.
  template<typename U, EnableUpcast<U> = 0>
  cons auto operator=(SharedPointer<U>&& that) noexce -> SharedPointer& {
    if (ctrl_ptr != nullptr) {
      decrement_shared();
    }
    ctrl_ptr = that.ctrl_ptr;
    obj_ptr = that.obj_ptr;
    that.ctrl_ptr = nullptr;
    that.obj_ptr = nullptr;
    return *this;
  }

  /// Destructor
  ~SharedPointer() { release(); }


  /// Release ownership and deinitialize the pointer.
  /// If the `ref_count` becomes zero, then the owned object is `removed`.
  cons auto release() noexce -> bool {
    if (ctrl_ptr == nullptr) {
      return false;
    }
    decrement_shared();
    ctrl_ptr = nullptr;
    obj_ptr = nullptr;
    return true;
  }

  /// Atomically decrements the `ref_count`.
  /// If the `ref_count` becomes zero, owned object is `removed`.
  inln cons auto decrement_shared() noexce -> void {
    if (ctrl_ptr->ref_count.fetch_sub(1, mem::order_release) == 1) {
      std::atomic_thread_fence(mem::order_acquire);
      ctrl_ptr->release(ctrl_ptr);
    }
  }

  inln cons operator bool() noexce { return ctrl_ptr != nullptr; }
};  // SharedPointer

/// Makes a new `SharedPointer` that is the first owner of a `T` instance.
template<typename T, typename... Args>
cons auto make_shared(Args&&... args) -> SharedPointer<T> {
  ControlBlock<T>* ctrl_ptr = mem::make<ControlBlock<T>>(1);
  if (ctrl_ptr == nullptr) {
    return {};
  }
  mem::place<T>(ctrl_ptr->placement_ptr(), 1, cx_forward_args(args)...);
  if (ctrl_ptr->object_ptr() == nullptr) {
    return {};
  }
  ctrl_ptr->ref_count.store(1, mem::order_relaxed);
  return {ctrl_ptr, ctrl_ptr->object_ptr(), SharedPointer<T>::adopt_tag};
}

/// Decrements the reference counter of `p`.
/// Requires: `p.ctrl_ptr` must not be `nullptr`.
// inln cons auto decrement_shared(Shared_Pointer& p) -> void {
//   if (p.ctrl_ptr->ref_count.fetch_sub(1, mem::order_release) == 1) {
//     std::atomic_thread_fence(mem::order_acquire);
//     p.ctrl_ptr->release(p.ctrl_ptr);
//   }
// }

// /// Release ownership held by `p`.
// /// If the counter reaches zero, the pointee is removed.
// /// Returns: `true` if released, `false` if `p` was already null.
// cons auto release_shared(Shared_Pointer& p) -> bool {
//   if (!p) {
//     return false;
//   }
//   decrement_shared(p);
//   p.ctrl_ptr = nullptr;
//   p.obj_ptr = nullptr;
//   return true;
// }

}  // namespace cx::mem

#ifdef CX_TEST
void test_make_shared() {
  auto p = cx::mem::make_shared<int>(42);
  printf("%d\n", *p.obj_ptr);
}
#endif
