/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <mach/clock.h>
#include <mach/clock_types.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

#include "roc_core/errno_to_str.h"
#include "roc_core/panic.h"
#include "roc_core/time.h"

namespace roc {
namespace core {

namespace {

pthread_once_t once_control = PTHREAD_ONCE_INIT;

double steady_factor = 0;

/* mach_absolute_time() returns a Mach Time unit - clock ticks. The
 * length of a tick is a CPU dependent. On most Intel CPUs it probably
 * will be 1 nanoseconds per tick, but let's not rely on this. Mach
 * provides a transformation factor that can be used to convert abstract
 * mach time units to nanoseconds.
 */
void init_steady_factor() {
    mach_timebase_info_data_t info;

    kern_return_t ret = mach_timebase_info(&info);
    if (ret != KERN_SUCCESS) {
        roc_panic("mach_timebase_info: %s", mach_error_string(ret));
    }

    steady_factor = (double)info.numer / info.denom;
}

} // namespace

/* As Apple mentioned: "The mach_timespec_t API is deprecated in OS X. The
 * newer and preferred API is based on timer objects that in turn use
 * AbsoluteTime as the basic data type".
 *
 * We still use a function from the old OS X API (clock_sleep), because OS X
 * API only provides one method to wait until some period (mach_wait_until)
 * and doesn't allow to select a clock against which the sleep interval is to
 * be measured to specify the sleep interval as either an absolute or a
 * relative value.
 *
 * https://developer.apple.com/library/content/documentation/Darwin/Conceptual/KernelProgramming/Mach/Mach.html#//apple_ref/doc/uid/TP30000905-CH209-TPXREF111
 */
nanoseconds_t timestamp() {
    if (int err = pthread_once(&once_control, init_steady_factor)) {
        roc_panic("pthread_once: %s", errno_to_str(err).c_str());
    }
    return nanoseconds_t(mach_absolute_time() * steady_factor);
}

void sleep_until(nanoseconds_t ns) {
    mach_timespec_t ts;
    ts.tv_sec = (unsigned int)(ns / 1000000000);
    ts.tv_nsec = (int)(ns % 1000000000);

    kern_return_t ret = KERN_SUCCESS;
    for (;;) {
        // We are interested in using SYSTEM_CLOCK (aka CLOCK_MONOTONIC in
        // other world), digging into XNU source code we can find that if we
        // set a name for a clock port to MACH_PORT_NULL, a kernel will use
        // SYSTEM_CLOCK.
        //
        // https://opensource.apple.com/source/xnu/xnu-2422.1.72/osfmk/kern/clock_oldops.c
        // We are interested in a @clock_sleep_trap, because it's used by
        // @clock_sleep under the hood.
        ret = clock_sleep(MACH_PORT_NULL, TIME_ABSOLUTE, ts, NULL);
        if (ret == KERN_SUCCESS) {
            break;
        }

        if (ret != KERN_ABORTED) {
            roc_panic("clock_sleep(TIME_ABSOLUTE): %s", mach_error_string(ret));
        }
    }
}

void sleep_for(nanoseconds_t ns) {
    sleep_until(timestamp() + ns);
}

} // namespace core
} // namespace roc
