/*  -*- Mode: C++ -*-
 * Copyright (C) 2012, Julian Stecklina <jsteckli@os.inf.tu-dresden.de>
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

namespace nre {

    class Thread;

    class Vi : public ObjCap {
    public:
        explicit Vi(capsel_t cap) : ObjCap(cap, KEEP_CAP_BIT | KEEP_SEL_BIT) { }
        explicit Vi(Thread *ec_handler, Thread *ec_recall, capsel_t cap, uint32_t evt, Pd *pd = Pd::current());

        static void block(word_t mask = ~0UL) { Syscalls::vi_ctrl(0, Syscalls::ViOp::BLOCK, mask); }
        void trigger() { Syscalls::vi_ctrl(sel(), Syscalls::ViOp::TRIGGER, 0); }
    };

}

// EOF
