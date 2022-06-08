# darc
`darc` stands for ***D***ynamic ***AR***ray ***C***ollection. 

This repo hosts 5 type-generic C99 implementations :

- `mga` (***M***acro ***G***enerated ***A***rray)

  Implemented with code-generating macros alike C++ class templates,
  enabling the characteristic type safety and excellent optimisations with similar drawbacks (though to a lesser degree) of
  potential binary bloat, slow compiles and long, cryptic error messages.
- `vpa` (***V***oid ***P***ointer ***A***rray)

  Instead of the monomorphisation of `mga`, `vpa` uses boxing with `void *`s.
  The benefits are extreme flexibility, low binary footprint, fast & few compiles
  and similar run-time performance to type-specific code if LTO is enabled. The drawbacks
  are the lack of static type-checking and the need to perform much type-casting.
- `fpa` (***F***at ***P***ointer ***A***rray)
  
  This employs the same "fat pointer" trick/approach as [stb_ds](http://nothings.org/stb_ds/) or [libcello](https://libcello.org/learn/a-fat-pointer-library), i.e. , the caller only deals directly with the pointer to data, and the metadata is hiddden in memory preceeding that. The advantage here is mostly just the reduction in syntactic and conceptual complexity to the user. However, the extra indirection plays spoilsport with performance (even with LTO) and tricky corruptions are possible due to silent pointer invalidation.
- `sbomga` (***S***hort ***B***uffer ***O***ptimised ***MGA***)

  Implemented in the same way as `mga`, but provides customisable short buffer optimisation with good defaults.
  Best suited to normally small, short-lived dynamic arrays - like strings - since the extra branching makes it a bit less
  efficient for larger arrays, especially in tight loops.
- `stkmga` (***St***ac***k*** ***MGA***)

  Implemented with well-documented macros to use `alloca` for all allocation/reallocation, enabling a stack-allocated
  dynamic array. As the stack is usually hot in the cache, it has excellent locality. However, allocation failure is undetectable UB
  and causes stack overflow, and macros can cause binary bloat. User descretion is advised. 

My priorities are :
1. Correctness
2. Simplicity
3. Efficiency  

The interface is mostly uniform, providing :
- `create()`, to allocate and initialize space.
- `destroy()`, to free allocations and reset bookkeeping.
- `reserve()`, ensure sufficient allocations for some number of elements.
- `insert()`, insert some number of elements at some position into array, or construct them in-place.
- `selfinsert()`, insert some number of elements from array at some position within itself.
- `remove()`, remove some number of elements from some position in array.
- `shrink_to_fit()`, free redundant allocations.
- Direct access to raw array and bookkeeping data.
- Custom allocator support.

Exact performance characteristics vary. In general, all are better than `std::vector`, as only trivially copyable elements are supported, enabling us to use `realloc`.
