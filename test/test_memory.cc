/// @file
#include <catch/catch_amalgamated.hh>
#include <cx/io/io.hh>
#include <cx/mem/memory.hh>


using CX;

proc test_memory_zii_make() -> void {
  struct {
    u32 x = 42;
    u32 y = 43;
  } a;
  using A = decltype(a);

  auto [p, err1] = mem::make<A>(8);
  REQUIRE(err1 == nullptr);
  defer(free(p));
  for (usz i = 0; i < 16; i++) {
    io::printf("{} ", *(cast(u32*) p + i));
  }
  io::printn("");

  auto [q, err2] = mem::make<A>(8);
  REQUIRE(err2 == nullptr);
  defer(free(q));
  for (usz i = 0; i < 8; i++) {
    io::printf("{} ", *(cast(i32*) q + i));
  }
  io::printn("");

  auto [r, err3] = mem::make<A>(8);
  REQUIRE(err3 == nullptr);
  defer(free(r));
  for (usz i = 0; i < 8; i++) {
    io::printf("{:.3f} ", *(cast(f64*) p + i));
  }
  io::printn("");
}

proc test_over_aligned_single() -> void {
  struct alignas(64) OA {
    u64 v;
  };

  auto [p, err] = mem::make<OA>(4);
  REQUIRE(err == nullptr);

  std::nullptr_t np;

  uptr addr = reinterpret_cast<uptr>(p);
  REQUIRE(addr % alignof(OA) == 0);

  auto* a = cast(OA*) p;
  for (usz i = 0; i < 4; ++i) {
    REQUIRE(a[i].v == 0);
  }

  mem::deallocate<OA>(p);
}

// proc test_over_aligned_multi() -> void {
//   struct alignas(64) A {
//     int x;
//   };
//   struct alignas(16) B {
//     int y;
//   };
//   usz n = 3;
//
//   void* base = mem::make_multi_zii<A, B>(n);
//   REQUIRE(base != null);
//
//   void* tmp = base;
//
//   uptr u = reinterpret_cast<uptr>(tmp);
//   u = mem::align_up(u, alignof(A));
//   auto* pa = reinterpret_cast<A*>(u);
//
//   tmp = pa + n;
//
//   u = reinterpret_cast<uptr>(tmp);
//   u = mem::align_up(u, alignof(B));
//   auto* pb = reinterpret_cast<B*>(u);
//
//   REQUIRE(reinterpret_cast<uptr>(pa) % alignof(A) == 0);
//   REQUIRE(reinterpret_cast<uptr>(pb) % alignof(B) == 0);
//
//   for (usz i = 0; i < n; ++i) {
//     REQUIRE(pa[i].x == 0);
//     REQUIRE(pb[i].y == 0);
//   }
//
//   cexpr uptr max_align = (std::max) ({static_cast<uptr>(alignof(A)),
//                                           static_cast<uptr>(alignof(B))});
//   struct alignas(max_align) MaxAlign {
//     u8 b;
//   };
//   mem::deallocate<MaxAlign>(base);
// }



TEST_CASE("test_memory") {
  test_memory_zii_make();
  test_over_aligned_single();
  // test_over_aligned_multi();
}
