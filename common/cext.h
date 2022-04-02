#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <assert.h>
#include <stdalign.h>

// macro helpers
#define __static_assert_decl(const_expr)    static_assert(const_expr, #const_expr)
#define __static_assert_expr(const_expr)    ( 0 * sizeof(struct { __static_assert_decl(const_expr); }) )

#define __must_be_array(a)  __static_assert_expr(!__same_type((a), &(a)[0]))
#define __same_type(a, b)   __builtin_types_compatible_p(typeof(a), typeof(b))

#define __choose_expr(const_expr, expr_true, expr_false)    __builtin_choose_expr(const_expr, expr_true, expr_false)

// new keywords
#define private         static
#define public
#define in              const
#define out
#define in_out
#define inline          __attribute__((always_inline))
#define no_inline       __attribute__((noinline))
#define nullable
#define nonnull
#define constants       enum
#define big_endian      __attribute__((scalar_storage_order("big-endian")))
#define little_endian   __attribute__((scalar_storage_order("little-endian")))

// useful macros
#define containerof(ptr, type, member) ( (type *)((char*)ptr - offsetof(type, member)) )
#define lengthof(array) (sizeof(array) / sizeof((array)[0]) + __must_be_array(array))

// GNU C extensions
#define auto_t __auto_type

/* Common Options:
 *  Functions := cdecl, fastcall, thiscall, ms_abi, sysv_abi
 *  Structs   := ms_struct, gcc_struct
 */
#define attr_abi(abi)               __attribute__((abi))
#define attr_section(section_name)  __attribute__((section(#section_name)))
#define attr_packed                 __attribute__((packed))
#define attr_naked                  __attribute__((naked))
#define attr_aligned(alignment)     __attribute__((packed, aligned(alignment)))
#define attr_unused                 __attribute__((unused))
#define attr_noderef                __attribute__((noderef))

#define OPTIMIZE_UNREACHABLE    __builtin_unreachable()
#define OPTIMIZE_ASSUME(expr)   do { if (!(expr)) OPTIMIZE_UNREACHABLE; } while (0)

// TODO: write _Generic macro to print the value in the ptr depending on type
#define DEBUG_DUMP_VARIABLE(variable_ptr, printf_ptr)   do { printf_ptr(#variable_ptr); } while (0)
#define DEBUG_DUMP_STRUCT(struct_ptr, printf_ptr)       do { printf_ptr(#struct_ptr); __builtin_dump_struct(struct_ptr, printf_ptr); } while (0)

// for missing math shit
// TODO: make these compatible with different compilers and add some fallback for ISO C
#ifndef INF
#   define INF (__builtin_inff())
#endif

#ifndef NAN
#   define NAN (__builtin_nanf(""))
#endif
