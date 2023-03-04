#ifndef TBVPA_H
#define TBVPA_H

#include "vpa.h"

/* Silence unnecessary compiler warnings when static funcs/vars unused. */
#if __STDC_VERSION__ >= 202311L
	#define TBVPA_UNUSED [[maybe_unused]]
#elif defined __GNUC__
	#define TBVPA_UNUSED __attribute__((unused))
#else
	#define TBVPA_UNUSED
#endif

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

/* Alias `name` with vpa and generate typed bindings given eltype `...` 
 * Create additional method name_arr() to access type-casted vpa.arr
 */
#define TBVPA_GEN(name, ...)                                                 \
typedef __VA_ARGS__ name##_eltype;                                           \
typedef vpa name;                                                            \
									     \
TBVPA_UNUSED static const size_t name##_maxcap =                             \
	SIZE_MAX/sizeof(name##_eltype);                                      \
									     \
TBVPA_UNUSED static inline name##_eltype *name##_arr(const name *foo)        \
{                                                                            \
	return foo? (name##_eltype *)foo->arr : NULL;                        \
}                                                                            \
                                                                             \
TBVPA_UNUSED static inline name name##_create(size_t n)                      \
{                                                                            \
	return vpa_create(n, sizeof(name##_eltype));                         \
}                                                                            \
TBVPA_UNUSED static inline void name##_destroy(name *foo)                    \
{                                                                            \
	vpa_destroy(foo);                                                    \
}                                                                            \
TBVPA_UNUSED static inline bool name##_reserve(name *foo, size_t n)          \
{                                                                            \
	return vpa_reserve(foo, n);                                          \
}                                                                            \
TBVPA_UNUSED static inline bool name##_insert(                               \
	name *dst, size_t i, const name##_eltype *restrict src, size_t n     \
)                                                                            \
{                                                                            \
	return vpa_insert(dst, i, src, n);                                   \
}                                                                            \
TBVPA_UNUSED static inline bool name##_selfinsert(                           \
	name *foo, size_t idst, size_t isrc, size_t n                        \
)                                                                            \
{                                                                            \
	return vpa_selfinsert(foo, idst, isrc, n);                           \
}                                                                            \
TBVPA_UNUSED static inline bool name##_remove(name *foo, size_t i, size_t n) \
{                                                                            \
	return vpa_remove(foo, i, n);                                        \
}                                                                            \
TBVPA_UNUSED static inline void name##_shrink_to_fit(name *foo)              \
{                                                                            \
	vpa_shrink_to_fit(foo);                                              \
}

#endif
