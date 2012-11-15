/*
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

#include <kobj/Vi.h>
#include <kobj/Thread.h>

namespace nre {

Vi::Vi(Thread *ec_handler, Thread *ec_recall, capsel_t cap, uint32_t evt, Pd *pd)
    : ObjCap(cap, KEEP_SEL_BIT)
{
    Syscalls::create_vi(cap, ec_handler->sel(), ec_recall ? ec_recall->sel() : 0, evt, pd->sel());
}

}

// EOF
