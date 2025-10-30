# ThreadPool-Cpp
This repository contains a C++20 implementation of a Thread Pool, with various extra functionalities!

## Documentation

### Constructors

- iK_from_i32(_n: i32) -> iK;

Instantiate an iK integer from an i32;

    @compute
    @workgroup_size(1, 1)
    fn cs() {
        var a = i128_from_i32(-42);
    }

- iK_from_u32(_n: u32) -> iK;

Instantiate an iK integer from an u32;

    @compute
    @workgroup_size(1, 1)
    fn cs() {
        var a = i64_from_u32(42);
    }

- iK_from_iJ(_n: iJ) -> iK;

Instantiate an iK integer from an iJ, useful when you realize that the result of your future operations might not fit in the current iJ;

    @compute
    @workgroup_size(1, 1)
    fn cs() {
        var a = i64_from_i32(-42);
        var b = i128_from_i64(a);
    }

- iK_from_u32_array(_number: array<u32, L>, _sign: i32) -> iK;

Instantiate an iK integer from an array of L u32 integers, useful when moving a BigInt from CPU to GPU;

_sign must be either 1 (Non-negative integer) or -1 (Negative integer), undefined behaviour otherwise;

    @compute
    @workgroup_size(1, 1)
    fn cs() {
        var arr = array<u32, 4>(1, 2, 3, 4);
        var a   = i128_from_u32_array(arr, 1); // Now contains 1*2^96 + 2*2^64 + 3*2^32 + 4, or 79228162551157825753847955460
    }
