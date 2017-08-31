/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_core/target_gnu/roc_core/atomic.h
//! @brief Atomic integer.

#ifndef ROC_CORE_ATOMIC_H_
#define ROC_CORE_ATOMIC_H_

#include "roc_core/noncopyable.h"

namespace roc {
namespace core {

//! Atomic integer.
class Atomic : public NonCopyable<> {
public:
    //! Initialize with given value.
    explicit Atomic(long value = 0)
        : value_(value) {
    }

    //! Atomic load.
    operator long() const {
        return __sync_add_and_fetch(&value_, 0);
    }

    //! Atomic store.
    //! @remarks
    //!  Only boolean values may be implemented in a cross-platform way
    //!  using GCC legacy __sync builtins.
    long operator=(bool v) {
        if (v) {
            __sync_lock_test_and_set(&value_, 1);
        } else {
            __sync_and_and_fetch(&value_, 0);
        }
        return v;
    }

    //! Atomic increment.
    long operator++() {
        return __sync_add_and_fetch(&value_, 1);
    }

    //! Atomic decrement.
    long operator--() {
        return __sync_sub_and_fetch(&value_, 1);
    }

    //! Atomic test-and-set.
    //! @remarks
    //!  Atomically sets value to non-zero and returns '0' if previous value
    //!  was '0' or '1' otherwise.
    long test_and_set() {
        return __sync_bool_compare_and_swap(&value_, 0, 1) ? 0 : 1;
    }

private:
    mutable long value_;
};

} // namespace core
} // namespace roc

#endif // ROC_CORE_ATOMIC_H_
