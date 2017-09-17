/*
 * Copyright (c) 2017 Mikhail Baranov
 * Copyright (c) 2017 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_core/heap_allocator.h
//! @brief Heap allocator implementation.

#ifndef ROC_CORE_HEAP_ALLOCATOR_H_
#define ROC_CORE_HEAP_ALLOCATOR_H_

#include "roc_core/atomic.h"
#include "roc_core/iallocator.h"
#include "roc_core/noncopyable.h"

namespace roc {
namespace core {

//! Heap allocator implementation.
//!
//! Uses global operator new[] and operator delete[].
//!
//! The memory is always maximum aligned. Thread-safe.
class HeapAllocator : public IAllocator, public NonCopyable<> {
public:
    ~HeapAllocator();

    //! Allocate memory.
    virtual void* allocate(size_t size);

    //! Deallocate previously allocated memory.
    virtual void deallocate(void*);

    //! Get number of allocated blocks.
    size_t num_allocations() const;

private:
    Atomic num_allocations_;
};

} // namespace core
} // namespace roc

#endif // ROC_CORE_HEAP_ALLOCATOR_H_
