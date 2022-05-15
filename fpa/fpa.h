#include <stdbool.h> /* bool   */
#include <stddef.h>  /* size_t */

/* fat pointer array handle */
typedef void * fpa;
/* pointer to fpa handle */
typedef void * fpaptr;

/* Returns alloc'd & init'd vpa of n elements elsz bytes each. */
fpa fpa_create(size_t n, size_t elsz,
        void *(*realloc)(void *, size_t), void (*free)(void *));

/* In all below functions, UB if fpaptr does not point to
 * an fpa created by prior fpa_create() and updated by last call to any of :
 * fpa_destroy(), fpa_reserve(), fpa_insert(), fpa_selfinsert(), fpa_shrink_to_fit().
 */

/* free()'s internal allocations & NULLs fpa handle */
void fpa_destroy(fpaptr);

/* Returns number of elements in fpa, or -1 when passed NULL */
long long fpa_len(const fpa);

/* Ensures capacity of at least n elements.
 * Returns true on success and false on failure.
 */
bool fpa_reserve(fpaptr, size_t n);

/* If src != NULL, inserts n elements from src at index i.
 * Else, moves elements at index >= i ahead by n, allowing 
 * caller to construct in-place at index i (emplace).
 *
 * UB if src overlaps with passed fpa.
 * Returns true if successful, else false.
 */
bool fpa_insert(fpaptr, size_t i, const void *restrict src, size_t n);

/* Inserts n elements from index isrc at index idst.
 * Returns true on success and false on failure.
 */
bool fpa_selfinsert(fpaptr, size_t idst, size_t isrc, size_t n);

/* Removes n elements from index i onwards.
 * Returns true on success and false on failure (out-of-bounds).
 */
bool fpa_remove(fpa, size_t i, size_t n);

/* Dynamic arrays overallocate for efficiency,
 * Reallocs FPA to eliminate redundant space, if any.
 */
void fpa_shrink_to_fit(fpaptr);
