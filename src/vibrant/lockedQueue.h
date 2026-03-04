#ifndef ORIGINAL_LOCKEDQUEUE_H
#define ORIGINAL_LOCKEDQUEUE_H

#include "atomic.h"
#include "condition.h"
#include "core/meta.h"
#include "mutex.h"
#include "queue.h"
#include "vector.h"


namespace original {

/**
 * @class lockedQueue
 * @brief Thread-safe FIFO queue with locking mechanism
 * @tparam TYPE Element type stored in the queue
 * @tparam SERIAL Underlying container type (default: vector)
 * @tparam ALLOC Allocator type (default: allocator)
 * @details
 * Provides a thread-safe wrapper around a FIFO queue with support for:
 * - Blocking and non-blocking operations
 * - Timeout-based operations
 * - Condition variable synchronization
 * - Atomic size tracking
 * - Head and tail access
 *
 * All operations are thread-safe and protected by internal mutex.
 * Size is maintained atomically for efficient empty() and size() checks.
 */
template <typename TYPE, template <typename, typename> typename SERIAL = vector,
          template <typename> typename ALLOC = allocator>
class lockedQueue : public noMeta {
  queue<TYPE, SERIAL, ALLOC> queue_; ///< Underlying FIFO queue
  mutable mutex mutex_{};            ///< Mutex for thread safety
  mutable condition condition_{};    ///< Condition variable for synchronization
  atomic<u_integer> size_;           ///< Atomic size counter

public:
  /**
   * @brief Constructs an empty locked queue
   */
  explicit lockedQueue();

  /**
   * @brief Checks if the queue is empty
   * @return True if queue is empty, false otherwise
   * @note Uses atomic size check for efficiency
   */
  [[nodiscard]] bool empty() const noexcept;

  /**
   * @brief Gets the number of elements in the queue
   * @return Current queue size
   * @note Uses atomic size check for efficiency
   */
  [[nodiscard]] u_integer size() const noexcept;

  /**
   * @brief Retrieves the head element without removing it
   * @return Optional containing the head element if available
   * @note Thread-safe, returns empty alternative if queue is empty
   */
  alternative<TYPE> head() const noexcept;

  /**
   * @brief Retrieves the tail element without removing it
   * @return Optional containing the tail element if available
   * @note Thread-safe, returns empty alternative if queue is empty
   */
  alternative<TYPE> tail() const noexcept;

  /**
   * @brief Pushes an element into the queue
   * @param e Element to push
   * @note Notifies one waiting thread after insertion
   */
  void push(TYPE e);

  /**
   * @brief Removes and returns the head element (blocking)
   * @return The head element from the queue
   * @details
   * Blocks the calling thread until an element is available.
   * Thread will wait on condition variable if queue is empty.
   */
  TYPE pop();

  /**
   * @brief Attempts to remove and return the head element (non-blocking)
   * @return Optional containing the head element if available
   * @note Returns empty alternative immediately if queue is empty
   */
  alternative<TYPE> tryPop();

  /**
   * @brief Removes and returns the head element with timeout
   * @param timeout Maximum duration to wait for an element
   * @return Optional containing the head element if available within timeout
   * @note Returns empty alternative if timeout expires
   */
  alternative<TYPE> popFor(time::duration timeout);

  /**
   * @brief Removes all elements from the queue
   * @note Thread-safe clear operation
   */
  void clear() noexcept;
};
} // namespace original

template <typename TYPE, template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
original::lockedQueue<TYPE, SERIAL, ALLOC>::lockedQueue()
    : size_(makeAtomic<u_integer>(0)) {}

template <typename TYPE, template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
bool original::lockedQueue<TYPE, SERIAL, ALLOC>::empty() const noexcept {
  return *this->size_ == 0;
}

template <typename TYPE, template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
original::u_integer
original::lockedQueue<TYPE, SERIAL, ALLOC>::size() const noexcept {
  return *this->size_;
}

template <typename TYPE, template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
original::alternative<TYPE>
original::lockedQueue<TYPE, SERIAL, ALLOC>::head() const noexcept {
  uniqueLock lock{this->mutex_};
  if (!this->empty())
    return alternative<TYPE>{std::move(this->queue_.head())};
  return alternative<TYPE>{};
}

template <typename TYPE, template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
original::alternative<TYPE>
original::lockedQueue<TYPE, SERIAL, ALLOC>::tail() const noexcept {
  uniqueLock lock{this->mutex_};
  if (!this->empty())
    return alternative<TYPE>{std::move(this->queue_.tail())};
  return alternative<TYPE>{};
}

template <typename TYPE, template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
void original::lockedQueue<TYPE, SERIAL, ALLOC>::push(TYPE e) {
  {
    uniqueLock lock{this->mutex_};
    this->queue_.push(std::move(e));
    this->size_ += 1;
  }
  this->condition_.notify();
}

template <typename TYPE, template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
TYPE original::lockedQueue<TYPE, SERIAL, ALLOC>::pop() {
  uniqueLock lock{this->mutex_};
  this->condition_.wait(this->mutex_, [this] { return !this->empty(); });
  TYPE e = std::move(this->queue_.pop());
  this->size_ -= 1;
  return e;
}

template <typename TYPE, template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
original::alternative<TYPE>
original::lockedQueue<TYPE, SERIAL, ALLOC>::tryPop() {
  uniqueLock lock{this->mutex_};
  if (this->empty()) {
    return alternative<TYPE>{};
  }
  TYPE e = std::move(this->queue_.pop());
  this->size_ -= 1;
  return alternative<TYPE>{std::move(e)};
}

template <typename TYPE, template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
original::alternative<TYPE>
original::lockedQueue<TYPE, SERIAL, ALLOC>::popFor(time::duration timeout) {
  uniqueLock lock{this->mutex_};
  const bool success = this->condition_.waitFor(
      this->mutex_, timeout, [this] { return !this->empty(); });
  if (success) {
    TYPE e = std::move(this->queue_.pop());
    this->size_ -= 1;
    return alternative<TYPE>{std::move(e)};
  }
  return alternative<TYPE>{};
}

template <typename TYPE, template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
void original::lockedQueue<TYPE, SERIAL, ALLOC>::clear() noexcept {
  uniqueLock lock{this->mutex_};
  this->queue_.clear();
  this->size_ = 0;
}

#endif // ORIGINAL_LOCKEDQUEUE_H