/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_audio/mixer.h
//! @brief Mixer.

#ifndef ROC_AUDIO_MIXER_H_
#define ROC_AUDIO_MIXER_H_

#include "roc_audio/ireader.h"
#include "roc_audio/units.h"
#include "roc_core/buffer_pool.h"
#include "roc_core/list.h"
#include "roc_core/noncopyable.h"

namespace roc {
namespace audio {

//! Mixer.
//! Mixes multiple input streams into one output stream.
//!
//! For example, these two input streams:
//! @code
//!  1, 2, 3, ...
//!  4, 5, 6, ...
//! @endcode
//!
//! are transformed into this output stream:
//! @code
//!  5, 7, 9, ...
//! @endcode
class Mixer : public IReader, public core::NonCopyable<> {
public:
    //! Initialize.
    //!
    //! @b Parameters
    //!  - @p buffer_pool is used to allocate a temporary chunk of samples
    explicit Mixer(core::BufferPool<sample_t>& buffer_pool);

    //! Read audio frame.
    //! @remarks
    //!  Reads samples from every input reader, mixes them, and fills @p frame
    //!  with the result.
    virtual void read(Frame& frame);

    //! Add input reader.
    void add(IReader&);

    //! Remove input reader.
    void remove(IReader&);

private:
    core::BufferPool<sample_t>& buffer_pool_;

    core::List<IReader, core::NoOwnership> readers_;
    Frame temp_;
};

} // namespace audio
} // namespace roc

#endif // ROC_AUDIO_MIXER_H_
