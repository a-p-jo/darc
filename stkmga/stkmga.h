#ifndef STKMGA_H
#define STKMGA_H

#include <stddef.h>  /* size_t              */
#include <string.h>  /* memcpy(), memmove() */
#include <alloca.h>  /* alloca()            */

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

/* Declares vector-type "vT" of elements of type "...".
 * The element type is aliased to "vT_eltype".
 * The maximum capacity is set to "vT_maxcap".
 *
 * For example, "STKMGA_DECL(ivec, int)"
 * declares ivec as a vector of ivec_eltype (ints).
 *
 * Usable at any scope; semicolon optional.
 */
#define STKMGA_DECL(vT, ...)                                            \
	typedef __VA_ARGS__ vT##_eltype;                                \
	static const size_t vT##_maxcap = SIZE_MAX/sizeof(vT##_eltype); \
	typedef struct vT {                                             \
		size_t len, cap;                                        \
		vT##_eltype *arr;                                       \
	} vT;

/* Initializes empty vector of vector-type "vT"
 * to capacity of n elements.
 *
 * Where,
 * "vT" is a previously declared vector-type.
 * "n"  is a unsigned integer rvalue sans side-effects
 * that does not exceed vT_maxcap.
 *
 * For example, "ivec v = STKMGA_CREATE(ivec, 0);"
 * sets v's length & capacity to 0 and array to NULL.
 *
 * Is a constant expression when n is 0.
 */
#define STKMGA_CREATE(vT, n) (vT) {                          \
	.cap = (n),                                          \
	.arr = (n)? alloca((n) * sizeof(vT##_eltype)) : NULL \
}

/* Ensures sufficient allocation to 
 * hold n vT_eltype elements in v.
 *
 * Where,
 * "vT", "n" are as specified for STKMGA_CREATE.
 * "v"  is a modifiable lvalue of type vT.
 * 
 * For example, "STKMGA_RESERVE(ivec, v, 100);"
 * allocates as needed such that v.arr can store
 * upto 100 ints.
 *
 * Allocation failure (stack overflow) is 
 * undetectable and is UB.
 */
#define STKMGA_RESERVE(vT, v, n) do {                       \
        enum { elsz = sizeof(vT##_eltype) };                \
                                                            \
        if (v.cap < (n)) {                                  \
                size_t newcap = v.cap+v.cap/2;              \
                if (newcap < (n) || newcap > vT##_maxcap)   \
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
 * "vT", "n" are as specified for STKMGA_CREATE.
 * "v" is as specified for STKMGA_RESERVE.
 * "i" is an unsigned integer rvalue sans side-effects.
 * "src" is an array-of or pointer-to rvalue for vT_eltype
 * sans side-effects.
 * "ok" is a modifiable integer lvalue.
 *
 * For example, "STKMGA_INSERT(ivec, v, v.len, (int[]){42}, 1, (int){1});"
 * appends 42 to v.arr .
 */
#define STKMGA_INSERT(vT, v, i, src, n, ok) do {               \
        enum { elsz = sizeof(vT##_eltype) };                   \
                                                               \
        if ((n) && vT##_maxcap-(n) >= v.len && (i) <= v.len) { \
                STKMGA_RESERVE(vT, v, v.len+(n));              \
                memmove(                                       \
                        v.arr+(i)+(n), v.arr+(i),              \
                        (v.len-i)*elsz                         \
                );                                             \
                if ((src))                                     \
                        memcpy(v.arr+(i), (src), (n)*elsz);    \
                v.len += (n);                                  \
        }  else if ((n))                                       \
                ok = 0;                                        \
} while (0)

/* Similar to STKMGA_INSERT, except src is v.arr
 * from index isrc onwards.
 *
 * For example, "STKMGA_SELFINSERT(ivec, v, v.len, 0, v.len, (int){1});"
 * appends v.arr to itself, duplicating it in-place.
 */
#define STKMGA_SELFINSERT(vT, v, idst, isrc, n, ok) do {    \
        enum { elsz = sizeof(vT##_eltype) };                \
                                                            \
        if (                                                \
                (n) && vT##_maxcap-(n) >= v.len             \
                && (idst) <= v.len && (isrc) < v.len        \
        )  {                                                \
                STKMGA_RESERVE(vT, v, v.len+(n));           \
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
 * "vT", "n" are as specified for STKMGA_CREATE.
 * "v" is as specified for STKMGA_RESERVE.
 * "i", "ok" are as specified for STKMGA_INSERT.
 *
 * For example, "STKMGA(ivec, v, 0, 4, (int){0});"
 * removes the first 4 elements from v.arr .
 */
#define STKMGA_REMOVE(vT, v, i, n, ok) do {               \
        enum { elsz = sizeof(vT##_eltype) };              \
                                                          \
        if (vT##_maxcap-(i) >= (n) && (i)+(n) <= v.len) { \
                memmove(                                  \
			v.arr+(i), v.arr+(i)+(n),         \
                        (v.len-(i)-(n))*elsz              \
		);                                        \
                v.len -= (n);                             \
        } else                                            \
                ok = 0;                                   \
} while (0)

#endif
