#include <stdlib.h> /* malloc(), realloc(), free(), size_t, NULL */
#include <string.h> /* memcpy(), memmove()                       */

#include "vpa.h"

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

vpa vpa_create(size_t n, size_t elsz)
{
	vpa res = {.len = 0, .sz = 0, .elsz = elsz, .arr = NULL};

	/* 0-size, overflow and malloc check */
	if(n && elsz && SIZE_MAX/elsz >= n && (res.arr = malloc(n*elsz)) )
		res.sz = n*elsz;

	return res;
}

void vpa_destroy(vpa *foo)
{
	if(foo) {
		free(foo->arr);
		foo->arr = NULL;
		foo->len = foo->sz = 0;
	}
}

bool vpa_reserve(vpa *foo, size_t n)
{
	/* NULL and overflow check */
	if(foo && SIZE_MAX/foo->elsz >= n) {
		if(foo->sz < n*foo->elsz) {
			const size_t sz = foo->sz,
			newsz = sz+sz/2 > n*foo->elsz : sz+sz/2 : n*foo->sz;
			void *new = realloc(foo->arr, newsz);
			if(new) {
				foo->arr = new;
				foo->sz = newsz;
			} else
				return false;
		}
		return true;
	} else
		return false;
}

bool vpa_insert(vpa *dst, size_t i, const void *restrict src, size_t n)
{
	if(!n)
		return true;
	/* NULL, overflow, out of bounds and space check */
	else if(dst && src && SIZE_MAX-n >= dst->len && i <= dst->len 
			&& vpa_reserve(dst, dst->len+n)) {

		char *at_i = (char *)dst->arr + i*dst->elsz;

		/* move elements at i to i+n to preserve them */
		memcpy(at_i + n*dst->elsz , at_i, (dst->len-i) * dst->elsz);
		memcpy(at_i, src, n*dst->elsz);

		dst->len += n;
		return true;
	} else
		return false;
}

bool vpa_selfinsert(vpa *foo, size_t idst, size_t isrc, size_t n)
{
	if(!n)
		return true;
	/* NULL, overflow, out-of-bounds and space checks */
	else if(foo && SIZE_MAX-n >= foo->len && idst <= foo->len
	&& isrc < foo->len && vpa_reserve(foo, foo->len+n)) {

		/* Get ptrs to .arr[isrc] & .arr[idst] */
		char *at_idst = (char *)foo->arr + idst*foo->elsz,
		     *at_isrc = (char *)foo->arr + isrc*foo->elsz;

		memmove(at_idst + n*foo->elsz, at_idst, 
				(foo->len-idst) * foo->elsz);
		/* If idst < isrc, isrc has moved n ahead.
		 * If idst == isrc, don't copy.
		 */
		memmove(at_idst, at_isrc + (idst < isrc) * n*foo->elsz,
					n*foo->elsz * (idst != isrc));
		foo->len += n;
		return true;
	} else
		return false;
}

bool vpa_remove(vpa *dst, size_t i, size_t n)
{
	/* NULL ptr, overflow or out of bounds check */
	if(dst && SIZE_MAX-i >= n && i+n < dst->len) {
		char *at_i = (char *)dst->arr + i*dst->elsz;

		/* Shift elements at index > i one step back */
		memmove(at_i, at_i + n*dst->elsz, (dst->len-i-n) * dst->elsz);
		dst->len -= n;
		return true;
	} else
		return false;
}

void vpa_shrink_to_fit(vpa *foo)
{
	/* Avoid realloc() call if not needed */
	if(foo && foo->sz > foo->len * foo->elsz) {
		void *p = realloc(foo->arr, foo->len * foo->elsz);
		if(p) {
			foo->arr = p;
			foo->sz  = foo->len * foo->elsz;
		}
	}
}
