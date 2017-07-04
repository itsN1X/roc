/*
 * Copyright (c) 2017 Mikhail Baranov
 * Copyright (c) 2017 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_core/ticker.h
//! @brief Ticker.

#ifndef ROC_CORE_TICKER_H_
#define ROC_CORE_TICKER_H_

#include "roc_core/noncopyable.h"
#include "roc_core/time.h"

namespace roc {
namespace core {

//! Ticker.
class Ticker : public NonCopyable<> {
public:
    //! Initialize.
    //! @remarks
    //!  @p freq defines the number of ticks per second.
    Ticker(uint64_t freq)
        : freq_(freq)
        , start_(0) {
    }

    //! Start ticker.
    void start() {
        if (start_ != 0) {
            roc_panic("ticker: can't start ticker twice");
        }
        start_ = timestamp_us();
    }

    //! Returns number of ticks elapsed since start.
    //! If ticker is not started yet, it is started automatically.
    uint64_t elapsed() {
        if (start_ == 0) {
            start();
            return 0;
        } else {
            return (timestamp_us() - start_) * freq_ / 1000000;
        }
    }

    //! Wait until the given number of ticks elapses since start.
    //! If ticker is not started yet, it is started automatically.
    void wait(uint64_t ticks) {
        if (start_ == 0) {
            start();
        }
        sleep_until_us(start_ + ticks * 1000000 / freq_);
    }

private:
    const uint64_t freq_;
    uint64_t start_;
};

} // namespace core
} // namespace roc

#endif // ROC_CORE_TICKER_H_
