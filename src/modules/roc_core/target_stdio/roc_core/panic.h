/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_core/target_stdio/roc_core/panic.h
//! @brief Panic function.

#ifndef ROC_CORE_PANIC_H_
#define ROC_CORE_PANIC_H_

#include "roc_core/attributes.h"
#include "roc_core/macros.h"

#ifndef ROC_MODULE
#error "ROC_MODULE not defined"
#endif

//! Panic if condition is true.
//! @note
//!  This is a shorthand; roc_panic() with a meaningful error message is
//!  preferred way to panic.
#define roc_panic_if(x)                                                                  \
    do {                                                                                 \
        if ((x)) {                                                                       \
            roc_panic("%s", #x);                                                         \
        }                                                                                \
    } while (0)

//! Panic if condition is false.
//! @note
//!  This is a shorthand; roc_panic() with a meaningful error message is
//!  preferred way to panic.
#define roc_panic_if_not(x) roc_panic_if(!(x))

//! Print error message and terminate program gracefully.
//! @remarks
//!  Never returns and never throws.
#define roc_panic(...)                                                                   \
    ::roc::core::panic(ROC_STRINGIZE(ROC_MODULE), __FILE__, __LINE__, __VA_ARGS__)

namespace roc {
namespace core {

//! Print error message and terminate program gracefully.
void panic(const char* module, const char* file, int line, const char* format, ...)
    ROC_ATTR_NOTHROW ROC_ATTR_NORETURN ROC_ATTR_PRINTF(4, 5);

//! Panic handler.
typedef void (*PanicHandler)(const char*);

//! Set panic handler.
//!
//! @remarks
//!  If @p handler is not NULL, it will be called instead of terminating program.
//!  Default panic handler is NULL.
//!
//! @note
//!  If panic handler doesn't terminate or throws exception, exception is
//!  suppressed and program is terminated anyway. Our code is not exception
//!  safe and excpects that roc_panic() never returns.
void set_panic_handler(PanicHandler handler);

} // namespace core
} // namespace roc

#endif // ROC_CORE_PANIC_H_
