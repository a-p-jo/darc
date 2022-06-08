#include <stddef.h>  /* size_t, NULL, max_align_t   */
#include <stdbool.h> /* bool, true, false           */
#include <string.h>  /* memcpy(), memmove()         */

/* Edit the below to use a custom allocator */
#include <stdlib.h>
static void *(*const fpa_realloc)(void *, size_t) = realloc;
static void  (*const fpa_free)   (void *)         = free;

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

/* Metadata header preceeding caller's array 
 * Aligned such that hdr * casts to any T * .
 */
typedef struct hdr {
	size_t len, cap, elsz;

	#if __STDC_VERSION__ < 201112L
	union {
		long double f; long long i;
		void *p; void (*fp)(void);
	} _align[];
	#else
	max_align_t _align[];
	#endif
} hdr;
enum { HDRSZ = sizeof(hdr) };

/* Returns maximum possible capacity for elements of given size */
static inline size_t maxcap(size_t elsz) { return (SIZE_MAX-HDRSZ)/elsz; }

size_t fpa_maxcap(const hdr *h)
{
	if (h)
		return maxcap(h[-1].elsz);
	else
		return 0;
}

size_t *fpa_len(const hdr *h)
{
	/* Const cast */
	return h ? (size_t *)&h[-1].len : NULL;
}

void *fpa_cast(hdr *h, size_t elsz)
{
	if (h && elsz) {
		h[-1].elsz = elsz;
		return h;
	} else
		return NULL;
}

void *fpa_create(size_t n, size_t elsz)
{
	if (elsz && n <= maxcap(elsz)) {
		hdr *new = fpa_realloc(NULL, HDRSZ + n*elsz);
		if (new) {
			*new = (hdr) {.len = 0, .cap = n, .elsz = elsz};
			return new+1; /* Return array region */
		}
	}
	return NULL;
}

/* Return pointer to metadata header given pointer to caller's array */
static inline hdr *hdrp(hdr **fpa_ptr) { return (*fpa_ptr)-1; }

void fpa_destroy(hdr **h)
{
	if (h && *h)
		fpa_free(hdrp(h)), *h = NULL;
}

bool fpa_reserve(hdr **foo, size_t n)
{
	register hdr h; /* header is saved to h, avoids repeated indirection */

	if ( foo && *foo && n <= maxcap((h = *hdrp(foo)).elsz) ) {
		if (h.cap < n) {
			size_t newcap = h.cap+h.cap/2;
			if (newcap < n || newcap > maxcap(h.elsz))
				newcap = n;
			
			hdr *new = fpa_realloc(hdrp(foo), HDRSZ + newcap*h.elsz);
			if(new) {
				new->cap = newcap;
				*foo = new+1; /* Update caller's data pointer */
				return true;
			}
		} else
			return true;
	}
	return false;
}

typedef unsigned char byte;

bool fpa_insert(hdr **dst, size_t i, const void *restrict src, size_t n)
{
	if (n == 0)
		return true;

	register hdr h;
	if (dst && *dst && maxcap((h = *hdrp(dst)).elsz) - n >= h.len
			&& i <= h.len && fpa_reserve(dst, h.len+n)) {

		byte *at_i = (byte *)(*dst) + i*h.elsz;

		/* move elements at i to i+n to preserve them */
		memmove(at_i + n*h.elsz, at_i, (h.len-i)*h.elsz);
		/* if src == NULL, caller will emplace, don't copy */
		if (src)
			memcpy(at_i, src, n*h.elsz);

		hdrp(dst)->len = h.len+n;
		return true;
	} else
		return false;
}

bool fpa_selfinsert(hdr **foo, size_t idst, size_t isrc, size_t n)
{
	if (n == 0)
		return true;

	register hdr h;
	if (foo && *foo && maxcap((h = *hdrp(foo)).elsz) - n >= h.len
			&& idst <= h.len && isrc < h.len && fpa_reserve(foo, h.len+n)) {

		byte *at_idst = (byte *)(*foo) + idst*h.elsz;
		byte *at_isrc = (byte *)(*foo) + isrc*h.elsz;

		memmove(at_idst + n*h.elsz, at_idst, (h.len-idst) * h.elsz);
		/* If idst < isrc, isrc has moved n ahead.
		 * If idst == isrc, don't copy.
		 */
		memmove(at_idst, at_isrc + (idst < isrc) * n*h.elsz,
				n*h.elsz * (idst != isrc));

		hdrp(foo)->len = h.len+n;
		return true;
	} else
		return false;
}

/* Since removal does not relocate data and invalidate refrences,
 * it does not need a double-pointer.
 */
bool fpa_remove(hdr *dst, size_t i, size_t n)
{
	register hdr h;
	if (dst && maxcap((h = dst[-1]).elsz) - i >= n && i+n <= h.len) {
		
		byte *at_i = (byte *)dst + i*h.elsz;
		/* Shift elements at index > i one step back */
		memmove(at_i, at_i + n*h.elsz, (h.len-i-n)*h.elsz);

		dst[-1].len = h.len-n;
		return true;
	} else
		return false;
}

void fpa_shrink_to_fit(hdr **foo)
{
	register hdr h;
	/* Avoid realloc() call if not needed */
	if (foo && *foo && (h = *hdrp(foo)).cap > h.len) {
		struct hdr *new = fpa_realloc(hdrp(foo), HDRSZ + h.len*h.elsz);
		if (new)
			new->cap = h.len, *foo = new+1;
	}
}
