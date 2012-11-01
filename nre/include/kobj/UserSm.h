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

#include <kobj/Sm.h>
#include <util/Atomic.h>
#include <stream/Serial.h>

namespace nre {

/**
 * A user semaphore optimized for the case where we do not block.
 */
class UserSm {
public:
    /**
     * Creates a user semaphore.
     *
     * @param initial the initial value for the semaphore (default 1)
     */
    explicit UserSm(uint initial = 1) : _sem(0), _value(initial) { }

    /**
     * Performs a down on this semaphore. That is, if the value is zero, it blocks on the associated
     * kernel semaphore. If not, it decreases the value.
     */
    void down() {
        if(Atomic::add(&_value, -1) <= 0)
            _sem.down();
    }

    /**
     * Performs an up on this semaphore. That is, it increases the value and if necessary, it
     * unblocks a waiting Ec that blocked on the associated kernel semaphore.
     */
    void up() {
        if(Atomic::add(&_value, +1) < 0)
            _sem.up();
    }

private:
    Sm _sem;
    long _value;
};

}
