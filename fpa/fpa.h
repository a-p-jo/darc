#include <stdbool.h> /* bool              */
#include <stddef.h>  /* size_t, ptrdiff_t */

/* fpa methods interact with an fpa object in one of three ways :
 * 1. fpa
 *    This is a pointer to the array data's 0th element.
 *    Methods taking such an argument modify the fpa's metadata,
 *    (say, length or capacity) which preceeds the array
 *    and is managed by fpa methods.
 *
 * 2. const fpa
 *    Like fpa, but the method only reads the metadata.
 *
 * 3. fpa_ptr
 *    This is a double pointer, i.e., pointer to the array.
 *    For example, if we do : T *x = fpa_create(42, sizeof(T))
 *    Then an fpa_ptr should be the address of x, like so :
 *    fpa_reserve(&x, 55);
 *
 *    A method accepting an fpa_ptr may modify the value of x,
 *    by reallocating it if necesssary, invalidating prior aliasing refrences.
 *    For example : T *y = x; fpa_insert(&x, ...);
 *    Now, y is an invalidated dangling pointer.
 */
typedef void * fpa;
typedef void * fpa_ptr; 

/* Returns the largest possible capacity of the vpa, or 0 if passed NULL. */
size_t fpa_maxcap(const fpa);

/* Returns pointer to number of elements in fpa, or NULL when passed NULL. 
 * The pointer is invalidated upon a call to any fpa_ptr method.
 */
size_t *fpa_len(const fpa);

/* Casts and returns fpa with new elsz, or NULL if passed NULL or 0 elsz.
 *
 * The "cast" here means updation of metadata so subsequent method calls
 * treat the fpa as an array of elements elsz bytes large.
 */
fpa fpa_cast(fpa, size_t elsz);

/* Returns alloc'd & init'd fpa of n elements elsz bytes each. 
 * 
 * This function *always* allocates, even when n is 0,
 * to hoist metadata.
 * To default-initialze an fpa, just set the pointer to NULL.
 */
fpa fpa_create(size_t n, size_t elsz);

/* free()'s internal allocations & NULLs fpa. */
void fpa_destroy(fpa_ptr);

/* Ensures capacity of at least n elements.
 * Returns true on success and false on failure.
 */
bool fpa_reserve(fpa_ptr, size_t n);

/* If src != NULL, inserts n elements from src at index i.
 * Else, moves elements at index >= i ahead by n, allowing 
 * caller to construct in-place at index i (emplace).
 *
 * UB if src overlaps with passed fpa.
 * Returns true if successful, else false.
 */
bool fpa_insert(fpa_ptr, size_t i, const void *restrict src, size_t n);

/* Inserts n elements from index isrc at index idst.
 * Returns true on success and false on failure.
 */
bool fpa_selfinsert(fpa_ptr, size_t idst, size_t isrc, size_t n);

/* Removes n elements from index i onwards.
 * Returns true on success and false on failure (out-of-bounds).
 */
bool fpa_remove(fpa, size_t i, size_t n);

/* Dynamic arrays overallocate for efficiency,
 * Reallocs fpa to eliminate redundant space, if any.
 */
void fpa_shrink_to_fit(fpa_ptr);
