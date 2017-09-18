/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "roc_rtp/parser.h"
#include "roc_core/log.h"
#include "roc_rtp/headers.h"

namespace roc {
namespace rtp {

Parser::Parser(const FormatMap& format_map, packet::IParser* inner_parser)
    : format_map_(format_map)
    , inner_parser_(inner_parser) {
}

bool Parser::parse(packet::Packet& packet, const core::Slice<uint8_t>& buffer) {
    if (buffer.size() < sizeof(Header)) {
        roc_log(LogDebug, "rtp parser: bad packet, size < %d (rtp header)",
                (int)sizeof(Header));
        return false;
    }

    const Header& header = *(const Header*)buffer.data();

    if (header.version() != V2) {
        roc_log(LogDebug, "rtp parser: bad version, get %d, expected %d",
                (int)header.version(), (int)V2);
        return false;
    }

    size_t header_size = header.header_size();

    if (header.has_extension()) {
        header_size += sizeof(ExtentionHeader);
    }

    if (buffer.size() < header_size) {
        roc_log(LogDebug, "rtp parser: bad packet, size < %d (rtp header + ext header)",
                (int)header_size);
        return false;
    }

    if (header.has_extension()) {
        const ExtentionHeader& extension =
            *(const ExtentionHeader*)(buffer.data() + header.header_size());

        header_size += extension.data_size();
    }

    if (buffer.size() < header_size) {
        roc_log(LogDebug,
                "rtp parser: bad packet, size < %d (rtp header + ext header + ext data)",
                (int)header_size);
        return false;
    }

    size_t payload_begin = header_size;
    size_t payload_end = buffer.size();

    if (header.has_padding()) {
        if (payload_begin == payload_end) {
            roc_log(LogDebug,
                    "rtp parser: bad packet, empty payload but padding flag is set");
            return false;
        }

        const uint8_t pad_size = buffer.data()[payload_end - 1];

        if (pad_size == 0) {
            roc_log(LogDebug, "rtp parser: bad packet, padding size octet is zero");
            return false;
        }

        if (size_t(payload_end - payload_begin) < size_t(pad_size)) {
            roc_log(LogDebug,
                    "rtp parser: bad packet, padding size octet > %d (payload size)",
                    (int)(payload_end - payload_begin));
            return false;
        }

        payload_end -= pad_size;
    }

    packet.add_flags(packet::Packet::FlagRTP);

    packet::RTP& rtp = *packet.rtp();

    rtp.source = header.ssrc();
    rtp.seqnum = header.seqnum();
    rtp.timestamp = header.timestamp();
    rtp.marker = header.marker();
    rtp.payload_type = header.payload_type();
    rtp.header = buffer.range(0, header_size);
    rtp.payload = buffer.range(payload_begin, payload_end);

    if (const Format* format = format_map_.format(header.payload_type())) {
        packet.add_flags(format->flags);
        rtp.duration = format->duration(rtp);
    }

    if (inner_parser_) {
        return inner_parser_->parse(packet, rtp.payload);
    }

    return true;
}

} // namespace packet
} // namespace roc
