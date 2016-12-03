/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @brief C API.

#ifndef ROC_C_API_H_
#define ROC_C_API_H_ 

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    #define ROC_TRANSMITTER_OPTIONS_FEC             1
    #define ROC_TRANSMITTER_OPTIONS_SE16_44100      128

    #define ROC_VALID       (0)
    #define ROC_INVALID     (-1)

    typedef struct Roc_Handler Roc_Handler ;

    void roc_initialize(const unsigned int verbosity);

    int roc_initialize_transmitter(Roc_Handler **handler,
            const char *destination_adress, uint32_t options);
    size_t roc_transmit( Roc_Handler *handler, const void *data, size_t data_len );

    void roc_close( Roc_Handler *handler );

    //! Returns sum of all knnown latencies in microseconds.
    //!
    //! The Roc sums all latencies produced by every cause during its
    //! work. For example: 
    //!   - length of the source packet on loading;
    //!   - whole receivers latency which is available on SDP link.
    //! @returns Returns the sum in microseconds. It doesn't indicate error.
    uint32_t roc_trasnmitter_latency( Roc_Handler *handler );

#ifdef __cplusplus
}
#endif

#endif // ROC_C_API_H_
