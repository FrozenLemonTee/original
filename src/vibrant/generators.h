#ifndef ORIGINAL_GENERATORS_H
#define ORIGINAL_GENERATORS_H
#include "coroutines.h"
#include "couple.h"
#include "sets.h"
#include "vector.h"


namespace original {
    /**
     * @brief Enumerates elements of a generator with their indices.
     * @tparam TYPE The type of elements in the generator.
     * @param gen The generator to enumerate.
     * @return A generator yielding couples of index and element.
     * @details Creates a sequence of (index, value) pairs starting from index 0.
     *          Useful for tracking element positions during iteration.
     *
     * Example:
     * @code
     * auto vec = vector{10, 20, 30};
     * auto gen = vec.generator();
     * for (auto [i, val] : enumerate(gen)) {
     *     std::cout << i << ": " << val << "\n";  // 0: 10, 1: 20, 2: 30
     * }
     * @endcode
     */
    template<typename TYPE>
    coroutine::generator<couple<u_integer, TYPE>> enumerate(coroutine::generator<TYPE> gen);

    /**
     * @brief Reduces a generator to a single value using an accumulator function.
     * @tparam TYPE The type of elements in the generator.
     * @tparam Callback The accumulator function type.
     * @param gen The generator to reduce.
     * @param init Initial value for the reduction.
     * @param c Binary accumulator function that combines the current result with the next element.
     * @return The final reduced value.
     * @details Applies the accumulator function sequentially to all elements of the generator,
     *          starting with the initial value. The accumulator function should have the signature
     *          `TYPE(TYPE accumulator, TYPE element)`.
     *
     * Example:
     * @code
     * auto vec = vector{1, 2, 3, 4};
     * auto gen = vec.generator();
     * auto sum = reduce(gen, 0, [](int acc, int x) { return acc + x; });  // Returns 10
     * @endcode
     */
    template<typename TYPE, typename Callback>
    TYPE reduce(coroutine::generator<TYPE> gen, TYPE init, Callback&& c);

    /**
     * @brief Finds the maximum element in a generator.
     * @tparam TYPE The type of elements in the generator.
     * @param gen The generator to search.
     * @param init Initial maximum value (typically the smallest possible value).
     * @return The maximum element found in the generator.
     * @details Compares all elements using the `maximum` function and returns the largest one.
     *          If the generator is empty, returns the initial value.
     *
     * Example:
     * @code
     * auto vec = vector{5, 2, 8, 1};
     * auto gen = vec.generator();
     * auto max_val = maximum(gen, 0);  // Returns 8
     * @endcode
     */
    template<typename TYPE>
    TYPE maximum(coroutine::generator<TYPE> gen, TYPE init);

    /**
     * @brief Finds the minimum element in a generator.
     * @tparam TYPE The type of elements in the generator.
     * @param gen The generator to search.
     * @param init Initial minimum value (typically the largest possible value).
     * @return The minimum element found in the generator.
     * @details Compares all elements using the `minimum` function and returns the smallest one.
     *          If the generator is empty, returns the initial value.
     *
     * Example:
     * @code
     * auto vec = vector{5, 2, 8, 1};
     * auto gen = vec.generator();
     * auto min_val = minimum(gen, 100);  // Returns 1
     * @endcode
     */
    template<typename TYPE>
    TYPE minimum(coroutine::generator<TYPE> gen, TYPE init);

    /**
     * @brief Computes the sum of all elements in a generator.
     * @tparam TYPE The type of elements in the generator.
     * @param gen The generator to sum.
     * @param init Initial sum value (typically 0 or identity element for addition).
     * @return The sum of all elements in the generator.
     * @details Adds all elements of the generator together using the `+` operator.
     *          If the generator is empty, returns the initial value.
     *
     * Example:
     * @code
     * auto vec = vector{1, 2, 3, 4};
     * auto gen = vec.generator();
     * auto total = summation(gen, 0);  // Returns 10
     * @endcode
     */
    template<typename TYPE>
    TYPE summation(coroutine::generator<TYPE> gen, TYPE init);

    /**
     * @brief Transforms generator elements using a callable.
     * @tparam TYPE The input element type.
     * @tparam Callback The transformation function type.
     * @param gen The source generator.
     * @param c The transformation callable.
     * @return A generator yielding transformed elements.
     * @details Applies a transformation function to each element of the input generator,
     *          producing a new generator with the transformed values.
     *
     * Example:
     * @code
     * auto vec = vector{1, 2, 3};
     * auto gen = vec.generator();
     * auto squared = transforms(gen, [](int x) { return x * x; });
     * // Yields: 1, 4, 9
     * @endcode
     */
    template<typename TYPE, typename Callback>
    auto transforms(coroutine::generator<TYPE> gen, Callback&& c) -> coroutine::generator<std::invoke_result_t<Callback, TYPE>>;

    /**
     * @brief Filters generator elements based on a predicate.
     * @tparam TYPE The element type.
     * @tparam Callback The predicate function type.
     * @param gen The source generator.
     * @param c The filter predicate.
     * @return A generator yielding only elements that satisfy the predicate.
     * @details Creates a new generator that only includes elements for which
     *          the predicate returns true.
     *
     * Example:
     * @code
     * auto vec = vector{1, 2, 3, 4, 5};
     * auto gen = vec.generator();
     * auto evens = filters(gen, [](int x) { return x % 2 == 0; });
     * // Yields: 2, 4
     * @endcode
     */
    template<typename TYPE, typename Callback>
    coroutine::generator<TYPE> filters(coroutine::generator<TYPE> gen, Callback&& c);

    /**
     * @brief Extracts elements that do not satisfy a predicate.
     * @tparam TYPE The element type.
     * @tparam Callback The predicate function type.
     * @param gen The source generator.
     * @param c The exclusion predicate.
     * @return A generator yielding elements that do not satisfy the predicate.
     * @details Opposite of filters() - excludes elements that match the predicate
     *          and includes all others.
     *
     * Example:
     * @code
     * auto vec = vector{1, 2, 3, 4, 5};
     * auto gen = vec.generator();
     * auto not_evens = extract(gen, [](int x) { return x % 2 == 0; });
     * // Yields: 1, 3, 5
     * @endcode
     */
    template<typename TYPE, typename Callback>
    coroutine::generator<TYPE> extract(coroutine::generator<TYPE> gen, Callback&& c);

    /**
     * @brief Zips two generators into pairs.
     * @tparam T Type of first generator elements.
     * @tparam U Type of second generator elements.
     * @param gen1 First generator.
     * @param gen2 Second generator.
     * @return A generator yielding couples of elements from both generators.
     * @details Pairs elements from two generators positionally. Stops when either
     *          generator is exhausted.
     *
     * Example:
     * @code
     * auto vec1 = vector{1, 2, 3};
     * auto vec2 = vector{'a', 'b', 'c'};
     * auto gen1 = vec1.generator();
     * auto gen2 = vec2.generator();
     * for (auto [num, ch] : zip(gen1, gen2)) {
     *     // Yields: (1, 'a'), (2, 'b'), (3, 'c')
     * }
     * @endcode
     */
    template<typename T, typename U>
    coroutine::generator<couple<T, U>> zip(coroutine::generator<T> gen1, coroutine::generator<U> gen2);

    /**
     * @brief Counts total elements in a generator.
     * @tparam TYPE The element type.
     * @param gen The generator to count.
     * @return The number of elements in the generator.
     * @details Consumes the generator and returns the total element count.
     *
     * Example:
     * @code
     * auto vec = vector{1, 2, 3, 4, 5};
     * auto gen = vec.generator();
     * auto cnt = count(gen);  // Returns 5
     * @endcode
     */
    template<typename TYPE>
    u_integer count(coroutine::generator<TYPE> gen);

    /**
     * @brief Counts elements satisfying a predicate.
     * @tparam TYPE The element type.
     * @tparam Callback The predicate function type.
     * @param gen The generator to count from.
     * @param c The counting predicate.
     * @return The number of elements satisfying the predicate.
     * @details Counts only elements for which the predicate returns true.
     *
     * Example:
     * @code
     * auto vec = vector{1, 2, 3, 4, 5};
     * auto gen = vec.generator();
     * auto even_count = count(gen, [](int x) { return x % 2 == 0; });  // Returns 2
     * @endcode
     */
    template<typename TYPE, typename Callback>
    u_integer count(coroutine::generator<TYPE> gen, Callback&& c);

    /**
     * @brief Checks if all elements satisfy a predicate.
     * @tparam TYPE The element type.
     * @tparam Callback The predicate function type.
     * @param gen The generator to check.
     * @param c The validation predicate.
     * @return true if all elements satisfy the predicate, false otherwise.
     * @details Short-circuits on first false result. Returns true for empty generators.
     *
     * Example:
     * @code
     * auto vec = vector{2, 4, 6, 8};
     * auto gen = vec.generator();
     * bool all_even = all(gen, [](int x) { return x % 2 == 0; });  // Returns true
     * @endcode
     */
    template<typename TYPE, typename Callback>
    bool all(coroutine::generator<TYPE> gen, Callback&& c);

    /**
     * @brief Checks if no elements satisfy a predicate.
     * @tparam TYPE The element type.
     * @tparam Callback The predicate function type.
     * @param gen The generator to check.
     * @param c The exclusion predicate.
     * @return true if no elements satisfy the predicate, false otherwise.
     * @details Short-circuits on first true result. Returns true for empty generators.
     *
     * Example:
     * @code
     * auto vec = vector{1, 3, 5, 7};
     * auto gen = vec.generator();
     * bool no_evens = none(gen, [](int x) { return x % 2 == 0; });  // Returns true
     * @endcode
     */
    template<typename TYPE, typename Callback>
    bool none(coroutine::generator<TYPE> gen, Callback&& c);

    /**
     * @brief Checks if any element satisfies a predicate.
     * @tparam TYPE The element type.
     * @tparam Callback The predicate function type.
     * @param gen The generator to check.
     * @param c The existence predicate.
     * @return true if at least one element satisfies the predicate, false otherwise.
     * @details Short-circuits on first true result. Returns false for empty generators.
     *
     * Example:
     * @code
     * auto vec = vector{1, 3, 4, 5};
     * auto gen = vec.generator();
     * bool has_even = any(gen, [](int x) { return x % 2 == 0; });  // Returns true
     * @endcode
     */
    template<typename TYPE, typename Callback>
    bool any(coroutine::generator<TYPE> gen, Callback&& c);

    /**
     * @brief Joins two generators of compatible types.
     * @tparam T Type of first generator and result elements.
     * @tparam U Type of second generator elements (must be convertible to T).
     * @param gen1 First generator.
     * @param gen2 Second generator.
     * @return A generator yielding all elements from gen1 followed by gen2.
     * @details Concatenates two generators, converting elements from the second
     *          generator to the type of the first.
     *
     * Example:
     * @code
     * auto vec1 = vector{1, 2, 3};
     * auto vec2 = vector{4, 5};
     * auto gen1 = vec1.generator();
     * auto gen2 = vec2.generator();
     * for (auto val : join(gen1, gen2)) {
     *     // Yields: 1, 2, 3, 4, 5
     * }
     * @endcode
     */
    template<typename T, std::convertible_to<T> U>
    coroutine::generator<T> join(coroutine::generator<T> gen1, coroutine::generator<U> gen2);

    /**
     * @brief Flattens a generator of couples into a single generator.
     * @tparam T Type of first element in couples.
     * @tparam U Type of second element in couples (must be convertible to T).
     * @param gen Generator of couples.
     * @return A generator yielding all elements from the couples sequentially.
     * @details Converts a generator of pairs into a flat sequence by yielding
     *          both elements of each couple in order.
     *
     * Example:
     * @code
     * auto vec = vector{couple{1, 2}, couple{3, 4}};
     * auto gen = vec.generator();
     * for (auto val : flatten(gen)) {
     *     // Yields: 1, 2, 3, 4
     * }
     * @endcode
     */
    template<typename T, std::convertible_to<T> U>
    coroutine::generator<T> flatten(coroutine::generator<couple<T, U>> gen);

    /**
     * @brief Takes the first n elements from a generator.
     * @tparam TYPE The element type.
     * @param gen The source generator.
     * @param n Number of elements to take.
     * @return A generator yielding at most n elements.
     * @details Limits the output to the first n elements of the input generator.
     *          Stops early if the input generator has fewer than n elements.
     *
     * Example:
     * @code
     * auto vec = vector{1, 2, 3, 4, 5};
     * auto gen = vec.generator();
     * for (auto val : take(gen, 3)) {
     *     // Yields: 1, 2, 3
     * }
     * @endcode
     */
    template<typename TYPE>
    coroutine::generator<TYPE> take(coroutine::generator<TYPE> gen, u_integer n);

    /**
     * @brief Skips the first n elements of a generator.
     * @tparam TYPE The element type.
     * @param gen The source generator.
     * @param n Number of elements to skip.
     * @return A generator yielding elements after the first n.
     * @details Discards the first n elements and yields the remainder of the sequence.
     *
     * Example:
     * @code
     * auto vec = vector{1, 2, 3, 4, 5};
     * auto gen = vec.generator();
     * for (auto val : skip(gen, 2)) {
     *     // Yields: 3, 4, 5
     * }
     * @endcode
     */
    template<typename TYPE>
    coroutine::generator<TYPE> skip(coroutine::generator<TYPE> gen, u_integer n);

    /**
     * @brief Finds the position of the first element satisfying a predicate.
     * @tparam TYPE The element type.
     * @tparam Callback The predicate function type.
     * @param gen The generator to search.
     * @param c The search predicate.
     * @return The index of the first matching element, or the total count if not found.
     * @details Returns the 0-based index of the first element that satisfies the predicate.
     *          If no element matches, returns the total number of elements processed.
     *
     * Example:
     * @code
     * auto vec = vector{10, 20, 30, 40};
     * auto gen = vec.generator();
     * auto pos = position(gen, [](int x) { return x > 25; });  // Returns 2
     * @endcode
     */
    template<typename TYPE, typename Callback>
    u_integer position(coroutine::generator<TYPE> gen, Callback&& c);

    /**
     * @brief Finds the first element satisfying a predicate.
     * @tparam TYPE The element type.
     * @tparam Callback The predicate function type.
     * @param gen The generator to search.
     * @param c The search predicate.
     * @return The first matching element, or default-constructed `TYPE` if not found.
     * @details Returns the first element that satisfies the predicate. If no element
     *          matches, returns a default-constructed value of type `TYPE`.
     *
     * Example:
     * @code
     * auto vec = vector{10, 20, 30, 40};
     * auto gen = vec.generator();
     * auto found = find(gen, [](int x) { return x > 25; });  // Returns 30
     * @endcode
     */
    template<typename TYPE, typename Callback>
    TYPE find(coroutine::generator<TYPE> gen, Callback&& c);

    /**
     * @class genPipe
     * @brief Pipe adapter for generator operations to enable fluent chaining.
     * @tparam Callback The operation to be applied in the pipe.
     * @details This class enables the use of the pipe operator (|) for generator
     *          operations, allowing for fluent and readable generator pipelines.
     *
     * The genPipe wraps generator operations so they can be composed using the
     * pipe operator, creating expressive data processing pipelines.
     *
     * Example usage:
     * @code
     * auto result = container.generator()
     *     | transforms([](auto x) { return x * 2; })
     *     | filters([](auto x) { return x > 10; })
     *     | take(5)
     *     | list<vector>();
     * @endcode
     *
     * @see transforms(), filters(), take(), skip(), etc. for available pipe operations.
     */
    template<typename Callback>
    class genPipe {
        Callback c_;

        genPipe() = default;

        explicit genPipe(Callback&& c);

        template<typename Generator>
        auto operator()(Generator&& gen);
    public:
        template<typename TYPE, typename F>
        friend auto operator|(coroutine::generator<TYPE> gen, genPipe<F> p);

        template<typename T, typename U>
        friend auto join(coroutine::generator<T> gen2);

        template<typename T, typename U>
        friend auto flatten();

        template<typename F>
        friend auto transforms(F&& f);

        template<typename F>
        friend auto filters(F&& f);

        template<typename F>
        friend auto extract(F&& f);

        template<typename>
        friend auto enumerate();

        template<typename>
        friend auto take(u_integer n);

        template<typename>
        friend auto skip(u_integer n);

        template<typename F>
        friend auto zipWith(coroutine::generator<F> gen2);

        template<typename>
        friend auto count();

        template<typename F>
        friend auto count(F&& f);

        template<typename F>
        friend auto all(F&& f);

        template<typename F>
        friend auto none(F&& f);

        template<typename F>
        friend auto any(F&& f);

        template<typename F>
        friend auto position(F&& f);

        template<typename F>
        friend auto find(F&& f);

        template<typename TYPE, typename F>
        friend auto reduce(TYPE init, F&& c);

        template<typename TYPE>
        friend auto maximum(TYPE init);

        template<typename TYPE>
        friend auto minimum(TYPE init);

        template<typename TYPE>
        friend auto summation(TYPE init);

        template<typename TYPE, typename SET>
        friend auto collect();

        template<template<typename> typename SERIAL>
        friend auto list();
    };

    /**
     * @brief Pipe operator for generator operations.
     * @tparam TYPE The generator element type.
     * @tparam F The pipe operation type.
     * @param gen The source generator.
     * @param p The pipe operation to apply.
     * @return The result of applying the pipe operation to the generator.
     * @details Enables fluent chaining of generator operations using the | operator.
     *
     * Example:
     * @code
     * auto processed = source_generator
     *     | transforms(transformer)
     *     | filters(predicate);
     * @endcode
     */
    template<typename TYPE, typename F>
    auto operator|(coroutine::generator<TYPE> gen, genPipe<F> p);

    /**
     * @brief Creates a transform pipe operation.
     * @tparam F The transformation function type.
     * @param f The transformation function.
     * @return A genPipe that applies the transformation.
     * @details Factory function for creating transform operations that can be
     *          used with the pipe operator.
     *
     * Example:
     * @code
     * auto double_transform = transforms([](int x) { return x * 2; });
     * auto result = gen | double_transform;
     * @endcode
     */
    template<typename F>
    auto transforms(F&& f);

    /**
     * @brief Creates a filter pipe operation.
     * @tparam F The predicate function type.
     * @param f The filter predicate.
     * @return A genPipe that applies the filter.
     * @details Factory function for creating filter operations that can be
     *          used with the pipe operator.
     */
    template<typename F>
    auto filters(F&& f);

    /**
     * @brief Creates an extract pipe operation.
     * @tparam F The predicate function type.
     * @param f The exclusion predicate.
     * @return A genPipe that applies the extraction.
     * @details Factory function for creating extract operations that can be
     *          used with the pipe operator.
     */
    template<typename F>
    auto extract(F&& f);

    /**
     * @brief Creates an enumerate pipe operation.
     * @return A genPipe that adds indices to elements.
     * @details Factory function for creating enumerate operations that can be
     *          used with the pipe operator.
     */
    template<typename = void>
    auto enumerate();

    /**
     * @brief Creates a take pipe operation.
     * @param n Number of elements to take.
     * @return A genPipe that takes the first n elements.
     * @details Factory function for creating take operations that can be
     *          used with the pipe operator.
     */
    template<typename = void>
    auto take(u_integer n);

    /**
     * @brief Creates a skip pipe operation.
     * @param n Number of elements to skip.
     * @return A genPipe that skips the first n elements.
     * @details Factory function for creating skip operations that can be
     *          used with the pipe operator.
     */
    template<typename = void>
    auto skip(u_integer n);

    /**
     * @brief Creates a join pipe operation.
     * @tparam T The target element type.
     * @tparam U The source element type.
     * @param gen2 The generator to join with.
     * @return A genPipe that joins generators.
     * @details Factory function for creating join operations that can be
     *          used with the pipe operator.
     */
    template<typename T, typename U>
    auto join(coroutine::generator<U> gen2);

    /**
     * @brief Creates a flatten pipe operation.
     * @tparam T The first element type in couples.
     * @tparam U The second element type in couples.
     * @return A genPipe that flattens couples.
     * @details Factory function for creating flatten operations that can be
     *          used with the pipe operator.
     */
    template<typename T, typename U>
    auto flatten();

    /**
     * @brief Creates a zipWith pipe operation.
     * @tparam U The element type of the second generator.
     * @param gen2 The generator to zip with.
     * @return A genPipe that zips generators.
     * @details Factory function for creating zip operations that can be
     *          used with the pipe operator.
     */
    template<typename U>
    auto zipWith(coroutine::generator<U> gen2);

    /**
     * @brief Creates a count pipe operation.
     * @return A genPipe that counts elements.
     * @details Factory function for creating count operations that can be
     *          used with the pipe operator.
     */
    template<typename = void>
    auto count();

    /**
     * @brief Creates a conditional count pipe operation.
     * @tparam F The predicate function type.
     * @param f The counting predicate.
     * @return A genPipe that counts elements satisfying the predicate.
     * @details Factory function for creating conditional count operations.
     */
    template<typename F>
    auto count(F&& f);

    /**
     * @brief Creates an all-match pipe operation.
     * @tparam F The predicate function type.
     * @param f The validation predicate.
     * @return A genPipe that checks if all elements satisfy the predicate.
     * @details Factory function for creating all-match operations.
     */
    template<typename F>
    auto all(F&& f);

    /**
     * @brief Creates a none-match pipe operation.
     * @tparam F The predicate function type.
     * @param f The exclusion predicate.
     * @return A genPipe that checks if no elements satisfy the predicate.
     * @details Factory function for creating none-match operations.
     */
    template<typename F>
    auto none(F&& f);

    /**
     * @brief Creates an any-match pipe operation.
     * @tparam F The predicate function type.
     * @param f The existence predicate.
     * @return A genPipe that checks if any element satisfies the predicate.
     * @details Factory function for creating any-match operations.
     */
    template<typename F>
    auto any(F&& f);

    /**
     * @brief Creates a position-finding pipe operation.
     * @tparam F The predicate function type.
     * @param f The search predicate.
     * @return A genPipe that finds the position of the first matching element.
     * @details Factory function for creating position-finding operations.
     */
    template<typename F>
    auto position(F&& f);

    /**
     * @brief Creates an element-finding pipe operation.
     * @tparam F The predicate function type.
     * @param f The search predicate.
     * @return A genPipe that finds the first matching element.
     * @details Factory function for creating element-finding operations.
     */
    template<typename F>
    auto find(F&& f);

    /**
     * @brief Creates a reduce pipe operation.
     * @tparam TYPE The element type.
     * @tparam Callback The reduction function type.
     * @param init Initial value for reduction.
     * @param c Reduction function.
     * @return A genPipe that reduces the generator.
     * @details Factory function for creating reduce operations.
     */
    template<typename TYPE, typename Callback>
    auto reduce(TYPE init, Callback&& c);

    /**
     * @brief Creates a maximum pipe operation.
     * @tparam TYPE The element type.
     * @param init Initial maximum value.
     * @return A genPipe that finds the maximum element.
     * @details Factory function for creating maximum operations.
     */
    template<typename TYPE>
    auto maximum(TYPE init);

    /**
     * @brief Creates a minimum pipe operation.
     * @tparam TYPE The element type.
     * @param init Initial minimum value.
     * @return A genPipe that finds the minimum element.
     * @details Factory function for creating minimum operations.
     */
    template<typename TYPE>
    auto minimum(TYPE init);

    /**
     * @brief Creates a summation pipe operation.
     * @tparam TYPE The element type.
     * @param init Initial sum value.
     * @return A genPipe that sums all elements.
     * @details Factory function for creating summation operations.
     */
    template<typename TYPE>
    auto summation(TYPE init);

    /**
     * @brief Creates a collect pipe operation.
     * @tparam TYPE The type of elements in the generator.
     * @tparam SET The set type to collect into (default: hashSet).
     * @return A genPipe that collects elements into a set.
     * @details Factory function for creating collect operations.
     */
    template<typename TYPE, typename SET = hashSet<TYPE>>
    auto collect();

    /**
     * @brief Creates a list pipe operation.
     * @tparam SERIAL The list container type.
     * @return A genPipe that collects elements into a list.
     * @details Factory function for creating list operations.
     */
    template<template<typename> typename SERIAL = vector>
    auto list();

    /**
     * @brief Collects generator elements into a set.
     * @tparam TYPE The type of elements in the generator.
     * @tparam SET The set type to collect into (default: hashSet).
     * @param gen The generator to collect from.
     * @return A set containing all unique elements from the generator.
     * @details Transforms a generator sequence into a set container, removing duplicates
     *          and providing fast lookup capabilities.
     *
     * Example:
     * @code
     * auto vec = vector{1, 2, 2, 3, 3, 3};
     * auto gen = vec.generator();
     * auto set = collect<int>(gen);  // {1, 2, 3}
     * @endcode
     */
    template<typename TYPE, typename SET = hashSet<TYPE>>
    requires original::ExtendsOf<set<TYPE, allocator<couple<const TYPE, const bool>>>, SET>
    SET collect(coroutine::generator<TYPE> gen)
    {
        SET set;
        for (auto elem : gen)
        {
            set.add(elem);
        }
        return set;
    }

    /**
     * @brief Collects generator elements into a list container.
     * @tparam TYPE The type of elements in the generator.
     * @tparam SERIAL The list container type (default: vector).
     * @param gen The generator to collect from.
     * @return A list container with all generator elements in order.
     * @details Converts a generator sequence into a concrete list container,
     *          preserving element order and allowing random access.
     *
     * Example:
     * @code
     * auto gen = someContainer.generator();
     * auto vec = list<int>(gen);  // Creates vector<int> with all elements
     * @endcode
     */
    template <typename TYPE, template <typename> typename SERIAL = vector>
    requires original::ExtendsOf<baseList<TYPE, allocator<TYPE>>, SERIAL<TYPE>>
    SERIAL<TYPE> list(coroutine::generator<TYPE> gen)
    {
        SERIAL<TYPE> list;
        for (auto elem : gen)
        {
            list.pushEnd(elem);
        }
        return list;
    }
}

template <typename TYPE>
original::coroutine::generator<original::couple<original::u_integer, TYPE>>
original::enumerate(coroutine::generator<TYPE> gen)
{
    u_integer i = 0;
    for (auto elem : gen)
    {
        co_yield {i, elem};
        i += 1;
    }
}

template <typename TYPE, typename Callback>
TYPE original::reduce(coroutine::generator<TYPE> gen, TYPE init, Callback&& c)
{
    TYPE result = std::move(init);
    for (auto elem : gen)
    {
        result = c(std::move(result), std::forward<TYPE>(elem));
    }
    return result;
}

template <typename TYPE>
TYPE original::maximum(coroutine::generator<TYPE> gen, TYPE init)
{
    return reduce<TYPE>(std::move(gen), std::move(init), [](TYPE max, TYPE cur) { return maximum(max, cur); });
}

template <typename TYPE>
TYPE original::minimum(coroutine::generator<TYPE> gen, TYPE init)
{
    return reduce<TYPE>(std::move(gen), std::move(init), [](TYPE min, TYPE cur) { return minimum(min, cur); });
}

template <typename TYPE>
TYPE original::summation(coroutine::generator<TYPE> gen, TYPE init)
{
    return reduce<TYPE>(std::move(gen), std::move(init), [](TYPE sum, TYPE cur) { return std::move(sum) + std::move(cur); });
}

template <typename TYPE, typename Callback>
auto original::transforms(coroutine::generator<TYPE> gen, Callback&& c) -> coroutine::generator<std::invoke_result_t<Callback, TYPE>>
{
    for (auto elem : gen)
    {
        co_yield c(std::forward<TYPE>(elem));
    }
}

template <typename TYPE, typename Callback>
original::coroutine::generator<TYPE>
original::filters(coroutine::generator<TYPE> gen, Callback&& c)
{
    for (auto elem : gen)
    {
        if (c(std::forward<TYPE>(elem)))
        {
            co_yield elem;
        }
    }
}

template <typename TYPE, typename Callback>
original::coroutine::generator<TYPE>
original::extract(coroutine::generator<TYPE> gen, Callback&& c)
{
    for (auto elem : gen)
    {
        if (c(std::forward<TYPE>(elem)))
        {
            continue;
        }
        co_yield elem;
    }
}

template <typename T, typename U>
original::coroutine::generator<original::couple<T, U>>
original::zip(coroutine::generator<T> gen1, coroutine::generator<U> gen2)
{
    auto elem1 = gen1.next();
    auto elem2 = gen2.next();
    while (elem1 && elem2)
    {
        co_yield {std::move(*elem1), std::move(*elem2)};
        elem1 = std::move(gen1.next());
        elem2 = std::move(gen2.next());
    }
}

template <typename TYPE>
original::u_integer original::count(coroutine::generator<TYPE> gen)
{
    u_integer res = 0;
    while (auto val = gen.next())
    {
        res += 1;
    }
    return res;
}

template <typename TYPE, typename Callback>
original::u_integer original::count(coroutine::generator<TYPE> gen, Callback&& c)
{
    u_integer res = 0;
    while (auto val = gen.next())
    {
        if (c(std::forward<TYPE>(*val)))
            res += 1;
    }
    return res;
}

template <typename TYPE, typename Callback>
bool original::all(coroutine::generator<TYPE> gen, Callback&& c)
{
    while (auto val = gen.next())
    {
        if (!c(std::forward<TYPE>(*val)))
            return false;
    }
    return true;
}

template <typename TYPE, typename Callback>
bool original::none(coroutine::generator<TYPE> gen, Callback&& c)
{
    while (auto val = gen.next())
    {
        if (c(std::forward<TYPE>(*val)))
            return false;
    }
    return true;
}

template <typename TYPE, typename Callback>
bool original::any(coroutine::generator<TYPE> gen, Callback&& c)
{
    while (auto val = gen.next())
    {
        if (c(std::forward<TYPE>(*val)))
            return true;
    }
    return false;
}

template<typename T, std::convertible_to<T> U>
original::coroutine::generator<T>
original::join(coroutine::generator<T> gen1, coroutine::generator<U> gen2)
{
    for (auto elem : gen1)
    {
        co_yield elem;
    }
    for (auto elem : gen2)
    {
        co_yield static_cast<T>(elem);
    }
}

template <typename T, std::convertible_to<T> U>
original::coroutine::generator<T>
original::flatten(coroutine::generator<couple<T, U>> gen)
{
    for (auto [elem1, elem2] : gen)
    {
        co_yield elem1;
        co_yield static_cast<T>(elem2);
    }
}

template <typename TYPE>
original::coroutine::generator<TYPE>
original::take(coroutine::generator<TYPE> gen, const u_integer n)
{
    u_integer i = 0;
    for (auto elem : gen)
    {
        if (i == n)
        {
            co_return;
        }
        i += 1;
        co_yield elem;
    }
}

template <typename TYPE>
original::coroutine::generator<TYPE>
original::skip(coroutine::generator<TYPE> gen, const u_integer n)
{
    u_integer i = 0;
    for (auto elem : gen)
    {
        if (i < n)
        {
            i += 1;
            continue;
        }
        co_yield elem;
    }
}

template <typename TYPE, typename Callback>
original::u_integer original::position(coroutine::generator<TYPE> gen, Callback&& c)
{
    u_integer i = 0;
    for (auto elem : gen)
    {
        if (c(std::forward<TYPE>(elem))){
            return i;
        }
        i += 1;
    }
    return i;
}

template <typename TYPE, typename Callback>
TYPE original::find(coroutine::generator<TYPE> gen, Callback&& c)
{
    for (auto elem : gen)
    {
        if (c(std::forward<TYPE>(elem)))
        {
            return elem;
        }
    }
    return TYPE{};
}

template <typename Callback>
original::genPipe<Callback>::genPipe(Callback&& c) : c_(std::forward<Callback>(c)) {}

template <typename Callback>
template <typename Generator>
auto original::genPipe<Callback>::operator()(Generator&& gen)
{
    return this->c_(std::forward<Generator>(gen));
}

template <typename TYPE, typename F>
auto original::operator|(coroutine::generator<TYPE> gen, genPipe<F> p)
{
    return p(std::move(gen));
}

template <typename F>
auto original::transforms(F&& f)
{
    return genPipe{[c = std::forward<F>(f)]<typename TYPE>(coroutine::generator<TYPE> gen) mutable {
        return transforms(std::move(gen), std::move(c));
    }};
}

template <typename F>
auto original::filters(F&& f)
{
    return genPipe{[c = std::forward<F>(f)]<typename TYPE>(coroutine::generator<TYPE> gen) mutable {
        return filters(std::move(gen), std::move(c));
    }};
}

template <typename F>
auto original::extract(F&& f)
{
    return genPipe{[c = std::forward<F>(f)]<typename TYPE>(coroutine::generator<TYPE> gen) mutable {
        return extract(std::move(gen), std::move(c));
    }};
}

template<typename>
auto original::enumerate()
{
    return genPipe{[]<typename TYPE>(coroutine::generator<TYPE> gen) mutable {
        return enumerate(std::move(gen));
    }};
}

template<typename>
auto original::take(u_integer n)
{
    return genPipe{[n]<typename TYPE>(coroutine::generator<TYPE> gen) mutable {
        return take(std::move(gen), n);
    }};
}

template<typename>
auto original::skip(u_integer n)
{
    return genPipe{[n]<typename TYPE>(coroutine::generator<TYPE> gen) mutable {
        return skip(std::move(gen), n);
    }};
}

template <typename T, typename U>
auto original::join(coroutine::generator<U> gen2)
{
    return genPipe{[gen2 = std::move(gen2)]<typename TYPE>(coroutine::generator<TYPE> gen1) mutable {
        return join<T, TYPE>(std::move(gen1), std::move(gen2));
    }};
}

template<typename T, typename U>
auto original::flatten()
{
    return genPipe{[]<typename TYPE>(coroutine::generator<TYPE> gen) mutable {
        return flatten<T, U>(std::move(gen));
    }};
}

template<typename U>
auto original::zipWith(coroutine::generator<U> gen2)
{
    return genPipe{[gen2 = std::move(gen2)]<typename T>(coroutine::generator<T> gen1) mutable {
        return zip(std::move(gen1), std::move(gen2));
    }};
}

template<typename>
auto original::count()
{
    return genPipe{[]<typename TYPE>(coroutine::generator<TYPE> gen) {
        return count(std::move(gen));
    }};
}

template<typename F>
auto original::count(F&& f)
{
    return genPipe{[c = std::forward<F>(f)]<typename TYPE>(coroutine::generator<TYPE> gen) mutable {
        return count(std::move(gen), std::move(c));
    }};
}

template<typename F>
auto original::all(F&& f)
{
    return genPipe{[c = std::forward<F>(f)]<typename TYPE>(coroutine::generator<TYPE> gen) mutable {
        return all(std::move(gen), std::move(c));
    }};
}

template<typename F>
auto original::none(F&& f)
{
    return genPipe{[c = std::forward<F>(f)]<typename TYPE>(coroutine::generator<TYPE> gen) mutable {
        return none(std::move(gen), std::move(c));
    }};
}

template<typename F>
auto original::any(F&& f)
{
    return genPipe{[c = std::forward<F>(f)]<typename TYPE>(coroutine::generator<TYPE> gen) mutable {
        return any(std::move(gen), std::move(c));
    }};
}

template<typename F>
auto original::position(F&& f)
{
    return genPipe{[c = std::forward<F>(f)]<typename TYPE>(coroutine::generator<TYPE> gen) mutable {
        return position(std::move(gen), std::move(c));
    }};
}

template<typename F>
auto original::find(F&& f)
{
    return genPipe{[c = std::forward<F>(f)]<typename TYPE>(coroutine::generator<TYPE> gen) mutable {
        return find(std::move(gen), std::move(c));
    }};
}

template <typename TYPE, typename Callback>
auto original::reduce(TYPE init, Callback&& c)
{
    return genPipe{[c = std::forward<Callback>(c), init = std::move(init)](coroutine::generator<TYPE> gen) mutable
    {
       return reduce<TYPE>(std::move(gen), std::move(init), std::move(c));
    }};
}

template <typename TYPE>
auto original::maximum(TYPE init)
{
    return genPipe{[init = std::move(init)](coroutine::generator<TYPE> gen) mutable
    {
       return maximum<TYPE>(std::move(gen), std::move(init));
    }};
}

template <typename TYPE>
auto original::minimum(TYPE init)
{
    return genPipe{[init = std::move(init)](coroutine::generator<TYPE> gen) mutable
    {
        return minimum<TYPE>(std::move(gen), std::move(init));
    }};
}

template <typename TYPE>
auto original::summation(TYPE init)
{
    return genPipe{[init = std::move(init)](coroutine::generator<TYPE> gen) mutable
    {
        return summation<TYPE>(std::move(gen), std::move(init));
    }};
}

template <typename TYPE, typename SET>
auto original::collect()
{
    return genPipe{[](coroutine::generator<TYPE> gen) mutable
    {
       return collect<TYPE, SET>(std::move(gen));
    }};
}

template <template <typename> class SERIAL>
auto original::list()
{
    return genPipe{[]<typename TYPE>(coroutine::generator<TYPE> gen) mutable {
        return list<TYPE, SERIAL>(std::move(gen));
    }};
}

#endif //ORIGINAL_GENERATORS_H
