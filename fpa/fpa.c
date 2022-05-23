#include <stddef.h>  /* size_t, NULL, max_align_t   */
#include <stdbool.h> /* bool, true, false           */
#include <string.h>  /* memcpy(), memmove()         */

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

/* Bookeeping header preceeding caller's array */
typedef struct hdr {
	size_t len, cap, elsz;
	void *(*realloc)(void *, size_t), (*free)(void *);

	/* Align most restrictively so struct hdr * casts to any <type> * */
	#if __STDC_VERSION__ < 201112L
	union {
		long double f; long long i;
		void *p; void (*fp)(void);
	} _align[];
	#else
	max_align_t _align[];
	#endif
} hdr;

enum {HDRSZ = sizeof(hdr)};

void *fpa_create(size_t n, size_t elsz,
	void *(*realloc)(void *, size_t), void (*free)(void *))
{
	if (elsz && SIZE_MAX/elsz >= n && SIZE_MAX-HDRSZ >= n*elsz) {
		hdr *new = realloc(NULL, HDRSZ + n*elsz);
		if (new) {
			new->len = 0, new->cap = n;
			new->elsz = elsz;
			new->realloc = realloc, new->free = free;
			/* Return array region, ahead of header */
			return new+1;
		}
	}
	return NULL;
}

/* Access header feilds given caller's array */
#define GET(foo, attr) ( (*foo-1)->attr )
#define LEN(foo)  GET(foo, len)
#define CAP(foo)  GET(foo, cap)
#define ELSZ(foo) GET(foo, elsz)
#define REALLOC(foo) GET(foo, realloc)
#define FREE(foo)   GET(foo, free)

void fpa_destroy(hdr **foo)
{
	if (foo && *foo)
		FREE(foo)(*foo-1), *foo = NULL;
}

long long fpa_len(const hdr *src)
{
	return src? (src-1)->len : -1;
}

bool fpa_reserve(hdr **foo, size_t n)
{
	size_t elsz;
	if (foo && *foo && SIZE_MAX/(elsz = ELSZ(foo)) >= n) {
		size_t cap = CAP(foo);
		if (cap < n) {
			size_t newcap = cap+cap/2;
			if (newcap < n || SIZE_MAX/elsz < newcap)
				newcap = n;
			
			hdr *new = REALLOC(foo)(*foo-1, HDRSZ + newcap*elsz);
			if(new) {
				new->cap = newcap;
				/* Update caller's ref */
				*foo = new+1;
				return true;
			}
		} else
			return true;
	}
	return false;
}

bool fpa_insert(hdr **dst, size_t i, const void *restrict src, size_t n)
{
	if (n == 0)
		return true;
	size_t len;
	if (dst && *dst && SIZE_MAX-n >= (len = LEN(dst)) && i <= len
			&& fpa_reserve(dst, len+n)) {
		size_t elsz = ELSZ(dst);
		unsigned char *at_i = (unsigned char *)(*dst) + i*elsz;

		/* move elements at i to i+n to preserve them */
		memmove(at_i + n*elsz, at_i, (len-i)*elsz);
		/* if src == NULL, caller will emplace, don't copy */
		if (src)
			memcpy(at_i, src, n*elsz);
		LEN(dst) = len+n;
		return true;
	} else
		return false;
}

bool fpa_selfinsert(hdr **foo, size_t idst, size_t isrc, size_t n)
{
	if (n == 0)
		return true;
	size_t len;
	if (foo && *foo && SIZE_MAX-n >= (len = LEN(foo)) && idst <= len && isrc < len 
			&& fpa_reserve(foo, len+n)) {
		size_t elsz = ELSZ(foo);
		unsigned char *at_idst = (unsigned char *)(*foo) + idst*elsz,
			      *at_isrc = (unsigned char *)(*foo) + isrc*elsz;

		memmove(at_idst + n*elsz, at_idst, (len-idst) * elsz);
		/* If idst < isrc, isrc has moved n ahead.
		 * If idst == isrc, don't copy.
		 */
		memmove(at_idst, at_isrc + (idst < isrc) * n*elsz,
				n*elsz * (idst != isrc));
		LEN(foo) = len+n;
		return true;
	} else
		return false;
}

bool fpa_remove(hdr *dst, size_t i, size_t n)
{
	size_t len;
	if (dst-- && SIZE_MAX-i >= n && i+n <= (len = dst->len)) {
		size_t elsz = dst->elsz;
		unsigned char *at_i = (unsigned char *)(dst+1) + i*elsz;

		/* Shift elements at index > i one step back */
		memmove(at_i, at_i + n*elsz, (len-i-n)*elsz);
		dst->len = len-n;
		return true;
	} else
		return false;
}

void fpa_shrink_to_fit(hdr **foo)
{
	size_t len;
	/* Avoid realloc() call if not needed */
	if (foo && *foo && CAP(foo) > (len = LEN(foo)) ) {
		struct hdr *new = REALLOC(foo)(foo, HDRSZ + len*ELSZ(foo));
		if (new)
			new->cap = len, *foo = new+1;
	}
}
