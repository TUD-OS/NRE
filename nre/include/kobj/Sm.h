/*
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

#include <kobj/ObjCap.h>
#include <kobj/Pd.h>
#include <arch/SyscallABI.h>
#include <util/ScopedCapSels.h>

namespace nre {

/**
 * Represents a semaphore.
 */
class Sm : public ObjCap {
public:
    /**
     * Attaches a semaphore-object to the given capability-selector, that does already refer to a Sm.
     * The destructor will neither free the selector, nor the capability.
     *
     * @param cap the selector for the Sm
     */
    // TODO get rid of the bool
    explicit Sm(capsel_t cap, bool) : ObjCap(cap, KEEP_CAP_BIT | KEEP_SEL_BIT) { }

    /**
     * Creates a semaphore at given selector. The destructor will not free the selector, but only
     * the capability.
     *
     * @param cap the selector to use
     * @param initial the initial value of the semaphore
     * @param pd the Pd to create it in (default: current)
     */
    explicit Sm(capsel_t cap, uint initial, Pd *pd = Pd::current()) : ObjCap(cap, KEEP_SEL_BIT) {
        Syscalls::create_sm(cap, initial, pd->sel());
    }

    /**
     * Creates a semaphore
     *
     * @param initial the initial value of the semaphore
     * @param pd the Pd to create it in (default: current)
     */
    explicit Sm(uint initial, Pd *pd = Pd::current()) {
        ScopedCapSels scs;
        Syscalls::create_sm(scs.get(), initial, pd->sel());
        sel(scs.release());
    }

    /**
     * Performs a down on this semaphore. That is, if the value of it is zero, it will block until
     * someone does an up(). Otherwise it will decrease the value.
     */
    void down() {
        Syscalls::sm_ctrl(sel(), Syscalls::SM_DOWN);
    }

    /**
     * Performs a zero on this semaphore. That is, if the value of it is zero, it will block until
     * someone does an up(). Otherwise it will set the value to zero.
     */
    void zero() {
        Syscalls::sm_ctrl(sel(), Syscalls::SM_ZERO);
    }

    /**
     * Performs an up on this semaphore. That is, if there is somebody blocking on it, it unblocks
     * it. Otherwise it increases the value of the semaphore.
     */
    void up() {
        Syscalls::sm_ctrl(sel(), Syscalls::SM_UP);
    }
};

}
