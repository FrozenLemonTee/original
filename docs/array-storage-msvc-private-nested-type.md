# Split arrayStorage from array to keep private implementation types out of iterable APIs

## Background

MSVC failed to compile `vibrant_test` when `taskDelegator` stored its worker slots with:

```cpp
array<strongPtr<workerSlot>> workers_;
```

where `workerSlot` is a private nested implementation type inside `taskDelegator`.

The relevant error was:

```text
error C2248: "original::taskDelegator::workerSlot": cannot access private struct declared in class "original::taskDelegator"
```

The same code compiled with GCC.

## Root Cause

`workerSlot` is intentionally private:

```cpp
class taskDelegator {
private:
    struct workerSlot;
    array<strongPtr<workerSlot>> workers_;
};
```

However, `original::array<T>` is not only a fixed-size storage container. It also inherits public iteration and stream behavior:

```cpp
array<T>
  -> iterationStream<T, array<T>>
  -> iterable<T>
  -> iterator<T>
```

That means instantiating `array<strongPtr<workerSlot>>` also instantiates public iterator-related APIs such as:

```cpp
iterable<T>::iterAdaptor::clone()
array<T>::Iterator::clone()
randomAccessIterator<T, ALLOC>::clone()
```

Those public template instantiations mention:

```cpp
strongPtr<taskDelegator::workerSlot>
```

outside the access scope of `taskDelegator`. MSVC checks this access path and rejects the code. GCC accepts it, likely due to more permissive or delayed access checking in this template-instantiation scenario, but relying on that behavior is not portable.

## Why not make workerSlot public?

Making `workerSlot` public would make the error disappear, but it would expose an implementation detail of `taskDelegator`.

`workerSlot` owns worker-local queues, thread state, idle state, and work-stealing operations. It is not part of the public task-delegation API and should remain private.

## Why not use friend declarations?

Another possible workaround is to friend the relevant template instantiations, for example `array<strongPtr<workerSlot>>`, `iterable<strongPtr<workerSlot>>`, and iterator types.

That approach is brittle:

- The friend list depends on internal implementation details of `array` and its iterator hierarchy.
- New iterator or stream APIs can reintroduce the same error.
- A generic container hierarchy should not need special friendship with a task scheduler implementation detail.

## Chosen Design

Split fixed-size contiguous storage from public iterable container behavior:

```cpp
arrayStorage<T, ALLOC>
array<T, ALLOC>
```

`arrayStorage<T, ALLOC>` is a low-level, non-iterable storage layer. It provides:

```cpp
size()
empty()
data()
operator[]
get()
set()
swap()
```

It does not inherit:

```cpp
iterationStream
iterable
printable
comparable
hashable
iterator
```

`array<T, ALLOC>` keeps the existing public container behavior and now delegates its storage to `arrayStorage<T, ALLOC>`.

## taskDelegator Change

`taskDelegator` now uses:

```cpp
arrayStorage<strongPtr<workerSlot>> workers_;
```

instead of:

```cpp
array<strongPtr<workerSlot>> workers_;
```

This preserves:

- private `workerSlot`
- index-based worker access
- allocator-backed project-owned storage
- existing `taskDelegator` behavior

and avoids instantiating public iterable APIs with a private nested type.

## Compatibility Notes

The public `array<T, ALLOC>` API remains intact. Existing callers should continue to use `array` when they need a normal public container with iteration support.

Use `arrayStorage` for implementation storage when:

- the element type is private or otherwise should not leak through public APIs
- only `size`, `empty`, `data`, and index access are needed
- iteration/stream/comparison/hash behavior is not part of the intended interface

## Files Changed

- `src/core/arrayStorage.h`
  - Added non-iterable fixed-size contiguous storage.

- `src/core/array.h`
  - Replaced direct `size_`/`body` ownership with `arrayStorage<TYPE, ALLOC> storage_`.
  - Kept public `array` iteration and serial-container behavior.

- `src/vibrant/tasks.h`
  - Replaced `array<strongPtr<workerSlot>> workers_` with `arrayStorage<strongPtr<workerSlot>> workers_`.
  - Kept `workerSlot` private.

## Verification

MSVC Debug:

```text
cmake --build cmake-build-debug-visualstudio2022 --config Debug --target vibrant_test
```

Result: passed. The original `C2248` error did not reproduce.

MSVC Debug runtime:

```text
cmake-build-debug-visualstudio2022/test/unit_test/test_vibrant/Debug/vibrant_test.exe
```

Result:

```text
429 tests from 26 test suites ran.
429 passed.
```

MSVC Release:

```text
cmake --build cmake-build-release-visualstudio2022 --config Release --target vibrant_test
```

Result: passed.

## Known Unrelated Verification Blocker

Building the full `core_tests` target currently fails in `test_core/test_maths.cpp` on MSVC around `std::max` and `std::min` usage. This appears to be the common Windows macro collision with `max`/`min`, not an `arrayStorage` or `array` regression.

That should be handled separately, for example by using `(std::max)(...)`, `(std::min)(...)`, or ensuring `NOMINMAX` is consistently defined before Windows headers are included.

## Follow-up Work

Recommended follow-ups:

1. Add focused unit tests for `arrayStorage`.
2. Fix the unrelated MSVC `std::max`/`std::min` issue in `test_maths.cpp`.
3. Run full `core_tests` after that blocker is removed.
4. Consider gradually using `arrayStorage` in other internal implementation sites that store private types and do not need public iteration.

