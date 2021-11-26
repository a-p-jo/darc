# darc
`darc` stands for ***D***ynamic ***AR***ray ***C***ollection. 

This repository currently hosts 3 variants :

1. `mga` (***M***acro ***G***enerated ***A***rray),
2. `vpa` (***V***oid ***P***ointer ***A***rray), 
3. `fpa` (***F***at ***P***ointer ***A***rray)

The above are separately maintained and have their own READMEs.

The values of this project, in decreasing order of priority, are :
1. Safety and correctness
2. Simplicity and minimalism
3. Performance and efficiency

The interface of these is somewhat uniform. The methods provided are :
- `create()`, to allocate and initialize space.
- `destroy()`, to free allocations and reset bookkeeping.
- `reserve()`, ensure sufficient allocations for some number of elements.
- `insert()`, insert some number of elements at some position into array.
- `selfinsert()`, insert some number of elements from array at some position within itself.
- `remove()`, remove some number of elements from some position in array.
- `shrink_to_fit()`, free redundant allocations.

Performance varies by platform/compiler/usecase. 
`mga` and `vpa` are close, `fpa` is slowest by a significant margin, all three are measurably faster than `std::vector`.
