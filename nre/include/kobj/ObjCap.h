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

#include <arch/Types.h>

namespace nre {

/**
 * The base class for all object-capabilities. Manages the selector and capability.
 */
class ObjCap {
public:
    static const capsel_t INVALID   = 0x3FFFFFFF;

protected:
    enum {
        // whether we don't want to free the selector
        KEEP_SEL_BIT    = 1 << (sizeof(capsel_t) * 8 - 1),
        // whether we don't want to free the capability
        KEEP_CAP_BIT    = 1 << (sizeof(capsel_t) * 8 - 2),
        KEEP_BITS       = KEEP_SEL_BIT | KEEP_CAP_BIT
    };

    /**
     * Constructor for a new capability with given selector. Does not actually create the cap. This
     * will be done in subclasses.
     *
     * @param sel the selector
     * @param flags control whether the selector and/or the capability should be free'd
     *  during destruction
     */
    explicit ObjCap(capsel_t sel = INVALID, uint flags = 0) : _sel(sel | flags) {
    }

public:
    /**
     * Destructor. Depending on the flags, it frees the selector and/or the capability (revoke).
     */
    virtual ~ObjCap();

    /**
     * @return the selector for this capability
     */
    capsel_t sel() const {
        return _sel & ~KEEP_BITS;
    }

protected:
    /**
     * Sets the selector but keeps the flags
     *
     * @param sel the new selector
     */
    void sel(capsel_t sel) {
        _sel = (_sel & KEEP_BITS) | sel;
    }
    /**
     * Sets the flags but keeps the selector
     *
     * @param flags the new flags
     */
    void set_flags(uint flags) {
        _sel = (_sel & ~KEEP_BITS) | flags;
    }

private:
    // object-caps cannot be copied, because I think there are very few usecases and often it
    // causes problems:
    // - how to clone a Sm? we can't clone the current state of it, i.e. its value. so it would be
    //   a partial clone, which is strange.
    // - for many object-caps we don't know all properties to be able to really clone it. thus, we would
    //   have to store all of them, which causes more memory-overhead and is probably rarely used.
    // - what about "fixed" portals, i.e. exception-portals for example? creating another portal
    //   for the same cap-selector won't work. using a different selector doesn't make any sense.
    //   the same is true for interrupt-semaphores
    ObjCap(const ObjCap&);
    ObjCap& operator=(const ObjCap&);

    capsel_t _sel;
};

}
