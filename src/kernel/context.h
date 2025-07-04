// Copyright (c) 2022 The Shahcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_KERNEL_CONTEXT_H
#define SHAHCOIN_KERNEL_CONTEXT_H

#include <util/signalinterrupt.h>

#include <memory>

namespace kernel {
//! Context struct holding the kernel library's logically global state, and
//! passed to external libshahcoin_kernel functions which need access to this
//! state. The kernel library API is a work in progress, so state organization
//! and member list will evolve over time.
//!
//! State stored directly in this struct should be simple. More complex state
//! should be stored to std::unique_ptr members pointing to opaque types.
struct Context {
    //! Interrupt object that can be used to stop long-running kernel operations.
    util::SignalInterrupt interrupt;

    //! Declare default constructor and destructor that are not inline, so code
    //! instantiating the kernel::Context struct doesn't need to #include class
    //! definitions for all the unique_ptr members.
    Context();
    ~Context();
};

//! Global pointer to kernel::Context for legacy code. New code should avoid
//! using this, and require state it needs to be passed to it directly.
//!
//! Having this pointer is useful because it allows state be moved out of global
//! variables into the kernel::Context struct before all global references to
//! that state are removed. This allows the global references to be removed
//! incrementally, instead of all at once.
extern Context* g_context;
} // namespace kernel

#endif // SHAHCOIN_KERNEL_CONTEXT_H
