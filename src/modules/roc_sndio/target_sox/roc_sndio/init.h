/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_sndio/target_sox/roc_sndio/init.h
//! @brief Init.

#ifndef ROC_SNDIO_INIT_H_
#define ROC_SNDIO_INIT_H_

#include "roc_core/stddefs.h"

namespace roc {
namespace sndio {

enum {
    InitOnce = (1 << 0), //!< Don't init twice.
    InitSox = (1 << 1),  //!< Call sox_init().
    InitLog = (1 << 2)   //!< Install SoX log handler.
};

//! Initialize SoX.
//!
//! If InitOnce option is set, and sndio::init() was already called
//! with any options, just return.
//!
//! Otherwise:
//!  - If InitSox option is set, call sox_init().
//!  - If InitLog option is set, install sox log handler.
//!  - If InitBufsz option is set, set sox buffer sizes.
//!
//! If you need non-default initialization, call init() with desired
//! options before using this library. Other calls from this library
//! use InitOnce, thus they will be ignored.
//!
//! @note
//!  It's not allowed to call sox_init() twice without paired sox_quit().
void init(int options = InitOnce | InitSox | InitLog);

} // namespace sndio
} // namespace roc

#endif // ROC_SNDIO_INIT_H_
