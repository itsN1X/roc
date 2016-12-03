/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @brief C API.

#ifndef ROC_H_
#define ROC_H_

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void roc_log_set_level(const unsigned int verbosity);

typedef struct roc_transport roc_transport;

roc_transport* roc_transport_open(const char *destination_adress);

size_t roc_transport_send(roc_transport *transport, const float *samples, size_t n_samples);

void roc_transport_close(roc_transport *transport);

//! Returns sum of all knnown latencies in microseconds.
//!
//! The Roc sums all latencies produced by every cause during its
//! work. For example:
//!   - length of the source packet on loading;
//!   - whole receivers latency which is available on SDP link.
//! @returns Returns the sum in microseconds. It doesn't indicate error.
uint32_t roc_transport_get_latency(roc_transport *transport);

#ifdef __cplusplus
}
#endif

#endif // ROC_H_
