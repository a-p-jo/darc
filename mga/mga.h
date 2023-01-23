#ifndef MGA_H
#define MGA_H

#include <stdbool.h> /* bool, true, false */
#include <stddef.h>  /* size_t            */

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif                                                                             
 
/* Declares an instantiation with given name, alloction functions,
 * scope and "..." element type.
 *
 * Where,
 * - "scope" is empty or a valid prefix for a function declaration,
 *   like static, inline, etc.
 * - "name" is a valid identifier.
 * - "reallocfn" is a function or pointer to function rvalue
 *    that follows stdlib realloc's ABI.
 * - "freefn" is a function or pointer to function rvalue
 *   that follows stdlib free's ABI.
 * - "..." is a type name such that a suffixed "*" creates
 *   a pointer to that type.
 *
 * Example : MGA_DECL(, ivec, realloc, free, int)
 * Declares ivec for ints with functions in the global scope
 * using stdlib realloc/free for allocation/deallocation.
 *
 * - Member types :
 *   - name_eltype, the type of the elements (aka value_type).
 * - Member constants : 
 *   - name_maxcap, the maxmimum number of elements.
 *   - name_realloc, name_free; aliases of reallocfn(),freefn().
 *
 * - Member functions :
 *   - name_create()
 *   - name_destroy()
 *   - name_reserve()
 *   - name_insert()
 *   - name_selfinsert()
 *   - name_remove()
 *   - name_shrink_to_fit()
 */
#define MGA_DECL(scope, name, ...)                         \
typedef __VA_ARGS__ name##_eltype;                                            \
typedef struct name { size_t len, cap; name##_eltype *arr; } name;            \
									      \
static const size_t name##_maxcap = SIZE_MAX/sizeof(name##_eltype);           \
									      \
scope name name##_create(size_t);                                             \
scope void name##_destroy(name *);                                            \
scope bool name##_reserve(name *, size_t);                                    \
scope bool name##_insert(name *, size_t i, const name##_eltype *restrict src, \
								   size_t n); \
scope bool name##_selfinsert(name *foo, size_t idst, size_t isrc, size_t n);  \
scope bool name##_remove(name *, size_t i, size_t n);		              \
scope void name##_shrink_to_fit(name *);                                      \

/* Define MGA_NOIMPL to strip implementation code */
#ifndef MGA_NOIMPL

#include <string.h>  /* memcpy(), memmove() */

/* Expands function definitons for previously MGA_DECL()'d name */
#define MGA_DEF(scope, name, reallocfn, freefn)                               \
static void *(*const name##_realloc)(void *, size_t) = reallocfn;             \
static void (*const name##_free)(void *) = freefn;                            \
									      \
scope name name##_create(size_t n)                                            \
{                                                                             \
	enum { elsz = sizeof(name##_eltype) };                                \
									      \
	name res = {0};                                                       \
	if (n && n <= name##_maxcap                                           \
			&& (res.arr = name##_realloc(NULL, n*elsz)) )         \
		res.cap = n;                                                  \
	return res;                                                           \
}                                                                             \
									      \
scope void name##_destroy(name *foo)                                          \
{                                                                             \
	if (foo)                                                              \
		name##_free(foo->arr), *foo = (name){0};                      \
}                                                                             \
									      \
scope bool name##_reserve(name *foo, size_t n)                                \
{                                                                             \
	enum { elsz = sizeof(name##_eltype) };                                \
									      \
	if (foo && n <= name##_maxcap) {                                      \
		register size_t cap = foo->cap;                               \
									      \
		if (cap < n) {                                                \
			size_t newcap = cap+cap/2; /* Try growing 1.5x */     \
			/* Or grow to n elements if its bigger or overflow */ \
			if (newcap < n || newcap > name##_maxcap)             \
				newcap = n;                                   \
									      \
			void *p = name##_realloc(foo->arr, newcap*elsz);      \
			if (p)                                                \
				foo->arr = p, foo->cap = newcap;              \
			else                                                  \
				return false;                                 \
		}                                                             \
		return true;                                                  \
	} else                                                                \
		return false;                                                 \
}                                                                             \
									      \
scope bool name##_insert(name *dst, size_t i,                                 \
		const name##_eltype *restrict src, size_t n)                  \
{                                                                             \
	enum { elsz = sizeof(name##_eltype) };                                \
									      \
	if (!n)                                                               \
		return true;                                                  \
									      \
	register size_t len;                                                  \
	if (dst && name##_maxcap-n >= (len = dst->len) && i <= len            \
			&& name##_reserve(dst, len+n)) {                      \
		register name##_eltype *arr = dst->arr;                       \
									      \
		/* move elements at i to i+n to preserve them */              \
		memmove(arr+i+n, arr+i, (len-i)*elsz);                        \
									      \
		/* if src is NULL, caller will emplace, don't copy */         \
		if (src)                                                      \
			memcpy(arr+i, src, n*elsz);                           \
									      \
		dst->len = len+n;                                             \
		return true;                                                  \
	} else                                                                \
		return false;                                                 \
}                                                                             \
									      \
scope bool name##_selfinsert(name *foo, size_t idst, size_t isrc, size_t n)   \
{                                                                             \
	enum { elsz = sizeof(name##_eltype) };                                \
									      \
	if (!n)                                                               \
		return true;                                                  \
									      \
	register size_t len;                                                  \
	if (foo && name##_maxcap-n >= (len = foo->len) && idst <= len         \
		&& isrc < len && name##_reserve(foo, len+n)) {                \
		register name##_eltype *arr = foo->arr;                       \
									      \
		memmove(arr+idst+n, arr+idst, (len-idst)*elsz);               \
		/* If idst < isrc, isrc has moved n ahead.
		 * If idst == isrc, don't copy.
		 */                                                           \
		memmove(arr+idst, arr+isrc + (idst < isrc)*n,                 \
				n*elsz * (idst != isrc));                     \
									      \
		foo->len = len+n;                                             \
		return true;                                                  \
	} else                                                                \
		return false;                                                 \
}                                                                             \
									      \
scope bool name##_remove(name *dst, size_t i, size_t n)                       \
{                                                                             \
	enum { elsz = sizeof(name##_eltype) };                                \
									      \
	register name m;                                                      \
	if (dst && name##_maxcap-i >= n && i+n <= (m = *dst).len) {           \
									      \
		/* Shift elements at index > i one step back */               \
		memmove(m.arr+i, m.arr+i+n, (m.len-i-n)*elsz);                \
									      \
		dst->len = m.len-n;                                           \
		return true;                                                  \
	} else                                                                \
		return false;                                                 \
}                                                                             \
									      \
scope void name##_shrink_to_fit(name *foo)                                    \
{                                                                             \
	enum { elsz = sizeof(name##_eltype) };                                \
									      \
	/* Avoid reallocation if not needed */                                \
	register name m;                                                      \
	if (foo && (m = *foo).cap > m.len) {                                  \
		void *p = name##_realloc(m.arr, m.len*elsz);                  \
		if (p)                                                        \
			foo->arr = p, foo->cap = m.len;                       \
	}                                                                     \
}                                                                             \

#define MGA_IMPL(name, reallocfn, freefn, ...)                                \
	MGA_DECL(static inline, name, __VA_ARGS__)                            \
	MGA_DEF(static inline, name, reallocfn, freefn)

#endif
#endif
