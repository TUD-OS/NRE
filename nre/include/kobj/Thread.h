/* -*- Mode: C++ -*-
 * Copyright (C) 2012, Nils Asmussen <nils@os.inf.tu-dresden.de>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * This file is part of NRE (NOVA runtime environment).
 *
 * NRE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * NRE is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 */

#pragma once

#include <arch/ExecEnv.h>
#include <kobj/Ec.h>
#include <kobj/Vi.h>
#include <mem/DataSpace.h>
#include <utcb/Utcb.h>
#include <util/SList.h>
#include <util/Atomic.h>
#include <util/Sync.h>
#include <Syscalls.h>

namespace nre {

class Pd;
class RCU;
class RCULock;
class Mutex;

/**
 * Represents a thread, i.e. an Ec that has a stack and a Utcb. It is the base class for the two
 * supported thread variants, LocalThread and GlobalThread. This class can't be used directly.
 * Note that each Thread contains a few slots for thread local storage (TLS). The index
 * Thread::TLS_PARAM is always available, e.g. to pass a parameter to a Thread. You may create
 * additional ones by Thread::create_tls().
 */
class Thread : public Ec, public SListItem {
    friend class RCU;
    friend class RCULock;
    friend class Mutex;

    static const size_t TLS_SIZE    = 4;

public:
    // the slot 0 is reserved for putting a ec-parameter in it
    static const size_t TLS_PARAM   = 0;
    enum Flags {
        HAS_OWN_STACK   = 1,
        HAS_OWN_UTCB    = 2,
    };

    /**
     * @return the current execution context
     */
    static Thread *current() {
        return ExecEnv::get_current_thread();
    }

protected:
    /**
     * Constructor
     *
     * @param cpu the logical cpu to bind the Thread to
     * @param evb the offset for the event-portals
     * @param cap the capability (INVALID if a new one should be used)
     * @param stack the stack address (0 = create one automatically)
     * @param uaddr the utcb address (0 = create one automatically)
     */
    explicit Thread(cpu_t cpu, capsel_t evb, capsel_t cap = INVALID, uintptr_t stack = 0,
                    uintptr_t uaddr = 0);
    /**
     * The actual creation of the Thread.
     *
     * @param pd the protection domain the Thread should run in
     * @param type the type of Thread
     * @param sp the stack-pointer
     */
    void create(Pd *pd, Syscalls::ECType type, void *sp);

public:
    /**
     * Destructor
     */
    virtual ~Thread();

    /**
     * @return the flags
     */
    uint flags() const {
        return _flags;
    }
    /**
     * @return the stack-address
     */
    uintptr_t stack() const {
        return _stack_addr;
    }
    /**
     * @return the utcb
     */
    Utcb *utcb() {
        return reinterpret_cast<Utcb*>(_utcb_addr);
    }

    /**
     * @see UtcbBase::fetch_events
     *
     * @return event bitmask
     */
    word_t fetch_events(word_t mask = ~0ULL) { return utcb()->fetch_events(mask); }

    /**
     * Creates a new TLS slot for all Threads
     *
     * @return the TLS index
     */
    size_t create_tls() {
        size_t next = Atomic::add(&_tls_idx, +1);
        assert(next < TLS_SIZE);
        return next;
    }
    /**
     * @param idx the TLS index
     * @return the value at given index
     */
    template<typename T>
    T get_tls(size_t idx) const {
        assert(idx < TLS_SIZE);
        return reinterpret_cast<T>(_tls[idx]);
    }
    /**
     * @param idx the TLS index
     * @param val the new value
     */
    template<typename T>
    void set_tls(size_t idx, T val) {
        assert(idx < TLS_SIZE);
        _tls[idx] = reinterpret_cast<void*>(val);
    }

private:
    Thread(const Thread&);
    Thread& operator=(const Thread&);

    /// Make sure
    void ensure_mutex_irq();

    /// Block the current thread, because it is blocking for a mutex.
    void mutex_wait();


    /// Called by another thread to wake up this thread from waiting for a mutex.
    void mutex_wakeup();

    Thread        *_waiting_for;

    // XXX Should actually be embedded in the thread object, but since
    // ObjCaps cannot be copied, this is not possible right now. :(
    Vi            *_mutex_irq;

    uint32_t       _rcu_counter;
    uintptr_t      _utcb_addr;
    uintptr_t      _stack_addr;
    uint           _flags;
    void          *_tls[TLS_SIZE];
    static size_t  _tls_idx;
};

}
