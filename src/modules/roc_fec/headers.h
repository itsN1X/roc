/*
 * Copyright (c) 2017 Mikhail Baranov
 * Copyright (c) 2017 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_fec/headers.h
//! @brief FECFRAME headers.

#ifndef ROC_FEC_HEADERS_H_
#define ROC_FEC_HEADERS_H_

#include "roc_core/attributes.h"
#include "roc_core/endian.h"
#include "roc_core/panic.h"
#include "roc_core/stddefs.h"

namespace roc {
namespace fec {

//! FEC Payload ID type.
enum PayloadID_Type {
    Source, //!< Source packet header of footer.
    Repair  //!< Source packet header of footer.
};

//! FEC Payload ID position.
enum PayloadID_Pos {
    Header, //!< PayloadID comes before payload.
    Footer  //!< PayloadID comes after payload.
};

//! LDPC Source FEC Payload ID.
//!
//! @code
//!    0                   1                   2                   3
//!    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//!   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//!   |   Source Block Number (SBN)   |   Encoding Symbol ID (ESI)    |
//!   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//!   |    Source Block Length (k)    |
//!   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! @endcode
class ROC_ATTR_PACKED LDPC_Source_PayloadID {
private:
    //! Source block number.
    uint16_t sbn_;

    //! Encoding symbol ID.
    uint16_t esi_;

    //! Source block length.
    uint16_t k_;

public:
    //! Clear header.
    void clear() {
        memset(this, 0, sizeof(*this));
    }

    //! Get source block number.
    uint16_t sbn() const {
        return ROC_NTOH_16(sbn_);
    }

    //! Set source block number.
    void set_sbn(uint16_t val) {
        sbn_ = ROC_HTON_16(val);
    }

    //! Get encoding symbol ID.
    uint16_t esi() const {
        return ROC_NTOH_16(esi_);
    }

    //! Set encoding symbol ID.
    void set_esi(uint16_t val) {
        esi_ = ROC_HTON_16(val);
    }

    //! Get source block length.
    uint16_t k() const {
        return ROC_NTOH_16(k_);
    }

    //! Set source block length.
    void set_k(uint16_t val) {
        k_ = ROC_HTON_16(val);
    }
};

//! LDPC Repair FEC Payload ID.
//!
//! @code
//!    0                   1                   2                   3
//!    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//!   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//!   |   Source Block Number (SBN)   |   Encoding Symbol ID (ESI)    |
//!   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//!   |    Source Block Length (k)    |  Number Encoding Symbols (n)  |
//!   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! @endcode
class ROC_ATTR_PACKED LDPC_Repair_PayloadID {
private:
    //! Source block number.
    uint16_t sbn_;

    //! Encoding symbol ID.
    uint16_t esi_;

    //! Source block length.
    uint16_t k_;

    //! Number encoding symbols.
    uint16_t n_;

public:
    //! Clear header.
    void clear() {
        memset(this, 0, sizeof(*this));
    }

    //! Get source block number.
    uint16_t sbn() const {
        return ROC_NTOH_16(sbn_);
    }

    //! Set source block number.
    void set_sbn(uint16_t val) {
        sbn_ = ROC_HTON_16(val);
    }

    //! Get encoding symbol ID.
    uint16_t esi() const {
        return ROC_NTOH_16(esi_);
    }

    //! Set encoding symbol ID.
    void set_esi(uint16_t val) {
        esi_ = ROC_HTON_16(val);
    }

    //! Get source block length.
    uint16_t k() const {
        return ROC_NTOH_16(k_);
    }

    //! Set source block length.
    void set_k(uint16_t val) {
        k_ = ROC_HTON_16(val);
    }

    //! Get number encoding symbols.
    uint16_t n() const {
        return ROC_NTOH_16(n_);
    }

    //! Set number encoding symbols.
    void set_n(uint16_t val) {
        n_ = ROC_HTON_16(val);
    }
};

//! Reed-Solomon Source or Repair Payload ID (for m=8).
//!
//! @code
//!    0                   1                   2                   3
//!    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//!   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//!   |           Source Block Number (24 bits)       | Enc. Symb. ID |
//!   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//!   |    Source Block Length (k)    |
//!   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! @endcode
class ROC_ATTR_PACKED RSm8_PayloadID {
private:
    //! Source block number.
    uint8_t sbn_[3];

    //! Encoding symbol ID.
    uint8_t esi_;

    //! Source block length.
    uint16_t k_;

public:
    //! Clear header.
    void clear() {
        memset(this, 0, sizeof(*this));
    }

    //! Get source block number.
    uint32_t sbn() const {
        return (uint32_t(sbn_[0]) << 16) | (uint32_t(sbn_[1]) << 8) | uint32_t(sbn_[2]);
    }

    //! Set source block number.
    void set_sbn(uint32_t val) {
        roc_panic_if((val >> 24) != 0);
        sbn_[0] = uint8_t((val >> 16) & 0xff);
        sbn_[1] = uint8_t((val >> 8) & 0xff);
        sbn_[2] = uint8_t(val & 0xff);
    }

    //! Get encoding symbol ID.
    uint8_t esi() const {
        return esi_;
    }

    //! Set encoding symbol ID.
    void set_esi(uint16_t val) {
        roc_panic_if((val >> 8) != 0);
        esi_ = (uint8_t)val;
    }

    //! Get source block length.
    uint16_t k() const {
        return ROC_NTOH_16(k_);
    }

    //! Set source block length.
    void set_k(uint16_t val) {
        k_ = ROC_HTON_16(val);
    }
};

} // namespace fec
} // namespace roc

#endif // ROC_FEC_HEADERS_H_
