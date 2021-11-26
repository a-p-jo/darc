#ifndef VPA_H
#define VPA_H

#include <stdbool.h> /* bool   */
#include <stddef.h>  /* size_t */

typedef struct vpa {
	/* buffer .arr is of .sz bytes allocated space
	 * containing .len elements .elsz bytes each.
	 */ 
	size_t len, sz, elsz;
	void *arr; 
} vpa;

/* Returns alloc'd & init'd vpa of n elements elsz bytes each.
 * If elnum == 0  or on error; .sz = 0, .arr = NULL.
 */
vpa vpa_create(size_t n, size_t elsz);

/* free()'s .arr & resets all feilds to 0 */
void vpa_destroy(vpa *);

/* Ensures capacity of at least n elements in .arr
 * realloc()'s .arr if .sz < n * .elsz
 * Returns true if successful, else false.
 */
bool vpa_reserve(vpa *, size_t n);

/* Inserts [src[i], src[i+n]) at .arr[i].
 * Reallocs if .sz < .len + n * .elsz
 * UB if src overlaps with .arr
 * Returns true if successful, else false.
 */
bool vpa_insert(vpa *, size_t i, const void *restrict src, size_t n);

/* Inserts [.arr[isrc], .arr[isrc+n]) at .arr[idst].
 * Reallocs if .sz < .len + n * .elsz
 * Returns true if successful, else false.
 */
bool vpa_selfinsert(vpa *, size_t idst, size_t isrc, size_t n);

/* Removes [.arr[i], .arr[i+n])
 * Returns true on success or false on failure (out-of-bounds).
 */
bool vpa_remove(vpa *, size_t i, size_t n);

/* Dynamic arrays overallocate for efficiency,
 * Reallocs .arr to eliminate redundant space, if any.
 */
void vpa_shrink_to_fit(vpa *);

#endif
