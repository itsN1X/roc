/*
 * Copyright (c) 2017 Mikhail Baranov
 * Copyright (c) 2017 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_core/rate_limiter.h
//! @brief Rate limiter.

#ifndef ROC_CORE_RATE_LIMITER_H_
#define ROC_CORE_RATE_LIMITER_H_

#include "roc_core/noncopyable.h"
#include "roc_core/ticker.h"

namespace roc {
namespace core {

//! Rate limiter.
class RateLimiter : public NonCopyable<> {
public:
    //! Initialize rate limiter.
    //! @remarks
    //!  @p period_us is tick duration in microseconds.
    RateLimiter(uint64_t period_us)
        : period_(period_us)
        , pos_(0)
        , ticker_(1000000) {
    }

    //! Check if it's time for a next tick and do it if so.
    //! @returns true if a tick occured.
    bool try_next() {
        if (ticker_.elapsed() >= pos_) {
            pos_ += period_;
            return true;
        } else {
            return false;
        }
    }

private:
    const uint64_t period_;
    uint64_t pos_;
    Ticker ticker_;
};

} // namespace core
} // namespace roc

#endif // ROC_CORE_RATE_LIMITER_H_
