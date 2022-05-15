#include <string.h> /* memcpy(), memmove() */

#include "vpa.h"

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

vpa vpa_create(size_t n, size_t elsz,
	void *(*realloc)(void *, size_t), void (*free)(void *))
{
	vpa res = {.elsz = elsz, .realloc = realloc, .free = free};
	if (n && elsz && SIZE_MAX/elsz >= n 
			&& (res.arr = realloc(NULL, n*elsz)))
		res.cap = n;
	return res;
}

void vpa_destroy(vpa *foo)
{
	if (foo) {
		foo->free(foo->arr), foo->arr = NULL;
		foo->len = foo->cap = 0;
	}
}

bool vpa_reserve(vpa *foo, size_t n)
{
	size_t elsz;
	if (foo && (elsz = foo->elsz) && SIZE_MAX/elsz >= n) {
		size_t cap = foo->cap;
		if (cap < n) {
			size_t newcap = cap+cap/2;
			if (newcap < n || SIZE_MAX/elsz < newcap)
				newcap = n;

			void *p = foo->realloc(foo->arr, newcap*elsz);
			if (p)
				foo->arr = p, foo->cap = newcap;
			else
				return false;
		}
		return true;
	} else
		return false;
}

bool vpa_insert(vpa *dst, size_t i, const void *restrict src, size_t n)
{
	if (n == 0)
		return true;
	size_t elsz, len;
	if (dst && (elsz = dst->elsz) && SIZE_MAX-n >= (len = dst->len)
			&& i <= len && vpa_reserve(dst, len+n)) {
		unsigned char *at_i = (unsigned char *)dst->arr + i*elsz;

		/* move elements at i to i+n to preserve them */
		memmove(at_i + n*elsz, at_i, (len-i)*dst->elsz);
		/* if src == NULL, caller will emplace, don't copy */
		if(src)
			memcpy(at_i, src, n*elsz);
		dst->len = len+n;
		return true;
	} else
		return false;
}

bool vpa_selfinsert(vpa *foo, size_t idst, size_t isrc, size_t n)
{
	if (n == 0)
		return true;
	size_t elsz, len;
	if (foo && (elsz = foo->elsz) && SIZE_MAX-n >= (len = foo->len)
		&& idst <= len && isrc < len && vpa_reserve(foo, len+n)) {

		unsigned char *at_idst = (unsigned char *)foo->arr + idst*elsz,
			      *at_isrc = (unsigned char *)foo->arr + isrc*elsz;

		memmove(at_idst + n*elsz, at_idst, (len-idst)*elsz);
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

bool vpa_remove(vpa *dst, size_t i, size_t n)
{
	/* NULL ptr, overflow or out of bounds check */
	size_t elsz, len;
	if (dst && SIZE_MAX-i >= n && (elsz = dst->elsz)
			&& i+n < (len = dst->len)) {
		unsigned char *at_i = (unsigned char *)dst->arr + i*elsz;

		/* Shift elements at index > i one step back */
		memmove(at_i, at_i + n*elsz, (len-i-n)*elsz);
		dst->len = len-n;
		return true;
	} else
		return false;
}

void vpa_shrink_to_fit(vpa *foo)
{
	/* Avoid realloc() call if not needed */
	size_t elsz, len;
	if(foo && (elsz = foo->elsz) && foo->cap > (len = foo->len)) {
		void *p = foo->realloc(foo->arr, len*elsz);
		if (p)
			foo->arr = p, foo->cap  = len;
	}
}
