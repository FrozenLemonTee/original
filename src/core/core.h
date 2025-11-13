#ifndef CORE_H
#define CORE_H

/**
 * @file core.h
 * @brief Core module header file
 * @details Contains all fundamental components that form the core of the Original project.
 * Integrates the following main functional modules:
 *
 * @section Main_Features Main Features
 *
 * @subsection Containers
 * - Fixed-size containers: array, bitSet
 * - Variable-size containers: vector, forwardChain, chain, blocksList
 * - Associative containers: hashMap, treeMap, hashSet, treeSet, JSet, JMap
 * - Container adapters: stack, queue, deque, prique
 *
 * @subsection Memory_Management
 * - Smart pointers: ownerPtr, strongPtr, weakPtr
 * - Allocators: allocatorBase, allocator, objPoolAllocator
 * - Deleters: deleterBase, deleter
 * - Singleton pattern: singleton
 *
 * @subsection Algorithms_Iterators
 * - Boolean algorithms: allOf, anyOf, noneOf
 * - Non-modifying algorithms: find, count, equal
 * - Modifying algorithms: fill, swap, forEach
 * - Sorting algorithms: sort, stableSort, introSort
 * - Iterators: iterator, iterAdaptor, directional iterators
 * - Algorithm adapters: transform, filter, comparator
 *
 * @subsection Utilities
 * - Interfaces: printable, comparable, cloneable, iterable
 * - Tuples: couple, tuple
 * - Optional types: alternative
 * - Error handling: error, outOfBoundError, unSupportedMethodError, allocateError, staticError
 * - Maths utilities: maths
 * - Configuration: config
 * - Type definitions: types
 *
 * @section Usage
 * Include this single header to access all core functionality:
 * @code
 * #include "original/core.h"
 * @endcode
 *
 * @note Full documentation available at: https://documents-original.vercel.app/
 * @see README.md for project overview and build instructions
 * @see allocator.h For detailed memory management documentation
 * @see chain.h For linked list implementation details
 * @see array.h For fixed-size array implementation
 * @see vector.h For dynamic array implementation
 * @see algorithms.h For algorithm implementations
 */


#include "algorithms.h"
#include "allocator.h"
#include "array.h"
#include "arrayView.h"
#include "autoPtr.h"
#include "baseArray.h"
#include "baseList.h"
#include "bitSet.h"
#include "blocksList.h"
#include "chain.h"
#include "cloneable.h"
#include "comparable.h"
#include "comparator.h"
#include "config.h"
#include "container.h"
#include "containerAdapter.h"
#include "couple.h"
#include "deleter.h"
#include "deque.h"
#include "doubleDirectionIterator.h"
#include "error.h"
#include "filter.h"
#include "filterStream.h"
#include "forwardChain.h"
#include "hash.h"
#include "iterable.h"
#include "iterationStream.h"
#include "iterator.h"
#include "map.h"
#include "maps.h"
#include "maths.h"
#include "optional.h"
#include "ownerPtr.h"
#include "printable.h"
#include "prique.h"
#include "queue.h"
#include "randomAccessIterator.h"
#include "RBTree.h"
#include "refCntPtr.h"
#include "serial.h"
#include "set.h"
#include "singleDirectionIterator.h"
#include "singleton.h"
#include "skipList.h"
#include "stack.h"
#include "stepIterator.h"
#include "transform.h"
#include "transformStream.h"
#include "tuple.h"
#include "types.h"
#include "vector.h"
#include "wrapper.h"

#endif //CORE_H
