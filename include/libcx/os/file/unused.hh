
// gb_inline isize gb_fprintf_va(struct gbFile* f, char const* fmt, va_list va) {
//   char buf[4096];
//   va_list va_save;
//   va_copy(va_save, va);
//   isize len = gb_snprintf_va(buf, gb_size_of(buf), fmt, va_save);
//   va_end(va_save);
//   char* new_buf = NULL;
//   isize n = gb_size_of(buf);
//   while (len < 0) {
//     va_copy(va_save, va);
//     defer(va_end(va_save));
//     n <<= 1;
//     gb_free(gb_heap_allocator(), new_buf);
//     new_buf = gb_alloc_array(gb_heap_allocator(), char, n);
//     ;
//     len = gb_snprintf_va(new_buf, n, fmt, va_save);
//   }
//   if (new_buf != NULL) {
//     gb_file_write(f, new_buf,
//                   len - 1);  // NOTE(bill): prevent extra whitespace
//     gb_free(gb_heap_allocator(), new_buf);
//   } else {
//     gb_file_write(f, buf,
//                   len - 1);  // NOTE(bill): prevent extra whitespace
//   }
//   return len;
// }

// gb_no_inline isize gb_snprintf_va(char* text, isize max_len, char const* fmt, va_list va) {
//   char const* text_begin = text;
//   isize remaining = max_len - 1, res;
//
//   while (*fmt && remaining > 0) {
//     gbprivFmtInfo info = {0};
//     isize len = 0;
//     info.precision = -1;
//
//     while (remaining > 0 && *fmt && *fmt != '%') {
//       *text++ = *fmt++;
//       remaining--;
//     }
//
//     if (*fmt == '%') {
//       do {
//         switch (*++fmt) {
//         case '-':
//           info.flags |= gbFmt_Minus;
//           break;
//         case '+':
//           info.flags |= gbFmt_Plus;
//           break;
//         case '#':
//           info.flags |= gbFmt_Alt;
//           break;
//         case ' ':
//           info.flags |= gbFmt_Space;
//           break;
//         case '0':
//           info.flags |= gbFmt_Zero;
//           break;
//         default:
//           info.flags |= gbFmt_Done;
//           break;
//         }
//       } while (!(info.flags & gbFmt_Done));
//     }
//
//     // NOTE(bill): Optional Width
//     if (*fmt == '*') {
//       int width = va_arg(va, int);
//       if (width < 0) {
//         info.flags |= gbFmt_Minus;
//         info.width = -width;
//       } else {
//         info.width = width;
//       }
//       fmt++;
//     } else {
//       info.width = cast(i32) gb_str_to_i64(fmt, cast(char**) & fmt, 10);
//     }
//
//     // NOTE(bill): Optional Precision
//     if (*fmt == '.') {
//       fmt++;
//       if (*fmt == '*') {
//         info.precision = va_arg(va, int);
//         fmt++;
//       } else {
//         info.precision = cast(i32) gb_str_to_i64(fmt, cast(char**) & fmt, 10);
//       }
//       info.flags &= ~gbFmt_Zero;
//     }
//
//
//     switch (*fmt++) {
//     case 'h':
//       if (*fmt == 'h') {  // hh => char
//         info.flags |= gbFmt_Char;
//         fmt++;
//       } else {  // h => short
//         info.flags |= gbFmt_Short;
//       }
//       break;
//
//     case 'l':
//       if (*fmt == 'l') {  // ll => long long
//         info.flags |= gbFmt_Llong;
//         fmt++;
//       } else {  // l => long
//         info.flags |= gbFmt_Long;
//       }
//       break;
//
//       break;
//
//     case 'z':  // NOTE(bill): usize
//       info.flags |= gbFmt_Unsigned;
//       // fallthrough
//     case 't':  // NOTE(bill): isize
//       info.flags |= gbFmt_Size;
//       break;
//
//     default:
//       fmt--;
//       break;
//     }
//
//
//     switch (*fmt) {
//     case 'u':
//       info.flags |= gbFmt_Unsigned;
//       // fallthrough
//     case 'd':
//     case 'i':
//       info.base = 10;
//       break;
//
//     case 'o':
//       info.base = 8;
//       break;
//
//     case 'x':
//       info.base = 16;
//       info.flags |= (gbFmt_Unsigned | gbFmt_Lower);
//       break;
//
//     case 'X':
//       info.base = 16;
//       info.flags |= (gbFmt_Unsigned | gbFmt_Upper);
//       break;
//
//     case 'f':
//     case 'F':
//     case 'g':
//     case 'G':
//       len = gb__print_f64(text, remaining, &info, va_arg(va, f64));
//       break;
//
//     case 'a':
//     case 'A':
//       // TODO(bill):
//       break;
//
//     case 'c':
//       len = gb__print_char(text, remaining, &info, cast(char) va_arg(va, int));
//       break;
//
//     case 's':
//       len = gb__print_string(text, remaining, &info, va_arg(va, char*));
//       break;
//
//     case 'p':
//       info.base = 16;
//       info.flags |= (gbFmt_Lower | gbFmt_Unsigned | gbFmt_Alt | gbFmt_Intptr);
//       break;
//
//     case '%':
//       len = gb__print_char(text, remaining, &info, '%');
//       break;
//
//     default:
//       fmt--;
//       break;
//     }
//
//     fmt++;
//
//     if (info.base != 0) {
//       if (info.flags & gbFmt_Unsigned) {
//         u64 value = 0;
//         switch (info.flags & gbFmt_Ints) {
//         case gbFmt_Char:
//           value = cast(u64) cast(u8) va_arg(va, int);
//           break;
//         case gbFmt_Short:
//           value = cast(u64) cast(u16) va_arg(va, int);
//           break;
//         case gbFmt_Long:
//           value = cast(u64) va_arg(va, unsigned long);
//           break;
//         case gbFmt_Llong:
//           value = cast(u64) va_arg(va, unsigned long long);
//           break;
//         case gbFmt_Size:
//           value = cast(u64) va_arg(va, usize);
//           break;
//         case gbFmt_Intptr:
//           value = cast(u64) va_arg(va, uintptr);
//           break;
//         default:
//           value = cast(u64) va_arg(va, unsigned int);
//           break;
//         }
//
//         len = gb__print_u64(text, remaining, &info, value);
//
//       } else {
//         i64 value = 0;
//         switch (info.flags & gbFmt_Ints) {
//         case gbFmt_Char:
//           value = cast(i64) cast(i8) va_arg(va, int);
//           break;
//         case gbFmt_Short:
//           value = cast(i64) cast(i16) va_arg(va, int);
//           break;
//         case gbFmt_Long:
//           value = cast(i64) va_arg(va, long);
//           break;
//         case gbFmt_Llong:
//           value = cast(i64) va_arg(va, long long);
//           break;
//         case gbFmt_Size:
//           value = cast(i64) va_arg(va, usize);
//           break;
//         case gbFmt_Intptr:
//           value = cast(i64) va_arg(va, uintptr);
//           break;
//         default:
//           value = cast(i64) va_arg(va, int);
//           break;
//         }
//
//         len = gb__print_i64(text, remaining, &info, value);
//       }
//     }
//
//
//     text += len;
//     if (len >= remaining) {
//       remaining = 0;
//     } else {
//       remaining -= len;
//     }
//   }
//
//   *text++ = '\0';
//   res = (text - text_begin);
//   return (res >= max_len || res < 0) ? -1 : res;
// }
