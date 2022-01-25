#ifndef MGA_H
#define MGA_H

#include <stdbool.h> /* bool, true, false */
#include <stddef.h> /* size_t             */

/* Declares typedefs and function prototypes, for headers. */

#define MGA_DECL(name, ...)                                                   \
	typedef __VA_ARGS__ name##_eltype;                                    \
	typedef struct name { size_t len, cap; name##_eltype *arr; } name;    \
	                                                                      \
	name name##_create(size_t n);                                         \
	void name##_destroy(name *);                                          \
	bool name##_reserve(name *, size_t n);                                \
	bool name##_insert(name *, size_t i,                                  \
				const name##_eltype *restrict src, size_t n); \
	bool name##_selfinsert(name *foo, size_t idst, size_t isrc, size_t n);\
	bool name##_remove(name *, size_t i, size_t n);		              \
	void name##_shrink_to_fit(name *);                                    \

/* Define MGA_NOIMPL to strip implementation code */

#ifndef MGA_NOIMPL

#include <stdlib.h>  /* malloc(), realloc(), free(), size_t, NULL */
#include <string.h>  /* memcpy, memmove()                         */

/* stdlib.h may not provide SIZE_MAX */

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

/* Expands function definitons for previously MGA_DECL()'d <name> */

#define MGA_DEF(name)                                                         \
	                                                                      \
	name name##_create(const size_t n)                                    \
	{                                                                     \
		enum { elsz = sizeof(name##_eltype) };                        \
		                                                              \
		name res = {0};                                               \
		if(n > 0 && n < SIZE_MAX/elsz && (res.arr = malloc(n*elsz)) ) \
			res.cap = n;                                          \
		return res;                                                   \
	}                                                                     \
	                                                                      \
	void name##_destroy(name *foo)                                        \
	{                                                                     \
		if(foo != NULL) {                                             \
			free(foo->arr);                                       \
			foo->arr = NULL;                                      \
			foo->len = foo->cap = 0;		              \
		}                                                             \
	}                                                                     \
									      \
	bool name##_reserve(name *foo, const size_t n)                        \
	{                                                                     \
		enum { elsz = sizeof(name##_eltype) };                        \
									      \
		if(foo != NULL && n < SIZE_MAX/elsz) {                        \
			const size_t cap = foo->cap;                          \
			if(cap < n) {                                         \
				size_t newcap = cap+cap/2;                    \
				if(newcap < n || SIZE_MAX/elsz < newcap)      \
					newcap = n;                           \
				                                              \
				void *new = realloc(foo->arr, newcap*elsz);   \
				if(new != NULL) {                             \
					foo->arr = new;                       \
					foo->cap = newcap;                    \
				} else                                        \
					return false;                         \
			}                                                     \
			return true;                                          \
		} else                                                        \
			return false;                                         \
	}                                                                     \
	                                                                      \
	bool name##_insert(name *dst, const size_t i,                         \
			const name##_eltype *restrict src, const size_t n)    \
	{                                                                     \
		enum { elsz = sizeof(name##_eltype) };                        \
									      \
		if(n == 0)                                                    \
			return true;                                          \
		size_t len;                                                   \
		if(dst != NULL && SIZE_MAX-n >= (len = dst->len) && i <= len  \
				&& name##_reserve(dst, len+n)) {              \
			                                                      \
			/* move elements at i to i+n to preserve them */      \
			memmove(dst->arr+i+n, dst->arr+i, (len-i)*elsz);      \
			/* if src == NULL, caller will emplace, don't copy */ \
			if(src != NULL)                                       \
				memcpy(dst->arr+i, src, n*elsz);              \
			                                                      \
			dst->len = len+n;                                     \
			return true;                                          \
		} else                                                        \
			return false;                                         \
	}                                                                     \
	                                                                      \
	bool name##_selfinsert(name *foo, const size_t idst,                  \
			const size_t isrc, const size_t n)                    \
	{                                                                     \
		enum { elsz = sizeof(name##_eltype) };                        \
		                                                              \
		if(n == 0)                                                    \
			return true;                                          \
		size_t len;                                                   \
		if(foo != NULL && SIZE_MAX-n >= (len = foo->len)              \
				&& idst <= len && isrc < len                  \
				&& name##_reserve(foo, len+n)) {              \
			                                                      \
			memmove(foo->arr+idst+n, foo->arr+idst,               \
					(len-idst) * elsz);                   \
			/* If idst < isrc, isrc has moved n ahead.
			 * If idst == isrc, don't copy.
			 */                                                   \
			memmove(foo->arr+idst, foo->arr+isrc+(idst < isrc)*n, \
						n*elsz*(idst != isrc));       \
			foo->len = len+n;                                     \
			return true;                                          \
		} else                                                        \
			return false;                                         \
	}                                                                     \
									      \
	bool name##_remove(name *dst, const size_t i, const size_t n)         \
	{                                                                     \
		enum { elsz = sizeof(name##_eltype) };                        \
									      \
		size_t len;                                                   \
		if(dst != NULL && SIZE_MAX-i >= n                             \
				&& i+n < (len = dst->len)) {                  \
			                                                      \
			/* Shift elements at index > i one step back */       \
			memmove(dst->arr+i, dst->arr+i+n, (len-i-n) * elsz);  \
			dst->len = len-n;                                     \
			return true;                                          \
		} else                                                        \
			return false;                                         \
	}                                                                     \
									      \
	void name##_shrink_to_fit(name *foo)                                  \
	{                                                                     \
		enum { elsz = sizeof(name##_eltype) };                        \
									      \
		/* Avoid realloc() call if not needed */                      \
		size_t len;                                                   \
		if(foo != NULL && foo->cap > (len = foo->len)) {              \
			void *p = realloc(foo->arr, len*elsz);                \
			if(p != NULL) {                                       \
				foo->arr = p;                                 \
				foo->cap  = len;                              \
			}                                                     \
		}                                                             \
	}                                                                     \

#define MGA_IMPL(name, ...)                                                   \
	MGA_DECL(name, __VA_ARGS__)                                           \
	MGA_DEF(name) 

#endif

#endif
