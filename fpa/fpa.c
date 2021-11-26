#include <stddef.h>  /* size_t, NULL                */
#include <stdbool.h> /* bool, true, false           */
#include <stdlib.h>  /* malloc(), realloc(), free() */
#include <string.h>  /* memcpy(), memmove()         */

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

/* Bookeeping header preceeding caller's array */
struct hdr {
	size_t len, sz, elsz;

	/* Align most restrictively so struct hdr * casts to any <type> * */
	#if __STDC_VERSION__ < 201112L
	union {
		long double f; long long i; 
		void *p; void (*fp)(void);
	} _align[];
	#else
	max_align_t _align[];
	#endif
};

enum { HDR_SZ = sizeof(struct hdr) };

void *fpa_create(size_t n, size_t elsz)
{
	/* 0-size, overflow check */
	if(elsz && SIZE_MAX/elsz >= n 
			&& SIZE_MAX - HDR_SZ >= n*elsz) {
		struct hdr *new = malloc(HDR_SZ + n*elsz);
		if(new) {
			new->len = 0;
			new->sz = n*elsz;
			new->elsz = elsz;
			/* Return array region, ahead of header */
			return new+1;
		}
	}
	return NULL;
}

void fpa_destroy(struct hdr **foo)
{
	if(foo && *foo) {
		/* Free allocated region */
		free(*foo-1);
		/* Mitigate use-after-free */
		*foo = NULL;
	}
}

/* Access header feilds given caller's array */
#define LEN(foo)  ( (*foo-1)->len  )
#define SZ(foo)   ( (*foo-1)->sz   )
#define ELSZ(foo) ( (*foo-1)->elsz )

long long fpa_len(struct hdr *src)
{
	return src--? src->len : -1;
}

bool fpa_reserve(struct hdr **foo, size_t n)
{
	if(foo && *foo && SIZE_MAX/ELSZ(foo) >= n) {
		const size_t efsz = n * ELSZ(foo),
		       sz = SZ(foo);
		if(sz < efsz) {
			size_t newsz = sz+sz/2 > efsz? sz+sz/2 : efsz;
			if(SIZE_MAX-HDR_SZ >= newsz) {
				struct hdr *new = realloc(*foo-1, newsz+HDR_SZ);
				if(new) {
					new->sz = newsz;
					/* Update caller's ref */
					*foo = new+1;
					return true;
				}
			}
		} else
			return true;
	}
	return false;
}

bool fpa_insert(struct hdr **dst, size_t i, const void *restrict src, size_t n)
{
	if(!n)
		return true;
	size_t len, elsz;
	/* NULL, overflow, out of bounds and space check */
	if(dst && *dst && SIZE_MAX-n >= (len = LEN(dst)) && i <= len
			&& fpa_reserve(dst, len+n) ) {
		char *at_i = (char *)(*dst) + i * (elsz = ELSZ(dst));

		/* move elements at i to i+n to preserve them */
		memcpy(at_i + n*elsz, at_i, (len-i) * elsz);
		memcpy(at_i, src, n*elsz);

		LEN(dst) += n;
		return true;
	} else
		return false;
}

bool fpa_selfinsert(struct hdr **foo, size_t idst, size_t isrc, size_t n)
{
	if(!n)
		return true;
	size_t len, elsz;
	if(foo && *foo && SIZE_MAX-n >= (len = LEN(foo)) && idst <= len
			&& isrc < len && fpa_reserve(foo, len+n) ) {

		char *at_idst = (char *)(*foo) + idst * (elsz = ELSZ(foo)),
		     *at_isrc = (char *)(*foo)+ isrc*elsz;

		memmove(at_idst + n*elsz, at_idst, (len-idst) * elsz);
		/* If idst < isrc, isrc has moved n ahead.
		 * If idst == isrc, don't copy.
		 */
		memmove(at_idst, at_isrc + (idst < isrc) * n*elsz,
				n*elsz * (idst != isrc));
		LEN(foo) += n;
		return true;
	} else
		return false;
}

bool fpa_remove(struct hdr *dst, size_t i, size_t n)
{
	/* NULL ptr, overflow or out of bounds check */
	if(dst-- && SIZE_MAX-i >= n && i+n < dst->len) {
		const size_t elsz = dst->elsz;
		char *at_i = (char *)(dst+1) + i*elsz;

		/* Shift elements at index > i one step back */
		memmove(at_i, at_i + n*elsz, (dst->len-i-n)*elsz);
		dst->len -= n;
		return true;
	} else
		return false;
}

void fpa_shrink_to_fit(struct hdr **foo)
{
	register size_t len, elsz;
	/* Avoid realloc() call if not needed */
	if(foo && *foo && SZ(foo) > (len = LEN(foo)) * (elsz = ELSZ(foo)) ) {
		struct hdr *new = realloc(foo, HDR_SZ + len*elsz);
		if(new) {
			new->sz = new->len * new->elsz;
			*foo = new+1;
		}
	}
}
