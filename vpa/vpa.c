#include <string.h> /* memcpy(), memmove() */
#include "vpa.h"

/* Edit the below to use a custom allocator */
#include <stdlib.h>
static void *(*const vpa_realloc)(void *, size_t) = realloc;
static void  (*const vpa_free)   (void *)         = free;

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

static inline size_t maxcap(size_t elsz) { return SIZE_MAX/elsz; }

size_t vpa_maxcap(const vpa *foo) { return foo->elsz ? maxcap(foo->elsz) : 0; }

vpa vpa_create(size_t n, size_t elsz)
{
	vpa res = {.elsz = elsz};
	/* We use vpa_maxcap() here as it checks that elsz != 0 for us */
	if (n && vpa_maxcap(&res) >= n && (res.arr = vpa_realloc(NULL, n*elsz)))
		res.cap = n;
	return res;
}

void vpa_destroy(vpa *foo)
{
	if (foo) {
		vpa_free(foo->arr), foo->arr = NULL;
		foo->len = foo->cap = 0;
	}
}

bool vpa_reserve(vpa *foo, size_t n)
{
	/* Save to locals, avoid repeated indirection */
	register size_t maxcap = vpa_maxcap(foo);

	if (foo && n <= maxcap) {
		register vpa v = *foo;

		if (v.cap < n) {
			size_t newcap = v.cap+v.cap/2; /* Try growing 1.5x */
			/* Or grow to n elements if its bigger or overflow */
			if (newcap < n || newcap > maxcap)
				newcap = n;

			void *p = vpa_realloc(v.arr, newcap*v.elsz);
			if (p)
				foo->arr = p, foo->cap = newcap;
			else
				return false;
		}
		return true;
	} else
		return false;
}

typedef unsigned char byte;

bool vpa_insert(vpa *dst, size_t i, const void *restrict src, size_t n)
{
	if (n == 0)
		return true;

	register size_t len, elsz;
	if (dst && (elsz = dst->elsz) && maxcap(elsz)-n >= (len = dst->len)
			&& i <= len && vpa_reserve(dst, len+n)) {
		
		byte *at_i = (byte *)dst->arr + i*elsz;
		/* move elements at i to i+n to preserve them */
		memmove(at_i + n*elsz, at_i, (len-i)*elsz);
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
	
	register size_t len, elsz;
	if (foo && (elsz = foo->elsz) && maxcap(elsz)-n >= (len = foo->len)
		&& idst <= len && isrc < len && vpa_reserve(foo, len+n)) {

		byte *at_idst = (byte *)foo->arr + idst*elsz;
		byte *at_isrc = (byte *)foo->arr + isrc*elsz;

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
	register vpa v;
	if (dst && (v = *dst).elsz
			&& maxcap(v.elsz)-i >= n && i+n <= v.len) {

		byte *at_i = (byte *)v.arr + i*v.elsz;
		/* Shift elements at index > i one step back */
		memmove(at_i, at_i + n*v.elsz, (v.len-i-n)*v.elsz);

		dst->len = v.len-n;
		return true;
	} else
		return false;
}

void vpa_shrink_to_fit(vpa *foo)
{
	/* Avoid realloc() call if not needed */
	register vpa v;
	if(foo && (v = *foo).elsz && v.cap > v.len) {
		void *p = vpa_realloc(v.arr, v.len*v.elsz);
		if (p)
			foo->arr = p, foo->cap = v.len;
	}
}
