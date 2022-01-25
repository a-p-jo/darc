#include <stdlib.h> /* malloc(), realloc(), free(), size_t, NULL */
#include <string.h> /* memcpy(), memmove()                       */

#include "vpa.h"

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

vpa vpa_create(const size_t n, const size_t elsz)
{
	vpa res = {.elsz = elsz};

	if(n > 0 && elsz > 0 && SIZE_MAX/elsz >= n 
			&& (res.arr = malloc(n*elsz)) != NULL)
		res.cap = n;

	return res;
}

void vpa_destroy(vpa *foo)
{
	if(foo != NULL) {
		free(foo->arr);
		foo->arr = NULL;
		foo->len = foo->cap = 0;
	}
}

bool vpa_reserve(vpa *foo, const size_t n)
{
	size_t elsz;
	if(foo != NULL && (elsz = foo->elsz) != 0 && SIZE_MAX/elsz >= n) {
		const size_t cap = foo->cap;
		if(cap < n) {
			size_t newcap = cap+cap/2;
			if(newcap < n || SIZE_MAX/elsz < newcap)
				newcap = n;
			
			void *new = realloc(foo->arr, newcap*elsz);
			if(new != NULL) {
				foo->arr = new;
				foo->cap = newcap;
			} else
				return false;
		}
		return true;
	} else
		return false;
}

bool vpa_insert(vpa *dst, const size_t i, const void *restrict src, const size_t n)
{
	if(n == 0)
		return true;
	size_t elsz, len;
	if(dst != NULL && (elsz = dst->elsz) != 0 
			&& SIZE_MAX-n >= (len = dst->len) && i <= len 
			&& vpa_reserve(dst, len+n)) {
		unsigned char *at_i = (unsigned char *)dst->arr + i*elsz;

		/* move elements at i to i+n to preserve them */
		memmove(at_i + n*elsz, at_i, (len-i) * dst->elsz);
		/* if src == NULL, caller will emplace, don't copy */
		if(src != NULL)
			memcpy(at_i, src, n*elsz);
		dst->len = len+n;
		return true;
	} else
		return false;
}

bool vpa_selfinsert(vpa *foo, const size_t idst, const size_t isrc, const size_t n)
{
	if(n == 0)
		return true;
	size_t elsz, len;
	if(foo != NULL && (elsz = foo->elsz) != 0 && SIZE_MAX-n >= (len = foo->len) 
			&& idst <= len && isrc < len 
			&& vpa_reserve(foo, len+n)) {

		unsigned char *at_idst = (unsigned char *)foo->arr + idst*elsz,
			      *at_isrc = (unsigned char *)foo->arr + isrc*elsz;

		memmove(at_idst + n*elsz, at_idst, (len-idst) * elsz);
		/* If idst < isrc, isrc has moved n ahead.
		 * If idst == isrc, don't copy.
		 */
		memmove(at_idst, at_isrc + (idst < isrc) * n*elsz,
				n*elsz * (idst != isrc));
		foo->len = len+n;
		return true;
	} else
		return false;
}

bool vpa_remove(vpa *dst, const size_t i, const size_t n)
{
	/* NULL ptr, overflow or out of bounds check */
	size_t elsz, len;
	if(dst != NULL && SIZE_MAX-i >= n 
			&& (elsz = dst->elsz) != 0 && i+n < (len = dst->len)) {
		char *at_i = (char *)dst->arr + i*elsz;

		/* Shift elements at index > i one step back */
		memmove(at_i, at_i + n*elsz, (len-i-n) * elsz);
		dst->len = len-n;
		return true;
	} else
		return false;
}

void vpa_shrink_to_fit(vpa *foo)
{
	/* Avoid realloc() call if not needed */
	size_t elsz, len;
	if(foo != NULL && (elsz = foo->elsz) != 0 && foo->cap > (len = foo->len)) {
		void *p = realloc(foo->arr, len*elsz);
		if(p != NULL) {
			foo->arr = p;
			foo->cap  = len;
		}
	}
}
