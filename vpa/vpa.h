#ifndef VPA_H
#define VPA_H

#include <stdbool.h> /* bool   */
#include <stddef.h>  /* size_t */

/* arr is a buffer of len elems allocated for upto cap elems,
 * where each elem is elsz bytes in size.
 */
typedef struct vpa {
        size_t len, cap, elsz;
        void *arr;
} vpa;

/* Returns the largest possible capacity of the vpa. 
 * If .elsz is 0, returns 0.
 */
size_t vpa_maxcap(const vpa *v);

/* Returns alloc'd & init'd vpa of n elements elsz bytes each.
 * If elnum == 0  or on error; .cap = 0, .arr = NULL.
 */
vpa vpa_create(size_t n, size_t elsz);

/* free()'s .arr & resets all feilds to 0 */
void vpa_destroy(vpa *);

/* Ensures capacity of at least n elems in .arr
 * Returns true if successful, else false.
 */
bool vpa_reserve(vpa *, size_t n);

/* If src != NULL, inserts n elements from src at .arr[i]
 * Else, moves elements at index >= i ahead by n, allowing 
 * caller to construct in-place at .arr[i] (emplace).
 *
 * UB if src overlaps with .arr
 * Returns true if successful, else false.
 */
bool vpa_insert(vpa *, size_t i, const void *restrict src, size_t n);

/* Inserts n elements from .arr[isrc] at .arr[idst].
 * Returns true if successful, else false.
 */
bool vpa_selfinsert(vpa *, size_t idst, size_t isrc, size_t n);

/* Removes n elements from .arr[i] onwards.
 * Returns true on success or false on failure (out-of-bounds).
 */
bool vpa_remove(vpa *, size_t i, size_t n);

/* Dynamic arrays overallocate for efficiency,
 * Reallocs .arr (if not already) to .cap == .len
 */
void vpa_shrink_to_fit(vpa *);

#endif
