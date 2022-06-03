#ifndef STKVEC_H
#define STKVEC_H

#include <stddef.h>  /* size_t              */
#include <string.h>  /* memcpy(), memmove() */
#include <alloca.h>  /* alloca()            */

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

/* Declares vector-type "vT" of elements of type "...".
 * The element type is aliased to "vT_eltype".
 *
 * For example, "STKVEC_DECL(ivec, int)"
 * declares ivec as a vector of ivec_eltype (ints).
 *
 * Usable at any scope; semicolon optional.
 */
#define STKVEC_DECL(vT, ...)             \
	typedef __VA_ARGS__ vT##_eltype; \
	typedef struct vT {              \
		size_t len, cap;         \
		vT##_eltype *arr;        \
	} vT;

/* Initializes empty vector of vector-type "vT"
 * to capacity of n elements.
 *
 * Where,
 * "vT" is a previously declared vector-type.
 * "n"  is a unsigned integer rvalue sans side-effects.
 *
 * For example, "ivec v = STKVEC_CREATE(ivec, 0);"
 * sets v's length & capacity to 0 and array to NULL.
 *
 * Is a constant expression when n is 0.
 */
#define STKVEC_CREATE(vT, n) (vT) {                          \
	.cap = (n),                                          \
	.arr = (n)? alloca((n) * sizeof(vT##_eltype)) : NULL \
}

/* Ensures sufficient allocation to 
 * hold n vT_eltype elements in v.
 *
 * Where,
 * "vT", "n" are as specified for STKVEC_CREATE.
 * "v"  is a modifiable lvalue of type vT.
 * 
 * For example, "STKVEC_RESERVE(ivec, v, 100);"
 * allocates as needed such that v.arr can store
 * upto 100 ints.
 *
 * Allocation failure (stack overflow) is 
 * undetectable and is UB.
 */
#define STKVEC_RESERVE(vT, v, n) do {                       \
        enum { elsz = sizeof(vT##_eltype) };                \
                                                            \
        if (v.cap < (n)) {                                  \
                size_t newcap = v.cap+v.cap/2;              \
                if (newcap < (n) || SIZE_MAX/elsz < newcap) \
                        newcap = (n);                       \
                void *new = alloca(newcap*elsz);            \
                memcpy(new, v.arr, v.len*elsz);             \
                v.arr = new, v.cap = newcap;                \
        }                                                   \
} while(0)

/* Inserts n vT_eltype elements from src into v at index i,
 * setting ok to 0 if out-of-bounds. Reallocates if needed.
 * src and v.arr must not overlap.
 *
 * Where,
 * "vT", "n" are as specified for STKVEC_CREATE.
 * "v" is as specified for STKVEC_RESERVE.
 * "i" is an unsigned integer rvalue sans side-effects.
 * "src" is an array-of or pointer-to rvalue for vT_eltype
 * sans side-effects.
 * "ok" is a modifiable integer lvalue.
 *
 * For example, "STKVEC_INSERT(ivec, v, v.len, (int[]){42}, 1, (int){1});"
 * appends 42 to v.arr .
 */
#define STKVEC_INSERT(vT, v, i, src, n, ok) do {            \
        enum { elsz = sizeof(vT##_eltype) };                \
                                                            \
        if ((n) && SIZE_MAX-(n) >= v.len && (i) <= v.len) { \
                STKVEC_RESERVE(vT, v, v.len+(n));           \
                memmove(                                    \
                        v.arr+(i)+(n), v.arr+(i),           \
                        (v.len-i)*elsz                      \
                );                                          \
                if ((src))                                  \
                        memcpy(v.arr+(i), (src), (n)*elsz); \
                v.len += (n);                               \
        }  else if ((n))                                    \
                ok = 0;                                     \
} while (0)

/* Similar to STKVEC_INSERT, except src is v.arr
 * from index isrc onwards.
 *
 * For example, "STKVEC_SELFINSERT(ivec, v, v.len, 0, v.len, (int){1});"
 * appends v.arr to itself, duplicating it in-place.
 */
#define STKVEC_SELFINSERT(vT, v, idst, isrc, n, ok) do {    \
        enum { elsz = sizeof(vT##_eltype) };                \
                                                            \
        if (                                                \
                (n) && SIZE_MAX-(n) >= v.len                \
                && (idst) <= v.len && (isrc) < v.len        \
        )  {                                                \
                STKVEC_RESERVE(vT, v, v.len+(n));           \
                memmove(                                    \
                        v.arr+(idst)+(n), v.arr+(idst),     \
                        (v.len-(idst))*elsz                 \
                );                                          \
                memmove(                                    \
                        v.arr+(idst),                       \
                        v.arr+(isrc)+((idst) < (isrc))*(n), \
                        (n)*elsz*((idst) != (isrc))         \
                );                                          \
                v.len += (n);                               \
        } else if ((n))                                     \
                ok = 0;                                     \
} while (0)

/* Removes n elements of v from index i onwards,
 * setting ok to 0 if out-of-bounds.
 *
 * Where,
 * "vT", "n" are as specified for STKVEC_CREATE.
 * "v" is as specified for STKVEC_RESERVE.
 * "i", "ok" are as specified for STKVEC_INSERT.
 *
 * For example, "STKVEC(ivec, v, 0, 4, (int){0});"
 * removes the first 4 elements from v.arr .
 */
#define STKVEC_REMOVE(vT, v, i, n, ok) do {            \
        enum { elsz = sizeof(vT##_eltype) };           \
                                                       \
        if (SIZE_MAX-(i) >= (n) && (i)+(n) <= v.len) { \
                memmove(                               \
			v.arr+(i), v.arr+(i)+(n),      \
                        (v.len-(i)-(n))*elsz           \
		);                                     \
                v.len -= (n);                          \
        } else                                         \
                ok = 0;                                \
} while (0)

#endif
