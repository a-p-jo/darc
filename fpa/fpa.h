#include <stdbool.h> /* bool   */
#include <stddef.h>  /* size_t */

/* fat pointer array handle */
typedef void * FPA;
/* pointer to fpa handle */
typedef void * FPA_PTR; 

/* Returns alloc'd & init'd vpa of n elements elsz bytes each. */
FPA fpa_create(size_t elnum, size_t elsz);

/* In all below functions, UB if FPA_PTR does not point to
 * an FPA created by prior fpa_create() and updated by last call to any of :
 * fpa_destroy(), fpa_reserve(), fpa_insert(), fpa_selfinsert(), fpa_shrink_to_fit().
 */

/* free()'s internal allocations & NULLs FPA handle */
void fpa_destroy(FPA_PTR);

/* Returns number of elements in FPA, or -1 on error */
long long fpa_len(FPA);

/* Ensures capacity of at least n elements.
 * Returns true on success and false on failure.
 */
bool fpa_reserve(FPA_PTR, size_t n);

/* Inserts [src[i], src[i+n]) at index i in FPA.
 * Returns true on success and false on failure.
 */
bool fpa_insert(FPA_PTR, size_t i, const void *restrict src, size_t n);

/* Inserts elements in indexes [isrc, isrc+n] in FPA at index idst.
 * Returns true on success and false on failure.
 */
bool fpa_selfinsert(FPA_PTR, size_t idst, size_t isrc, size_t n);

/* Removes elements in indexes [i, i+n) in FPA.
 * Returns true on success and false on failure.
 */
bool fpa_remove(FPA, size_t i, size_t n);

/* Dynamic arrays overallocate for efficiency,
 * Reallocs FPA to eliminate redundant space, if any.
 */
void fpa_shrink_to_fit(FPA_PTR);
