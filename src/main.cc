/** @file src/main.cc **/

#include <stdio.h>

#include "libcx/conf/type.hh"

// using CX;

// Result<fast_io::c_file> test_open_file() {
//   auto [file, err] = (io::fopen("main.cc", "r"));
//   if (err) {
//       return Result{uti::take(file), uti::take(err)};
//   }
//   io::printfn("Hello from `test_open_file`: {}", (void*)file.fp);
//   return {std::move(file), nullptr};
// }

// int main() {
//   auto [file, err] = test_open_file();
//   if (err) {
//     io::printfn("{}", err::get_message(err));
//     return 1;
//   }
//   io::printfn("Hello from `main`: {}", (void*)file.fp);
// }

fn test_dummy() -> void
{
    printf("Hello, this is a dummy test\n");
}

int main()
{
    test_dummy();
}
