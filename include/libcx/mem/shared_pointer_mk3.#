/** @file libcx/mem/shared_pointer_mk3.hh **/
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
CX_CONCEPT_GEN_TEMPL(
    SharedPointer, is_shared_pointer, CSharedPointer, typename T, T);
#define Control_Block_Base cx::mem::CControlBlockBase auto
#define Control_Block_Impl cx::mem::CControlBlock auto
#define Shared_Pointer cx::mem::CSharedPointer auto
typedef fn DeleterProc(IControlBlock*) noexce -> void;
inln cons fn priv__decrement_shared(Shared_Pointer& p) -> void;
cons fn priv__release_shared(Shared_Pointer& p) -> bool;

/// Control block interface.
struct IControlBlock {
  /// The reference counter.
  std::atomic<usize> ref_count{0};
  /// Type erased release interface.
  DeleterProc* release{nullptr};
};

/// Type erased control block encapsulating the element type `T`.
template<typename T> struct ControlBlock final : IControlBlock {
  /// The encalsupated element type.
  using Elem = T;
  /// The internal type erased storage.
  alignas(Elem) u8 storage[size_of(T)]{};
  /// Default constructor.
  cons ControlBlock() noexce { release = __release; }
  /// Typed release implementation.
  inln glob cons fn __release(IControlBlock* base) noexce -> void
  {
    auto* self = static_cast<ControlBlock<T>*>(base);
    if cons (!std::is_trivially_destructible_v<T>) {
      get_object(self)->~T();
    }
    mem::dealloc<ControlBlock<T>>(self);
  }
};

///
template<typename T> struct SharedPointer {
  static_assert(
      !is_void<T>, "SharedPointer<T>: the element type T must not be 'void'");
  static_assert(
      !is_raw_array<T>, "SharedPointer<T>: T must not be an array type.");
  static_assert(
      !is_ref<T> && !is_raw_ptr<T>,
      "SharedPointer<T>: T must not be a reference or pointer type.");
  static_assert(
      is_dtble<T>,
      "SharedPointer<T>: the element type T must be destructible. "
      "The expression 'declval<T&>().~T()' must be accessible.");
  static_assert(
      std::is_nothrow_destructible_v<T>,
      "SharedPointer<T>: destructor of T must be noexce.");
  static_assert(
      !std::is_polymorphic_v<T> || std::has_virtual_destructor_v<T> ||
          std::is_final_v<T>,
      "SharedPointer<T>: polymorphic base T must have a virtual "
      "destructor or be declared final");

  using Elem = T;
  template<typename U> using EnableUpcast = std::enable_if<std::is_convertible_v<U*, T*>, int>;
  glob onedef cons struct AdoptTag {} adopt_tag{};

  /// The pointer to control block.
  IControlBlock* ctrl_ptr{nullptr};
  /// The pointer to the owned object.
  Elem* obj_ptr{nullptr};

  /// Default constructor.
  cons SharedPointer() = default;

  SharedPointer& operator=(const SharedPointer&) = delete;
  SharedPointer& operator=(SharedPointer&&) = delete;
  /// Copy constructor.
  cons SharedPointer(SharedPointer const& that) noexce 
      : ctrl_ptr(that.ctrl_ptr),
        obj_ptr(that.obj_ptr)
  {
    if (ctrl_ptr != nullptr) {
      ctrl_ptr->ref_count.fetch_add(1, mem::order_relaxed);
    }
  }

  /// Polymorphic copy constructor.
  template<typename U, EnableUpcast<U> = 0>
  SharedPointer(SharedPointer<U> const& that) noexce
      : ctrl_ptr(that.ctrl_ptr),
        obj_ptr(static_cast<T*>(that.obj_ptr))
  {
    if (ctrl_ptr != nullptr) {
      ctrl_ptr->ref_count.fetch_add(1, mem::order_relaxed);
    }
  }

  /// Move constructor.
  cons SharedPointer(SharedPointer&& that) noexce 
      : ctrl_ptr(that.ctrl_ptr),
        obj_ptr(that.obj_ptr)
  {
    that.ctrl_ptr = nullptr;
    that.obj_ptr = nullptr;
  }

  /// Polymorphic move constructor
  template<typename U, EnableUpcast<U> = 0>
  cons SharedPointer(SharedPointer<U>&& that) noexce
      : ctrl_ptr(that.ctrl_ptr),
        obj_ptr(static_cast<T*>(that.obj_ptr))
  {
    that.ctrl_ptr = nullptr;
    that.obj_ptr = nullptr;
  }

  /// Aliasing contructor.
  template<typename U>
  cons SharedPointer(SharedPointer<U> const& that, T* alias_ptr) noexce
      : ctrl_ptr(that.ctrl_ptr),
        obj_ptr(alias_ptr)
  {
    if (ctrl_ptr != nullptr) {
      ctrl_ptr->ref_count.fetch_add(1, mem::order_relaxed);
    }
  }

  /// Polymorphic aliasing constructor.
  template<typename U, EnableUpcast<U> = 0>
  cons SharedPointer(SharedPointer<U> const& that, T* alias_ptr) noexce
      : ctrl_ptr(that.ctrl_ptr),
        obj_ptr(alias_ptr)
  {
    if (ctrl_ptr != nullptr) {
      ctrl_ptr->ref_count.fetch_add(1, mem::order_relaxed);
    }
  }

  /**
    Adopting constructor.
    Constructs a `SharedPointer` adopting existing control block and object.
    Does *not* modify reference counters.
  **/
  SharedPointer(IControlBlock* base_ptr, T* obj_ptr, AdoptTag) noexce
    : ctrl_ptr(base_ptr) , obj_ptr(obj_ptr)
  {
  }

  /// Copy assignment operator.
  cons SharedPointer& operator=(this auto& self, SharedPointer const& that)
  {
    if (&self == &that) {
      return self;
    }
    if (that.ctrl_ptr != nullptr) {
      that.ctrl_ptr->ref_count.fetch_add(1, mem::order_relaxed);
    }
    if (self.ctrl_ptr != nullptr) {
      mem::priv__decrement_shared(self);
    }
    self.ctrl_ptr = that.ctrl_ptr;
    self.obj_ptr = that.obj_ptr;
    return self;
  }

  /// Polymorphic copy assignment operator.
  template<typename U, EnableUpcast<U> = 0>
  cons SharedPointer& operator=(this auto& self, SharedPointer<U> const& that)
  {
    if (self.ctrl_ptr == that.ctrl_ptr) {
      return self;
    }
    if (that.ctrl_ptr != nullptr) {
      that.ctrl_ptr->ref_count.fetch_add(1, mem::order_relaxed);
    }
    if (self.ctrl_ptr != nullptr) {
      mem::priv__decrement_shared(self);
    }
    self.ctrl_ptr = that.ctrl_ptr;
    self.obj_ptr = that.obj_ptr;
    return self;
  }

  /// Move assignment operator
  cons SharedPointer& operator=(this auto& self, SharedPointer&& that) noexce
  {
    if (&self == &that) {
      return self;
    }
    if (self.ctrl_ptr != nullptr) {
      mem::priv__decrement_shared(self);
    }
    self.ctrl_ptr = that.ctrl_ptr;
    self.obj_ptr = that.obj_ptr;
    that.ctrl_ptr = nullptr;
    that.obj_ptr = nullptr;
    return self;
  }

  /// Polymorphic move assignment operator.
  template<typename U, EnableUpcast<U> = 0>
  cons SharedPointer& operator=(this auto& self, SharedPointer<U>&& that) noexce
  {
    if (self.ctrl_ptr != nullptr) {
      mem::priv__decrement_shared(self);
    }
    self.ctrl_ptr = that.ctrl_ptr;
    self.obj_ptr = that.obj_ptr;
    that.ctrl_ptr = nullptr;
    that.obj_ptr = nullptr;
    return self;
  }

  /// Destructor
  ~SharedPointer() { mem::priv__release_shared(*this); }

  inln cons operator bool() noexce { return ctrl_ptr != nullptr; }
};  // SharedPointer

/// Makes a new `SharedPointer` that is the first owner of a `T` instance.
template<typename T, typename... Args>
cons fn make_shared(Args&&... args) -> SharedPointer<T>
{
  ControlBlock<T>* ctrl_ptr = mem::make<ControlBlock<T>>(1);
  if (ctrl_ptr == nullptr) {
    return {};
  }
  mem::Init<T>(uti::get_place(*ctrl_ptr), 1, cx_forward_args(args)...);
  if (uti::get_object(*ctrl_ptr) == nullptr) {
    return {};
  }
  ctrl_ptr->ref_count.store(1, mem::order_relaxed);
  return {ctrl_ptr, uti::get_object(*ctrl_ptr), SharedPointer<T>::adopt_tag};
}

/// Decrements the reference counter of `p`.
/// Requires: `p.ctrl_ptr` must not be `nullptr`.
inln cons fn priv__decrement_shared(Shared_Pointer& p) -> void
{
  if (p.ctrl_ptr->ref_count.fetch_sub(1, mem::order_release) == 1) {
    std::atomic_thread_fence(mem::order_acquire);
    p.ctrl_ptr->release(p.ctrl_ptr);
  }
}

/// Release ownership held by `p`.
/// If the counter reaches zero, the pointee is removed.
/// Returns: `true` if released, `false` if `p` was already null.
cons fn priv__release_shared(Shared_Pointer& p) -> bool
{
  if (!p) {
    return false;
  }
  mem::priv__decrement_shared(p);
  p.ctrl_ptr = nullptr;
  p.obj_ptr = nullptr;
  return true;
}

// NOTE(manu)
// In order to achieve a procedular-like style API I can reason about mirror some
// speciae members with standolone functions.
// but this can be a bad idea since this breaks encapsulation that i think  is
// inherently fundamental for this type of automatic memory management structure.

// template<typename T> inln cons SharedPointer<T>
// copy_create(SharedPointer<T> const& src) noexce {
//   return SharedPointer<T>(src);
// }

// template<typename T>
// inln cons SharedPointer<T> move_create(SharedPointer<T>&& src) noexce {
//   return SharedPointer<T>(uti::move(src));
// }

// inln cons void copy_assign(Shared_Pointer& dst, Shared_Pointer const& src) noexce {
//   dst = src;
// }

// inln cons void move_assign(Shared_Pointer& dst, Shared_Pointer&& src) noexce {
//   dst = uti::move(src);
// }

}  // namespace cx::mem

#ifdef CX_TEST
void test_make_shared()
{
  using namespace cx;
  auto p = mem::make_shared<int>(42);
  printf("before:\n");
  printf("%d\n", *p.obj_ptr);
  mem::print_full_ptr(p.obj_ptr);
  Shared_Pointer q = mem::copy_create(p);
  (*q.obj_ptr)++;
  printf("after:\n");
  printf("%d\n", *p.obj_ptr);
  printf("%d\n", *q.obj_ptr);
  mem::print_full_ptr(p.obj_ptr);
  mem::print_full_ptr(q.obj_ptr);
}
#endif
