/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "roc_netio/udp_receiver.h"
#include "roc_core/log.h"
#include "roc_core/panic.h"
#include "roc_core/shared_ptr.h"
#include "roc_netio/inet_address.h"
#include "roc_packet/address_to_str.h"

namespace roc {
namespace netio {

UDPReceiver::UDPReceiver(uv_loop_t& event_loop,
                         packet::IWriter& writer,
                         packet::PacketPool& packet_pool,
                         core::BufferPool<uint8_t>& buffer_pool,
                         core::IAllocator& allocator)
    : allocator_(allocator)
    , loop_(event_loop)
    , handle_initialized_(false)
    , writer_(writer)
    , packet_pool_(packet_pool)
    , buffer_pool_(buffer_pool)
    , packet_counter_(0) {
}

UDPReceiver::~UDPReceiver() {
    stop();
}

void UDPReceiver::destroy() {
    allocator_.destroy(*this);
}

bool UDPReceiver::start(packet::Address& bind_address) {
    roc_log(LogDebug, "udp receiver: opening port %s",
            packet::address_to_str(address_).c_str());

    if (int err = uv_udp_init(&loop_, &handle_)) {
        roc_log(LogError, "udp receiver: uv_udp_init(): [%s] %s", uv_err_name(err),
                uv_strerror(err));
        return false;
    }

    handle_.data = this;
    handle_initialized_ = true;

    sockaddr_in inet_addr;
    to_inet_address(bind_address, inet_addr);

    if (int err = uv_udp_bind(&handle_, (sockaddr*)&inet_addr, UV_UDP_REUSEADDR)) {
        roc_log(LogError, "udp receiver: uv_udp_bind(): [%s] %s", uv_err_name(err),
                uv_strerror(err));
        return false;
    }

    int inet_addr_len = sizeof(inet_addr);
    if (int err = uv_udp_getsockname(&handle_, (sockaddr*)&inet_addr, &inet_addr_len)) {
        roc_log(LogError, "udp receiver: uv_udp_getsockname(): [%s] %s", uv_err_name(err),
                uv_strerror(err));
        return false;
    }

    from_inet_address(inet_addr, bind_address);
    address_ = bind_address;

    if (int err = uv_udp_recv_start(&handle_, alloc_cb_, recv_cb_)) {
        roc_log(LogError, "udp receiver: uv_udp_recv_start(): [%s] %s", uv_err_name(err),
                uv_strerror(err));
        return false;
    }

    return true;
}

void UDPReceiver::stop() {
    if (!handle_initialized_) {
        return;
    }

    handle_initialized_ = false;

    if (uv_is_closing((uv_handle_t*)&handle_)) {
        return;
    }

    roc_log(LogDebug, "udp receiver: closing port %s",
            packet::address_to_str(address_).c_str());

    if (int err = uv_udp_recv_stop(&handle_)) {
        roc_log(LogError, "udp receiver: uv_udp_recv_stop(): [%s] %s", uv_err_name(err),
                uv_strerror(err));
    }

    uv_close((uv_handle_t*)&handle_, NULL);
}

void UDPReceiver::alloc_cb_(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
    roc_panic_if_not(handle);
    roc_panic_if_not(buf);

    roc_log(LogTrace, "udp receiver: allocating buffer: size=%ld", (long)size);

    UDPReceiver& self = *(UDPReceiver*)handle->data;

    core::SharedPtr<core::Buffer<uint8_t> > bp =
        new (self.buffer_pool_) core::Buffer<uint8_t>(self.buffer_pool_);

    if (!bp) {
        roc_log(LogError, "udp receiver: can't allocate buffer");

        buf->base = NULL;
        buf->len = 0;

        return;
    }

    if (size > bp->size()) {
        roc_log(LogTrace, "udp receiver: truncating buffer size:"
                          " suggested=%ld max=%ld",
                (long)size, (long)bp->size());

        size = bp->size();
    }

    bp->incref(); // will be decremented in recv_cb_()

    buf->base = (char*)bp->data();
    buf->len = size;
}

void UDPReceiver::recv_cb_(uv_udp_t* handle,
                           ssize_t nread,
                           const uv_buf_t* buf,
                           const sockaddr* inet_src_addr,
                           unsigned flags) {
    roc_panic_if_not(handle);
    roc_panic_if_not(buf);

    UDPReceiver& self = *(UDPReceiver*)handle->data;
    self.packet_counter_++;

    packet::Address src_addr;
    if (inet_src_addr) {
        from_inet_address(*(const sockaddr_in*)inet_src_addr, src_addr);
    }

    roc_log(LogTrace, "udp receiver: got packet: num=%u src=%s dst=%s nread=%ld",
            self.packet_counter_, packet::address_to_str(src_addr).c_str(),
            packet::address_to_str(self.address_).c_str(), (long)nread);

    core::SharedPtr<core::Buffer<uint8_t> > bp =
        core::Buffer<uint8_t>::container_of(buf->base);

    // one reference for incref() called from alloc_cb_()
    // one reference for the shared pointer above
    roc_panic_if(bp->getref() != 2);

    // decrement reference counter incremented in alloc_cb_()
    bp->decref();

    if (nread < 0) {
        roc_log(LogError, "udp receiver: network error: num=%u src=%s dst=%s nread=%ld",
                self.packet_counter_, packet::address_to_str(src_addr).c_str(),
                packet::address_to_str(self.address_).c_str(), (long)nread);
        return;
    }

    if (nread == 0) {
        if (inet_src_addr == NULL) {
            // no more data for now
        } else {
            roc_log(LogTrace, "udp receiver: empty packet: num=%u src=%s dst=%s",
                    self.packet_counter_, packet::address_to_str(src_addr).c_str(),
                    packet::address_to_str(self.address_).c_str());
        }
        return;
    }

    if (inet_src_addr == NULL) {
        roc_panic("udp receiver: unexpected null source address");
    }

    if (flags & UV_UDP_PARTIAL) {
        roc_log(LogDebug, "udp receiver:"
                          " ignoring partial read: num=%u src=%s dst=%s nread=%ld",
                self.packet_counter_, packet::address_to_str(src_addr).c_str(),
                packet::address_to_str(self.address_).c_str(), (long)nread);
        return;
    }

    if ((size_t)nread > bp->size()) {
        roc_panic("udp receiver: unexpected buffer size (got %ld, max %ld)", (long)nread,
                  (long)bp->size());
    }

    packet::PacketPtr pp = new (self.packet_pool_) packet::Packet(self.packet_pool_);
    if (!pp) {
        roc_log(LogError, "udp receiver: can't allocate packet");
        return;
    }

    pp->add_flags(packet::Packet::FlagUDP);

    pp->udp()->src_addr = src_addr;
    pp->udp()->dst_addr = self.address_;

    pp->set_data(core::Slice<uint8_t>(*bp, 0, (size_t)nread));

    self.writer_.write(pp);
}

} // namespace netio
} // namespace roc
