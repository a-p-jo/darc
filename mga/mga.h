#ifndef MGA_H
#define MGA_H

#include <stdbool.h> /* bool, true, false */
#include <stddef.h> /* size_t             */

/* Declares typedefs and function prototypes, for headers. */

#define MGA_DECL(name, ...)                                                   \
	typedef __VA_ARGS__ name##_eltype;                                    \
	typedef struct name { size_t len, sz; name##_eltype *arr; } name;     \
	                                                                      \
	void name##_free(name *);                                             \
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

/* Expands function definitons for previously DYARR_DECL()'d <name> */

#define MGA_DEF(name)                                                         \
	/* free()'s .arr & resets all feilds to 0 */                          \
	void name##_free(name *foo)                                           \
	{                                                                     \
		if(foo) {                                                     \
			free(foo->arr);                                       \
			foo->arr = NULL;                                      \
			foo->len = foo->sz  = 0;		              \
		}                                                             \
	}                                                                     \
									      \
	/* Ensures capacity of at least n elements in .arr */                 \
	bool name##_reserve(name *foo, size_t n)                              \
	{                                                                     \
		enum {                                                        \
			elsz = sizeof(name##_eltype),                         \
			max_n = SIZE_MAX/elsz                                 \
		};                                                            \
									      \
		/* NULL ptr or overflow check */                              \
		if(foo && max_n >= n) {                                       \
			if(foo->sz < n*elsz) {                                \
				const size_t sz = foo->sz,                    \
				newsz = sz+sz/2 > n*elsz ? sz+sz/2 : n*elsz;  \
				void *new = realloc(foo->arr, newsz);         \
				if(new) {                                     \
					foo->arr = new;                       \
					foo->sz = newsz;                      \
				} else                                        \
					return false;                         \
			}                                                     \
			return true;                                          \
		} else                                                        \
			return false;                                         \
	}                                                                     \
	                                                                      \
	/* Inserts [src[i], src[i+n]) at .arr[i]. */                          \
	bool name##_insert(name *dst, size_t i,                               \
				const name##_eltype *restrict src, size_t n)  \
	{                                                                     \
		enum { elsz = sizeof(name##_eltype) };                        \
									      \
		if(!n)                                                        \
			return true;                                          \
		/* NULL ptr, overflow, out of bounds and space check */       \
		else if(dst && src && SIZE_MAX-n >= dst->len && i <= dst->len \
				&& name##_reserve(dst, dst->len+n)) {         \
			                                                      \
			/* move elements at i to i+n to preserve them */      \
			memcpy(dst->arr+i+n, dst->arr+i, (dst->len-i)*elsz);  \
			memcpy(dst->arr+i, src, n*elsz);                      \
			                                                      \
			dst->len += n;                                        \
			return true;                                          \
		} else                                                        \
			return false;                                         \
	}                                                                     \
	                                                                      \
	/* Inserts [.arr[isrc], .arr[isrc+n]) at .arr[idst] */                \
	bool name##_selfinsert(name *foo, size_t idst, size_t isrc, size_t n) \
	{                                                                     \
		enum { elsz = sizeof(name##_eltype) };                        \
		                                                              \
		if(!n)                                                        \
			return true;                                          \
		/* NULL, overflow, out-of-bounds and space checks */          \
		else if(foo && SIZE_MAX-n >= foo->len && idst <= foo->len     \
		&& isrc < foo->len && name##_reserve(foo, foo->len+n)) {      \
			                                                      \
			memmove(foo->arr+idst+n, foo->arr+idst,               \
					(foo->len-idst) * elsz);              \
			/* If idst < isrc, isrc has moved n ahead.
			 * If idst == isrc, don't copy.
			 */                                                   \
			memmove(foo->arr+idst, foo->arr+isrc+(idst < isrc)*n, \
						n*elsz*(idst != isrc));       \
			foo->len += n;                                        \
			return true;                                          \
		} else                                                        \
			return false;                                         \
	}                                                                     \
									      \
	/* Removes [.arr[i], .arr[i+n]) */                                    \
	bool name##_remove(name *dst, size_t i, size_t n)                     \
	{                                                                     \
		enum { elsz = sizeof(name##_eltype) };                        \
									      \
		/* NULL ptr, overflow or out of bounds check */               \
		if(dst && SIZE_MAX-i >= n && i+n < dst->len) {                \
			/* Shift elements at index > i one step back */       \
			memmove(dst->arr+i, dst->arr+i+n,                     \
				(dst->len-i-n) * elsz);                       \
			dst->len -= n;                                        \
			return true;                                          \
		} else                                                        \
			return false;                                         \
	}                                                                     \
									      \
	/* Reallocate to minimum requirement */                               \
	void name##_shrink_to_fit(name *foo)                                  \
	{                                                                     \
		enum { elsz = sizeof(name##_eltype) };                        \
									      \
		/* Avoid realloc() call if not needed */                      \
		if(foo && foo->sz > foo->len * elsz) {                        \
			void *p = realloc(foo->arr, foo->len * elsz);         \
			if(p) {                                               \
				foo->arr = p;                                 \
				foo->sz  = foo->len * elsz;                   \
			}                                                     \
		}                                                             \
	}                                                                     \

#define MGA_IMPL(name, ...)                                                   \
	MGA_DECL(name, __VA_ARGS__)                                           \
	MGA_DEF(name) 

#endif

#endif
