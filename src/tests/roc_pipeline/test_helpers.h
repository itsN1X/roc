/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ROC_PIPELINE_TEST_HELPERS_H_
#define ROC_PIPELINE_TEST_HELPERS_H_

#include "roc_audio/units.h"
#include "roc_core/stddefs.h"

namespace roc {
namespace pipeline {

namespace {

const audio::sample_t Epsilon = 0.00001f;

audio::sample_t nth_sample(uint8_t n) {
    return audio::sample_t(n) / 1024;
}

} // namespace

} // namespace pipeline
} // namespace roc

#endif // ROC_PIPELINE_TEST_HELPERS_H_
