#ifndef SBOMGA_H
#define SBOMGA_H

#include <stdbool.h> /* bool, true, false */
#include <stddef.h>  /* size_t            */
#include <limits.h>  /* CHAR_BIT          */

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

#define SBOMGA_MAX(x,y) ( (x) > (y) ? (x) : (y) )
#define SBOMGA_NBITS(x) ( CHAR_BIT * sizeof(x)  )

/* Declares an instantiation with given name, alloction functions,
 * short-buffer capacity, scope and "..." element type.
 *
 * Where,
 * - "scope" is empty or a valid prefix for a function declaration,
 *   like static, inline, etc.
 * - "name" is a valid identifier.
 * - "reallocfn" is a function or pointer to function rvalue
 *    that follows stdlib realloc's ABI.
 * - "freefn" is a function or pointer to function rvalue
 *   that follows stdlib free's ABI.
 * - "sbocap" is an integer rvalue sans side effects.
 * - "..." is a type name such that a suffixed "*" creates
 *   a pointer to that type.
 *
 * Example : MGA_DECL(, str, realloc, free, 0, char)
 * Declares str for chars with functions in the global scope
 * using stdlib realloc/free for allocation/deallocation
 * and the default sbocap.
 *
 * - Member types :
 *   - name_eltype, the type of the elements (aka value_type).
 * - Member constants :
 *   - name_sbocap, the actual capacity of the short-buffer.
 *     Is at least as much as the requested capacity.
 *   - name_maxcap, the maxmimum number of elements.
 *   - name_realloc, name_free; aliases of reallocfn(),freefn().
 *
 * - Member functions :
 *   - name_arr()
 *   - name_cap()
 *   - name_create()
 *   - name_destroy()
 *   - name_reserve()
 *   - name_insert()
 *   - name_selfinsert()
 *   - name_remove()
 *   - name_shrink_to_fit()
 */
#define SBOMGA_DECL(scope, name, reallocfn, freefn, sbocap, ...)              \
typedef __VA_ARGS__ name##_eltype;                                            \
								              \
enum {                                                                        \
	name##_sbocap = SBOMGA_MAX(SBOMGA_MAX(sbocap, 1),                     \
	sizeof(struct {name##_eltype *p; size_t q;}) / sizeof(name##_eltype)) \
};								              \
                                                                              \
typedef struct name {                                                         \
	union {                                                               \
		struct { name##_eltype *arr; size_t cap; };                   \
		name##_eltype sbo[name##_sbocap];                             \
	};                                                                    \
	size_t len : SBOMGA_NBITS(size_t)-1;                                  \
	bool big : 1;                                                         \
} name;                                                                       \
                                                                              \
static const size_t name##_maxcap =                                           \
	SIZE_MAX/SBOMGA_MAX(sizeof(name##_eltype), 2); /* -1 bit -> max/2 */  \
static void *(*const name##_realloc)(void *, size_t) = reallocfn;             \
static void (*const name##_free)(void *) = freefn;                            \
								              \
scope name##_eltype *name##_arr(const name *);                                \
scope size_t name##_cap(const name *foo);                                     \
								              \
scope name name##_create(size_t);                                             \
scope void name##_destroy(name *);                                            \
scope bool name##_reserve(name *, size_t);                                    \
scope bool name##_insert(name *, size_t i,                                    \
			const name##_eltype *restrict src, size_t n);         \
scope bool name##_selfinsert(name *foo, size_t idst, size_t isrc, size_t n);  \
scope bool name##_remove(name *, size_t i, size_t n);		              \
scope void name##_shrink_to_fit(name *);                                      \

/* Define SBOMGA_NOIMPL to strip implementation code */
#ifndef SBOMGA_NOIMPL

#include <string.h>  /* memcpy(), memmove() */

/* Expands function definitons for previously MGA_DECL()'d name */
#define SBOMGA_DEF(scope, name)                                               \
scope name##_eltype *name##_arr(const name *foo)                              \
{                                                                             \
	if (!foo)                                                             \
		return NULL;                                                  \
	else if (foo->big)                                                    \
		return foo->arr;                                              \
	else                                                                  \
		return (name##_eltype *)foo->sbo; /* Const cast */            \
}                                                                             \
								              \
scope size_t name##_cap(const name *foo)                                      \
{                                                                             \
	if (!foo)                                                             \
		return 0;                                                     \
	else if (foo->big)                                                    \
		return foo->cap;                                              \
	else                                                                  \
		return name##_sbocap;                                         \
}                                                                             \
								              \
scope name name##_create(size_t n)                                            \
{                                                                             \
	enum { elsz = sizeof(name##_eltype) };                                \
	                                                                      \
	name res = { .big = n > name##_sbocap };                              \
	if (res.big && n <= name##_maxcap       	                      \
			   && (res.arr = name##_realloc(NULL, n*elsz)))       \
		res.cap = n;                                                  \
	return res;                                                           \
}                                                                             \
								              \
scope void name##_destroy(name *foo)                                          \
{                                                                             \
	if (foo) {                                                            \
		if (foo->big)                                                 \
			name##_free(foo->arr), foo->big = false;              \
		foo->len = 0;                                                 \
	}                                                                     \
}                                                                             \
								              \
scope bool name##_reserve(name *foo, size_t n)                                \
{                                                                             \
	enum { elsz = sizeof(name##_eltype) };                                \
                                                                              \
	if (foo && n <= name##_maxcap) {                                      \
		register bool big = foo->big;                                 \
		register size_t cap = big? foo->cap : name##_sbocap;          \
		                                                              \
		if (cap < n) {                                                \
			size_t newcap = cap+cap/2; /* Try growing 1.5x */     \
			/* Or grow to n elements if its bigger or overflow */ \
			if (newcap < n || newcap > name##_maxcap)             \
				newcap = n;                                   \
								              \
			void *p = name##_realloc(big? foo->arr : NULL,        \
					newcap*elsz);                         \
			if (p) {                                              \
				if (!big)                                     \
					memcpy(p, foo->sbo, foo->len*elsz),   \
					foo->big = true;                      \
				foo->arr = p;                                 \
				foo->cap = newcap;                            \
			} else                                                \
				return false;                                 \
		}                                                             \
		return true;                                                  \
	} else                                                                \
		return false;                                                 \
}                                                                             \
								              \
scope bool name##_insert(name *dst, size_t i,                                 \
			 const name##_eltype *restrict src, size_t n)         \
{                                                                             \
	enum { elsz = sizeof(name##_eltype) };                                \
                                                                              \
	if (!n)                                                               \
		return true;                                                  \
                                                                              \
	register size_t len;                                                  \
	if (dst && name##_maxcap-n >= (len = dst->len) && i <= len            \
				     && name##_reserve(dst, len+n)) {         \
		                                                              \
		register name##_eltype *arr = dst->big? dst->arr : dst->sbo;  \
		                                                              \
		/* move elements at i to i+n to preserve them */              \
		memmove(arr+i+n, arr+i, (len-i)*elsz);                        \
		/* if src == NULL, caller will emplace, don't copy */         \
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
		       && isrc < len && name##_reserve(foo, len+n)) {         \
								              \
		register name##_eltype *arr = foo->big? foo->arr : foo->sbo;  \
		                                                              \
		memmove(arr+idst+n, arr+idst, (len-idst)*elsz);               \
		/* If idst < isrc, isrc has moved n ahead.
		 * If idst == isrc, don't copy.
		 */                                                           \
		memmove(arr+idst, arr+isrc + (idst < isrc)*n,                 \
					n*elsz * (idst != isrc));             \
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
	register size_t len;                                                  \
	if (dst && name##_maxcap-i >= n && i+n <= (len = dst->len)) {         \
		                                                              \
		register name##_eltype *arr = dst->big? dst->arr : dst->sbo;  \
		/* Shift elements at index > i one step back */               \
		memmove(arr+i, arr+i+n, (len-i-n)*elsz);                      \
		                                                              \
		dst->len = len-n;                                             \
		return true;                                                  \
	} else                                                                \
		return false;                                                 \
}                                                                             \
								              \
scope void name##_shrink_to_fit(name *foo)                                    \
{                                                                             \
	enum { elsz = sizeof(name##_eltype) };                                \
                                                                              \
	/* Avoid realloc() call if not needed */                              \
	register size_t len;                                                  \
	if (foo && foo->big && foo->cap > (len = foo->len)) {                 \
                                                                              \
		if (len <= name##_sbocap) { /* Move to short buffer */        \
			void *p = foo->arr;                                   \
			memcpy(foo->sbo, p, len*elsz);                        \
			name##_free(p), foo->big = false;                     \
		} else {                                                      \
			void *p = name##_realloc(foo->arr, len*elsz);         \
			if (p)                                                \
				foo->arr = p, foo->cap = len;                 \
	       }                                                              \
	}                                                                     \
}                                                                             \

#define SBOMGA_IMPL(name, reallocfn, freefn, sbocap, ...)                     \
SBOMGA_DECL(static inline, name, reallocfn, freefn, sbocap, __VA_ARGS__)      \
SBOMGA_DEF(static inline, name)

#endif
#endif
