#ifndef SBOVEC_H
#define SBOVEC_H

#include <stdbool.h> /* bool, true, false */
#include <stddef.h>  /* size_t            */

/* Declares types and functions */
#define SBOVEC_DECL(scope, name, sbocap, ...)                                 \
	typedef __VA_ARGS__ name##_eltype;                                    \
	enum { name##_sbocap =                                                \
	(sbocap)? (sbocap):1 > sizeof( struct{name##_eltype *p;size_t q;})    \
	/sizeof(name##_eltype) ?                                              \
	(sbocap)? (sbocap):1 : sizeof( struct{name##_eltype *p;size_t q;})    \
	/sizeof(name##_eltype) };                                             \
	                                                                      \
	typedef struct name {                                                 \
		union {                                                       \
			struct { name##_eltype *arr; size_t cap; };           \
			name##_eltype sbo[name##_sbocap];                     \
		};                                                            \
		void *(*realloc)(void *,size_t), (*free)(void *);             \
		size_t len;                                                   \
		bool big;                                                     \
	} name;                                                               \
	                                                                      \
	scope name##_eltype *name##_arr(const name *);                        \
	scope size_t name##_cap(const name *foo);                             \
	                                                                      \
	scope name name##_create(size_t n, void *(*realloc)(void *, size_t),  \
			                               void (*free)(void *)); \
	scope void name##_destroy(name *);                                    \
	scope bool name##_reserve(name *, size_t n);                          \
	scope bool name##_insert(name *, size_t i,                            \
				const name##_eltype *restrict src, size_t n); \
	scope bool name##_selfinsert(name *foo, size_t idst, size_t isrc,     \
	                                                           size_t n); \
	scope bool name##_remove(name *, size_t i, size_t n);		      \
	scope void name##_shrink_to_fit(name *);                              \

/* Define SBOVEC_NOIMPL to strip implementation code */
#ifndef SBOVEC_NOIMPL

#include <string.h>  /* memcpy(), memmove() */

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

/* Expands function definitons for previously MGA_DECL()'d name */
#define SBOVEC_DEF(scope, name)                                               \
	scope name##_eltype *name##_arr(const name *foo)                      \
	{                                                                     \
		enum { sbocap = name##_sbocap };                              \
		if (!foo)                                                     \
			return NULL;                                          \
		else if (foo->big)                                            \
			return foo->arr;                                      \
		else                                                          \
			return (name##_eltype *)foo->sbo; /* Const cast */    \
	}                                                                     \
                                                                              \
	scope size_t name##_cap(const name *foo)                              \
	{                                                                     \
		enum { sbocap = name##_sbocap };                              \
		if (!foo)                                                     \
			return 0;                                             \
		else if (foo->big)                                            \
			return foo->cap;                                      \
		else                                                          \
			return sbocap;                                        \
	}                                                                     \
	                                                                      \
	scope name name##_create(size_t n, void *(*realloc)(void *, size_t),  \
			                                void (*free)(void *)) \
	{                                                                     \
		enum {                                                        \
			elsz = sizeof(name##_eltype),                         \
			sbocap = name##_sbocap                                \
		};                                                            \
		name res = {                                                  \
			.realloc = realloc,                                   \
			.free = free,                                         \
			.big = n > sbocap                                     \
		};                                                            \
		if (res.big && n < SIZE_MAX/elsz       	                      \
				   && (res.arr = realloc(NULL, n*elsz)))      \
			res.cap = n;                                          \
		return res;                                                   \
	}                                                                     \
	                                                                      \
	scope void name##_destroy(name *foo)                                  \
	{                                                                     \
		enum { sbocap = name##_sbocap };                              \
		if (foo) {                                                    \
			if (foo->big)                                         \
				foo->free(foo->arr), foo->big = false;        \
			foo->len = 0;                                         \
		}                                                             \
	}                                                                     \
									      \
	scope bool name##_reserve(name *foo, size_t n)                        \
	{                                                                     \
		enum { elsz = sizeof(name##_eltype) };                        \
		if (foo && n < SIZE_MAX/elsz) {                               \
			size_t cap = name##_cap(foo);                         \
			if (cap < n) {                                        \
				size_t newcap = cap+cap/2;                    \
				if (newcap < n || SIZE_MAX/elsz < newcap)     \
					newcap = n;                           \
				                                              \
				void *p = foo->realloc(foo->big?              \
						foo->arr : NULL,newcap*elsz); \
				if (p) {                                      \
					if (!foo->big)                        \
						memcpy(p, foo->sbo, foo->len),\
						foo->big = true;              \
					foo->arr = p;                         \
					foo->cap = newcap;                    \
				} else                                        \
					return false;                         \
			}                                                     \
			return true;                                          \
		} else                                                        \
			return false;                                         \
	}                                                                     \
	                                                                      \
	scope bool name##_insert(name *dst, size_t i,                         \
	                         const name##_eltype *restrict src, size_t n) \
	{                                                                     \
		enum {elsz = sizeof(name##_eltype)};                          \
		if (n == 0)                                                   \
			return true;                                          \
		size_t len;                                                   \
		if (dst && SIZE_MAX-n >= (len = dst->len) && i <= len         \
				             && name##_reserve(dst, len+n)) { \
			name##_eltype *arr = name##_arr(dst);                 \
			/* move elements at i to i+n to preserve them */      \
			memmove(arr+i+n, arr+i, (len-i)*elsz);                \
			/* if src == NULL, caller will emplace, don't copy */ \
			if (src)                                              \
				memcpy(arr+i, src, n*elsz);                   \
			dst->len = len+n;                                     \
			return true;                                          \
		} else                                                        \
			return false;                                         \
	}                                                                     \
	                                                                      \
	scope bool name##_selfinsert(name *foo, size_t idst, size_t isrc,     \
			                                            size_t n) \
	{                                                                     \
		enum {elsz = sizeof(name##_eltype)};                          \
		if (n == 0)                                                   \
			return true;                                          \
		size_t len;                                                   \
		if (foo && SIZE_MAX-n >= (len = foo->len) && idst <= len      \
			       && isrc < len && name##_reserve(foo, len+n)) { \
			                                                      \
			name##_eltype *arr = name##_arr(foo);                 \
			memmove(arr+idst+n, arr+idst, (len-idst)*elsz);       \
			/* If idst < isrc, isrc has moved n ahead.
			 * If idst == isrc, don't copy.
			 */                                                   \
			memmove(arr+idst, arr+isrc+(idst < isrc)*n,           \
						n*elsz*(idst != isrc));       \
			foo->len = len+n;                                     \
			return true;                                          \
		} else                                                        \
			return false;                                         \
	}                                                                     \
									      \
	scope bool name##_remove(name *dst, size_t i, size_t n)               \
	{                                                                     \
		enum {elsz = sizeof(name##_eltype)};                          \
		size_t len;                                                   \
		if (dst && SIZE_MAX-i >= n && i+n <= (len = dst->len)) {      \
			name##_eltype *arr = name##_arr(dst);                 \
			/* Shift elements at index > i one step back */       \
			memmove(arr+i, arr+i+n, (len-i-n)*elsz);              \
			dst->len = len-n;                                     \
			return true;                                          \
		} else                                                        \
			return false;                                         \
	}                                                                     \
									      \
	scope void name##_shrink_to_fit(name *foo)                            \
	{                                                                     \
		enum {                                                        \
			elsz = sizeof(name##_eltype),                         \
			sbocap = name##_sbocap                                \
		};                                                            \
		/* Avoid realloc() call if not needed */                      \
		size_t len;                                                   \
		if (foo && foo->big && name##_cap(foo) > (len = foo->len)) {  \
			if (len <= sbocap) {                                  \
				void *p = foo->arr;                           \
				memcpy(foo->sbo, p, len);                     \
				foo->free(p), foo->big = false;               \
			} else {                                              \
				void *p = foo->realloc(foo->arr, len*elsz);   \
				if (p)                                        \
					foo->arr = p, foo->cap  = len;        \
		       }                                                      \
		}                                                             \
	}                                                                     \

#define SBOVEC_IMPL(name, sbocap, ...)                                        \
	SBOVEC_DECL(static inline, name, sbocap, __VA_ARGS__)                 \
	SBOVEC_DEF(static inline, name)

#endif
#endif
