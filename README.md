# darc
`darc` stands for ***D***ynamic ***AR***ray ***C***ollection. 

This repo hosts 3 type-generic C99 implementations :

1. `mga` (***M***acro ***G***enerated ***A***rray)
- type-safe
- 0-cost abstraction, allows type-specific optimisations.
2. `vpa` (***V***oid ***P***ointer ***A***rray) :
- least code size
- most flexible
3. `fpa` (***F***at ***P***ointer ***A***rray) :
- simpler syntax 
- safer (bookeeping info not overwritable).
4. `stkvec` (**St**ac**k** **Vec**tor) :
- similar to `mga`; uses `alloca()`
- unsafe due to risk of stack overflow

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

Exact performance characteristics vary. In general, `mga` and `vpa` are equivalent, `fpa` is much slower; all three are better than `std::vector`.
