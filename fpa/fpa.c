#include <stddef.h>  /* size_t, NULL, max_align_t   */
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

void *fpa_create(const size_t n, const size_t elsz)
{
	if(elsz != 0 && SIZE_MAX/elsz >= n && SIZE_MAX - HDR_SZ >= n*elsz) {
		const size_t reqsz = n*elsz;
		struct hdr *new = malloc(HDR_SZ + reqsz);

		if(new != NULL) {
			new->len = 0;
			new->sz = reqsz;
			new->elsz = elsz;
			/* Return array region, ahead of header */
			return new+1;
		}
	}
	return NULL;
}

void fpa_destroy(struct hdr **foo)
{
	if(foo != NULL && *foo != NULL) {
		free(*foo-1);
		*foo = NULL;
	}
}

/* Access header feilds given caller's array */
#define LEN(foo)  ( (*foo-1)->len  )
#define SZ(foo)   ( (*foo-1)->sz   )
#define ELSZ(foo) ( (*foo-1)->elsz )

long long fpa_len(const struct hdr *src)
{
	return src != NULL ? (src-1)->len : -1;
}

bool fpa_reserve(struct hdr **foo, const size_t n)
{
	if(foo != NULL && *foo != NULL && SIZE_MAX/ELSZ(foo) >= n) {
		const size_t minreq = n * ELSZ(foo), sz = SZ(foo);

		if(sz < minreq) {
			#define max(x,y) x > y ? x : y
			const size_t newsz = max(sz + sz/2, minreq);
			struct hdr *new = realloc(*foo-1, newsz + HDR_SZ);

			if(new != NULL) {
				new->sz = newsz;
				/* Update caller's ref */
				*foo = new+1;
				return true;
			}
		} else
			return true;
	}
	return false;
}

bool fpa_insert(struct hdr **dst, const size_t i, const void *restrict src, const size_t n)
{
	if(n == 0)
		return true;
	size_t len;
	if(dst != NULL && *dst != NULL 
			&& SIZE_MAX-n >= (len = LEN(dst)) && i <= len
			&& fpa_reserve(dst, len+n)) {

		const size_t elsz = ELSZ(dst);
		unsigned char *at_i = (unsigned char *)(*dst) + i*elsz;

		/* move elements at i to i+n to preserve them */
		memcpy(at_i + n*elsz, at_i, (len-i) * elsz);
		memcpy(at_i, src, n*elsz);

		LEN(dst) += n;
		return true;
	} else
		return false;
}

bool fpa_selfinsert(struct hdr **foo, const size_t idst, const size_t isrc, const size_t n)
{
	if(n == 0)
		return true;
	size_t len;
	if(foo != NULL && *foo != NULL && SIZE_MAX-n >= (len = LEN(foo)) 
			&& idst <= len && isrc < len 
			&& fpa_reserve(foo, len+n)) {

		const size_t elsz = ELSZ(foo);
		unsigned char *at_idst = (unsigned char *)(*foo) + idst*elsz,
			      *at_isrc = (unsigned char *)(*foo) + isrc*elsz;

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

bool fpa_remove(struct hdr *dst, const size_t i, const size_t n)
{
	size_t len;
	if(dst-- != NULL && SIZE_MAX-i >= n && i+n < (len = dst->len)) {
		const size_t elsz = dst->elsz;
		unsigned char *at_i = (unsigned char *)(dst+1) + i*elsz;

		/* Shift elements at index > i one step back */
		memmove(at_i, at_i + n*elsz, (len-i-n)*elsz);
		dst->len -= n;
		return true;
	} else
		return false;
}

void fpa_shrink_to_fit(struct hdr **foo)
{
	size_t len, elsz;
	/* Avoid realloc() call if not needed */
	if(foo != NULL && *foo != NULL 
	&& SZ(foo) > (len = LEN(foo)) * (elsz = ELSZ(foo)) ) {

		struct hdr *new = realloc(foo, HDR_SZ + len*elsz);
		if(new != NULL) {
			new->sz = len*elsz;
			*foo = new+1;
		}
	}
}
